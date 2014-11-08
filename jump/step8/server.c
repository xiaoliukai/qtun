#include <arpa/inet.h>
#include <linux/ip.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "library.h"
#include "vector.h"

#include "network.h"

int bind_and_listen(unsigned short port)
{
    int fd, rc;
    int opt = 1;
    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    addr.sin_port = htons(port);

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        return -1;
    }

    rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (rc == -1)
    {
        perror("setsockopt");
        close(fd);
        return -1;
    }

    rc = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (rc == -1)
    {
        perror("bind");
        close(fd);
        return -1;
    }

    rc = listen(fd, SOMAXCONN);
    if (rc == -1)
    {
        perror("listen");
        close(fd);
        return -1;
    }

    return fd;
}

inline static int check_ip_by_mask(unsigned int src, unsigned int dst, unsigned char mask)
{
    unsigned int m = LEN2MASK(mask);
    return (src & m) == (dst & m);
}

static void accept_and_check(int bindfd)
{
    int fd = accept(bindfd, NULL, NULL);
    msg_t* msg = NULL;
    int sys;
    void* data = NULL;
    unsigned short len;
    if (fd == -1) return;

    if (read_msg_t(fd, &msg, 5) > 0)
    {
        sys_login_msg_t* login;
        msg_t* new_msg;
        if (msg->compress != this.compress || msg->encrypt != this.encrypt) // 算法不同直接将本地的加密压缩算法返回
        {
            msg->compress = this.compress;
            msg->encrypt = this.encrypt;
            msg->checksum = 0;
            msg->checksum = checksum(msg, sizeof(msg_t) + msg_data_length(msg));
            write_n(fd, msg, sizeof(msg_t) + msg_data_length(msg));
            goto end;
        }
        if (!parse_msg(msg, &sys, &data, &len))
        {
            fprintf(stderr, "parse sys_login_request failed\n");
            close(fd);
            goto end;
        }
        login = (sys_login_msg_t*)data;
        if (sys != 1 ||
            !CHECK_SYS_OP(msg->unused, SYS_LOGIN, 1) ||
            memcmp(login->check, SYS_MSG_CHECK, sizeof(login->check)) ||
            !check_ip_by_mask(login->ip, this.localip, this.netmask)) // 非法数据包
        {
            fprintf(stderr, "unknown sys_login_request message\n");
            close(fd);
            goto end;
        }
        if (login->ip == this.localip || active_vector_exists(&this.clients, compare_clients_by_ip, (void*)(long)login->ip, sizeof(login->ip)) >= 0) // IP已被占用
        {
            unsigned short i;
            unsigned int localip = login->ip & LEN2MASK(this.netmask);
            for (i = 1; i < LEN2MASK(32 - this.netmask); ++i)
            {
                unsigned int newip = (i << this.netmask) | localip;
                if (active_vector_exists(&this.clients, compare_clients_by_ip, (void*)(long)newip, sizeof(newip)) == -1)
                {
                    new_msg = new_login_msg(newip, this.netmask, 0);
                    if (new_msg)
                    {
                        write_n(fd, new_msg, sizeof(msg_t) + msg_data_length(new_msg));
                        free(new_msg);
                    }
                    else
                    {
                        fprintf(stderr, "Not enough memory\n");
                        close(fd);
                    }
                    goto end;
                }
            }
            new_msg = new_login_msg(0, 0, 0);
            if (new_msg)
            {
                write_n(fd, new_msg, sizeof(msg_t) + msg_data_length(new_msg));
                free(new_msg);
            }
            else
            {
                fprintf(stderr, "Not enough memory\n");
                close(fd);
            }
        }
        else
        {
            char cmd[1024];
            struct in_addr a = {login->ip};
            client_t* client;
            new_msg = new_login_msg(login->ip, this.netmask, 0);
            if (new_msg == NULL)
            {
                fprintf(stderr, "Not enough memory\n");
                close(fd);
                goto end;
            }
            client = malloc(sizeof(*client));
            if (client == NULL)
            {
                fprintf(stderr, "Not enough memory\n");
                free(new_msg);
                close(fd);
                goto end;
            }
            client->fd = fd;
            client->ip = login->ip;
            client->keepalive = time(NULL);
            if (!active_vector_append(&this.clients, client, sizeof(*client)))
            {
                fprintf(stderr, "set to hash_ip error\n");
                free(new_msg);
                free(client);
                close(fd);
                goto end;
            }
            sprintf(cmd, "route add %s dev %s", inet_ntoa(a), this.dev_name);
            SYSTEM_NORMAL(cmd);
            write_n(fd, new_msg, sizeof(msg_t) + msg_data_length(new_msg));
            free(new_msg);
        }
    }
    else
    {
        fprintf(stderr, "read sys_login_request message error\n");
        close(fd);
    }
end:
    if (msg) free(msg);
    if (data) free(data);
}

static void server_process_sys(client_t* client, msg_t* msg, const void* buffer, const size_t len)
{
    switch (GET_SYS_OP(msg->unused))
    {
    case SYS_PING:
        if (IS_SYS_REQUEST(msg->unused))
        {
            client->keepalive = time(NULL);
            msg_t* new_msg = new_keepalive_msg(0);
            write_n(client->fd, new_msg, sizeof(msg_t));
            printf("reply keepalive message\n");
            free(new_msg);
        }
        break;
    }
}

static void remove_clients(vector_t* v, const char* pfx)
{
    void* tmp;
    size_t tmp_len;
    while (vector_pop_back(v, &tmp, &tmp_len))
    {
        client_t* client;
        char cmd[128];
        char ip[16];
        struct in_addr a;
        active_vector_get(&this.clients, (size_t)tmp, (void**)&client, &tmp_len);
        a.s_addr = client->ip;
        sprintf(ip, "%s", inet_ntoa(a));
        printf("%s: %s\n", pfx, ip);
        sprintf(cmd, "route del %s dev %s", ip, this.dev_name);
        SYSTEM_NORMAL(cmd);
        active_vector_del(&this.clients, (size_t)tmp);
    }
}

static void server_process(int max, fd_set* set, int remotefd, int localfd)
{
    msg_t* msg;
    active_vector_iterator_t iter;
    struct iphdr* ipHdr;
    vector_t v;
    vector_functor_t f = {
        vector_dummy_dup,
        NULL
    };

    if (FD_ISSET(remotefd, set)) accept_and_check(remotefd);
    iter = active_vector_begin(&this.clients);
    vector_init(&v, f);
    while (!active_vector_is_end(iter))
    {
        void* buffer;
        unsigned short len;
        int sys;
        client_t* client = iter.data;
        if (FD_ISSET(client->fd, set))
        {
            ssize_t rc;
            msg = NULL;
            buffer = NULL;
            rc = read_msg(client->fd, &msg);
            if (rc == 0)
            {
                vector_push_back(&v, (void*)(long)active_vector_iterator_idx(iter), sizeof(active_vector_iterator_idx(iter)));
            }
            else if (rc > 0 && parse_msg(msg, &sys, &buffer, &len))
            {
                if (sys) server_process_sys(client, msg, buffer, len);
                else printf("write local length: %ld\n", write_n(localfd, buffer, len));
                active_vector_up(&this.clients, active_vector_iterator_idx(iter));
            }
            if (msg) free(msg);
            if (buffer) free(buffer);
        }
        iter = active_vector_next(iter);
    }
    remove_clients(&v, "closed by");
    vector_free(&v);
    if (FD_ISSET(localfd, set))
    {
        unsigned char buffer[2048];
        ssize_t readen;

        readen = read(localfd, buffer, sizeof(buffer));
        if (readen > 0)
        {
            ssize_t idx;
            ipHdr = (struct iphdr*)buffer;
            idx = active_vector_lookup(&this.clients, compare_clients_by_ip, (void*)(long)ipHdr->daddr, sizeof(ipHdr->daddr));
            if (idx >= 0)
            {
                client_t* client;
                size_t len;
                active_vector_get(&this.clients, idx, (void**)&client, &len);
                msg = new_msg(buffer, readen);
                if (msg)
                {
                    write_n(client->fd, msg, sizeof(msg_t) + msg_data_length(msg));
                    free(msg);
                    printf("send msg length: %lu\n", msg_data_length(msg));
                }
            }
        }
    }
}

void server_loop(int remotefd, int localfd)
{
    fd_set set;
    int max;
    vector_t v;
    vector_functor_t f = {
        vector_dummy_dup,
        NULL
    };

    vector_init(&v, f);
    while (1)
    {
        struct timeval tv = {3, 0};
        active_vector_iterator_t iter;

        FD_ZERO(&set);
        FD_SET(remotefd, &set);
        FD_SET(localfd, &set);
        max = remotefd > localfd ? remotefd : localfd;
        iter = active_vector_begin(&this.clients);
        while (!active_vector_is_end(iter))
        {
            int fd = ((client_t*)iter.data)->fd;
            FD_SET(fd, &set);
            if (fd > max) max = fd;
            iter = active_vector_next(iter);
        }

        max = select(max + 1, &set, NULL, NULL, &tv);
        if (max > 0) server_process(max, &set, remotefd, localfd);

        iter = active_vector_begin(&this.clients);
        while (!active_vector_is_end(iter))
        {
            if ((time(NULL) - ((client_t*)iter.data)->keepalive) > KEEPALIVE_LIMIT)
                vector_push_back(&v, (void*)(long)active_vector_iterator_idx(iter), sizeof(active_vector_iterator_idx(iter)));
            iter = active_vector_next(iter);
        }
        remove_clients(&v, "keepalive timeouted");
    }
    vector_free(&v);
}
