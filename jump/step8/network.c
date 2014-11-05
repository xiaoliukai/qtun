#include <arpa/inet.h>
#include <linux/icmp.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "library.h"
#include "msg.h"
#include "network.h"

static ssize_t read_msg(int fd, msg_t** msg)
{
    ssize_t rc;
    size_t len;

    *msg = malloc(sizeof(msg_t));
    if (*msg == NULL) return -2;
    rc = read_n(fd, *msg, sizeof(**msg));
    if (rc <= 0) return rc;
    len = msg_data_length(*msg);
    *msg = realloc(*msg, sizeof(msg_t) + len);
    if (*msg == NULL) return -2;
    rc = read_n(fd, (*msg)->data, len);

    if (checksum(*msg, sizeof(msg_t) + len))
    {
        fprintf(stderr, "Invalid msg\n");
        free(*msg);
        *msg = NULL;
        return -2;
    }

    printf("read msg length: %lu\n", len);
    return rc;
}

static ssize_t read_msg_t(int fd, msg_t** msg, double timeout)
{
    ssize_t rc;
    size_t len;

    *msg = malloc(sizeof(msg_t));
    if (*msg == NULL) return -2;
    rc = read_t(fd, *msg, sizeof(**msg), timeout);
    if (rc <= 0) return rc;
    len = msg_data_length(*msg);
    *msg = realloc(*msg, sizeof(msg_t) + len);
    if (*msg == NULL) return -2;
    rc = read_t(fd, (*msg)->data, len, timeout);
    if (rc <= 0) return rc;

    if (checksum(*msg, sizeof(msg_t) + len))
    {
        fprintf(stderr, "Invalid msg\n");
        free(*msg);
        *msg = NULL;
        return -2;
    }

    printf("read msg length: %lu\n", len);
    return rc;
}

int tun_open(char name[IFNAMSIZ])
{
    struct ifreq ifr;
    int fd;

    if ((fd = open("/dev/net/tun", O_RDWR)) == -1)
    {
        perror("open");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, name, IFNAMSIZ);

    if (ioctl(fd, TUNSETIFF, (void *) &ifr) == -1)
    {
        perror("ioctl");
        close(fd);
        return -1;
    }

    strncpy(name, ifr.ifr_name, IFNAMSIZ);
    return fd;
}

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

int connect_server(char* ip, unsigned short port)
{
    int fd, rc;
    struct sockaddr_in addr = {0};
    msg_t* msg;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_aton(ip, &addr.sin_addr) == 0)
    {
        fprintf(stderr, "Convert ip address error!\n");
        close(fd);
        return -1;
    }

    rc = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (rc == -1)
    {
        perror("connect");
        close(fd);
        return -1;
    }

    msg = new_login_msg(this.localip, 0, 1);
    if (msg)
    {
        write_n(fd, msg, sizeof(msg_t) + msg_data_length(msg));
        free(msg);
        if (read_msg_t(fd, &msg, 5) > 0)
        {
            unsigned int ip;
            unsigned char mask;
            if (msg->compress != this.compress || msg->encrypt != this.encrypt)
            {
                fprintf(stderr, "compress algorithm or encrypt algorithm is not same\n");
                free(msg);
                goto end;
            }
            if (!parse_login_reply_msg(msg, &ip, &mask)) goto end;
            free(msg);
            if (ip == 0)
            {
                fprintf(stderr, "Not enough ip address\n");
                goto end;
            }
            if (ip != this.localip)
            {
                struct in_addr a;
                char saddr[16], daddr[16];
                a.s_addr = this.localip;
                strcpy(saddr, inet_ntoa(a));
                a.s_addr = ip;
                strcpy(daddr, inet_ntoa(a));
                fprintf(stderr, "%s is inuse, but %s is not inuse\n", saddr, daddr);
                goto end;
            }
            return fd;
        }
        fprintf(stderr, "read sys_login_reply message timeouted\n");
    }
    fprintf(stderr, "Not enough memory\n");
end:
    close(fd);
    return -1;
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
        void* value;
        size_t value_len;
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
        if (login->ip == this.localip || hash_get(&this.hash_ip, (void*)(long)login->ip, sizeof(login->ip), &value, &value_len)) // IP已被占用
        {
            unsigned short i;
            unsigned int localip = login->ip & LEN2MASK(this.netmask);
            msg_t* new_msg;
            for (i = 1; i < LEN2MASK(32 - this.netmask); ++i)
            {
                unsigned int newip = (i << this.netmask) | localip;
                if (!hash_get(&this.hash_ip, (void*)(long)newip, sizeof(localip), &value, &value_len))
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
            msg->unused = MAKE_SYS_OP(SYS_LOGIN, 0);
            ((sys_login_msg_t*)msg->data)->mask = this.netmask;
            msg->checksum = 0;
            msg->checksum = checksum(msg, sizeof(msg_t) + msg_data_length(msg));
            if (!hash_set(&this.hash_ip, (void*)(long)login->ip, sizeof(login->ip), (void*)(long)fd, sizeof(fd)))
            {
                fprintf(stderr, "set to hash_ip error\n");
                close(fd);
                goto end;
            }
            write_n(fd, msg, sizeof(msg_t) + msg_data_length(msg));
        }
    }
    else
        fprintf(stderr, "read sys_login_request message error\n");
end:
    if (msg) free(msg);
    if (data) free(data);
}

static void server_process(int max, fd_set* set, int remotefd, int localfd)
{
    msg_t* msg;
    hash_iterator_t iter;
    struct iphdr* ipHdr;
    void* value;
    size_t value_len;

    if (FD_ISSET(remotefd, set)) accept_and_check(remotefd);
    iter = hash_begin(&this.hash_ip);
    while (!hash_is_end(iter))
    {
        void* buffer;
        unsigned short len;
        int sys;
        int fd = hash2fd(iter.data.val);
        if (FD_ISSET(fd, set))
        {
            msg = NULL;
            buffer = NULL;
            if (read_msg(fd, &msg) > 0 && parse_msg(msg, &sys, &buffer, &len))
            {
                if (sys) ;
                else printf("write local length: %ld\n", write_n(localfd, buffer, len));
            }
            if (msg) free(msg);
            if (buffer) free(buffer);
        }
        iter = hash_next(&this.hash_ip, iter);
    }
    if (FD_ISSET(localfd, set))
    {
        unsigned char buffer[2048];
        ssize_t readen;

        readen = read(localfd, buffer, sizeof(buffer));
        if (readen > 0)
        {
            ipHdr = (struct iphdr*)buffer;
            if (hash_get(&this.hash_ip, (void*)(long)ipHdr->daddr, sizeof(ipHdr->daddr), &value, &value_len))
            {
                msg = new_msg(buffer, readen);
                if (msg)
                {
                    write_n(hash2fd(value), msg, sizeof(msg_t) + msg_data_length(msg));
                    free(msg);
                    printf("send msg length: %lu\n", msg_data_length(msg));
                }
            }
        }
    }
}

static void client_process(int max, fd_set* set, int remotefd, int localfd)
{
    msg_t* msg;
    if (FD_ISSET(localfd, set))
    {
        unsigned char buffer[2048];
        ssize_t readen;

        readen = read(localfd, buffer, sizeof(buffer));
        if (readen > 0)
        {
            msg = new_msg(buffer, readen);
            if (msg)
            {
                write_n(remotefd, msg, sizeof(msg_t) + msg_data_length(msg));
                free(msg);
                printf("send msg length: %lu\n", msg_data_length(msg));
            }
        }
    }
    if (FD_ISSET(remotefd, set))
    {
        int sys;
        void* buffer;
        unsigned short len;

        msg = NULL;
        buffer = NULL;
        if (read_msg(remotefd, &msg) > 0 && parse_msg(msg, &sys, &buffer, &len))
        {
            if (sys) ;
            else printf("write local length: %ld\n", write_n(localfd, buffer, len));
        }
        else
        {
            fprintf(stderr, "read error\n");
            close(remotefd);
            exit(1);
        }
        if (msg) free(msg);
        if (buffer) free(buffer);
    }
}

void server_loop(int remotefd, int localfd)
{
    fd_set set;
    int max;
    while (1)
    {
        struct timeval tv = {60, 0};
        hash_iterator_t iter;

        FD_ZERO(&set);
        FD_SET(remotefd, &set);
        FD_SET(localfd, &set);
        max = remotefd > localfd ? remotefd : localfd;
        iter = hash_begin(&this.hash_ip);
        while (!hash_is_end(iter))
        {
            int fd = hash2fd(iter.data.val);
            FD_SET(fd, &set);
            if (fd > max) max = fd;
            iter = hash_next(&this.hash_ip, iter);
        }

        max = select(max + 1, &set, NULL, NULL, &tv);
        if (max > 0) server_process(max, &set, remotefd, localfd);
    }
}

void client_loop(int remotefd, int localfd)
{
    fd_set set;
    int max;
    while (1)
    {
        struct timeval tv = {60, 0};
        FD_ZERO(&set);
        FD_SET(remotefd, &set);
        FD_SET(localfd, &set);
        max = remotefd > localfd ? remotefd : localfd;

        max = select(max + 1, &set, NULL, NULL, &tv);
        if (max > 0) client_process(max, &set, remotefd, localfd);
    }
}

ssize_t read_n(int fd, void* buf, size_t count)
{
    char* ptr = buf;
    size_t left = count;
    while (left)
    {
        ssize_t readen = read(fd, ptr, left);
        if (readen == 0) return 0;
        else if (readen == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            return -1;
        }
        ptr  += readen;
        left -= readen;
    }
    return count;
}

ssize_t write_n(int fd, const void* buf, size_t count)
{
    const char* ptr = buf;
    size_t left = count;
    while (left)
    {
        ssize_t written = write(fd, ptr, left);
        if (written == 0) return 0;
        else if (written == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            return -1;
        }
        ptr  += written;
        left -= written;
    }
    return count;
}

ssize_t read_t(int fd, void* buf, size_t count, double timeout)
{
    fd_set set;
    struct timeval tv = {(long)timeout, (long)(timeout * 1000000) % 1000000};
    int rc;
    FD_ZERO(&set);
    FD_SET(fd, &set);
    rc = select(fd + 1, &set, NULL, NULL, &tv);
    switch (rc)
    {
    case -1:
        return rc;
    case 0:
        errno = EAGAIN;
        return -1;
    default:
        return read(fd, buf, count);
    }
}