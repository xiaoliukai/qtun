#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef unix
#  ifdef HAVE_LINUX_TCP_H
#    include <linux/tcp.h>
#  elif HAVE_NETINET_TCP_H
#    include <netinet/tcp.h>
#  endif
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../library/common.h"
#include "../library/library.h"
#include "../library/proto.h"
#include "../library/script.h"
#include "../version.h"

#include "network.h"

#define RETURN_OK                 0
#define RETURN_CONNECTION_CLOSED -1
#define RETURN_READ_ERROR        -2

int connect_server(char* host, unsigned short port)
{
    fd_type fd;
    int rc;
    struct hostent* he;
    struct sockaddr_in addr = {0};
    msg_t* msg;
#ifdef WIN32
    char flag = 1;
#else
    int flag = 1;
#endif

    qtun->client.fd = qtun->remotefd = fd = socket(AF_INET, qtun->use_udp ? SOCK_DGRAM : SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if ((he = gethostbyname(host)) == NULL)
    {
        SYSLOG(LOG_ERR, "Convert ip address error");
        close(fd);
        return -1;
    }
    addr.sin_addr = *(struct in_addr*)he->h_addr_list[0];

    qtun->client.remote_ip = addr.sin_addr.s_addr;
    qtun->client.addr = addr;

    if (!qtun->use_udp)
    {        
        rc = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
        if (rc == -1)
        {
            perror("connect");
            close(fd);
            return -1;
        }
    }

    if (!qtun->use_udp && setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == -1)
    {
        perror("setsockopt");
    }

    {
        unsigned char signature[31] = {0};
        script_load_signature(qtun->lua, signature);
        msg = new_login_msg(qtun->localip, 0, 0, 1, 0, signature);
    }
    if (msg)
    {
        write_c(&qtun->client, msg, sizeof(msg_t) + msg_data_length(msg));
        pool_room_free(&qtun->pool, MSG_ROOM_IDX);
        if (read_msg_t(&qtun->client, &msg, 5) > 0)
        {
            hash_functor_t functor = {
                msg_ident_hash,
                msg_ident_compare,
                hash_dummy_dup,
                hash_dummy_dup,
                msg_group_free_hash,
                msg_group_free_hash_val
            };
            sys_login_msg_t* login;
            size_t room_id;
            struct in_addr a;
            if (msg->compress != qtun->compress || msg->encrypt != qtun->encrypt)
            {
                SYSLOG(LOG_ERR, "compress algorithm or encrypt algorithm is not same");
                pool_room_free(&qtun->pool, RECV_ROOM_IDX);
                goto end;
            }
            if (!parse_login_reply_msg(msg, &login, &room_id)) goto end;
            pool_room_free(&qtun->pool, RECV_ROOM_IDX);
            if (login->major_version != QTUN_MAJOR_VERSION ||
                login->minor_version != QTUN_MINOR_VERSION ||
                login->revision_version != QTUN_REVISION_VERSION) {
                SYSLOG(LOG_ERR, "Invalid Version: Server [%d.%d.%d] Client [%d.%d.%d]",
                       login->major_version, login->minor_version, login->revision_version,
                       QTUN_MAJOR_VERSION, QTUN_MINOR_VERSION, QTUN_REVISION_VERSION);
                pool_room_free(&qtun->pool, room_id);
                goto end;
            }
            if (login->ip == 0)
            {
                SYSLOG(LOG_ERR, "Not enough ip address");
                pool_room_free(&qtun->pool, room_id);
                goto end;
            }
            if (!script_signature_verify(qtun->lua, login->signature)) {
                SYSLOG(LOG_ERR, "invalid signature");
                pool_room_free(&qtun->pool, room_id);
                goto end;
            }
            if (login->ip != qtun->localip) {
                if (login->dhcp) {
                    qtun->localip = login->ip;
                } else {
                    char saddr[16], daddr[16];
                    a.s_addr = qtun->localip;
                    strcpy(saddr, inet_ntoa(a));
                    a.s_addr = login->ip;
                    strcpy(daddr, inet_ntoa(a));
                    SYSLOG(LOG_ERR, "%s is inuse, but %s is not inuse", saddr, daddr);
                    pool_room_free(&qtun->pool, room_id);
                    goto end;
                }
            }
            hash_init(&qtun->client.recv_msg_groups, functor, 11);
            qtun->client.local_ip = login->gateway;
            qtun->client.fd = fd;
            qtun->client.internal_mtu = ntohs(login->internal_mtu);
            qtun->client.max_length = ROUND_UP(qtun->client.internal_mtu - sizeof(msg_t) - sizeof(struct iphdr) - (qtun->use_udp ? sizeof(struct udphdr) : sizeof(struct tcphdr)), 8);
            if (qtun->use_udp)
            {
                qtun->recv_buffer_len = qtun->client.max_length + sizeof(msg_t);
                qtun->recv_buffer = pool_room_realloc(&qtun->pool, RECV_ROOM_IDX, qtun->recv_buffer_len);
                if (qtun->recv_buffer == NULL)
                {
                    SYSLOG(LOG_INFO, "Not enough memory");
                    pool_room_free(&qtun->pool, room_id);
                    goto end;
                }
            }
            qtun->netmask = login->mask;
            qtun->keepalive = (unsigned int)time(NULL);
#ifdef WIN32 // 将对端内网IP添加到ARP表
            {
                char cmd[1024];
                char str[16];
                a.s_addr = gateway;
                strcpy(str, inet_ntoa(a));
                sprintf(cmd, "arp -s %s ff-ff-ff-ff-ff-ff", str);
                SYSTEM_NORMAL(cmd);
            }
#endif
            return fd;
        }
        SYSLOG(LOG_ERR, "read sys_login_reply message timeouted");
        goto end;
    }
    SYSLOG(LOG_ERR, "Can not create login message");
end:
    close(fd);
    return -1;
}

static void client_process_sys(msg_t* msg)
{
    switch (GET_SYS_OP(msg->sys))
    {
    case SYS_PING:
        if (IS_SYS_REPLY(msg->sys))
        {
            qtun->keepalive_replyed = 1;
            SYSLOG(LOG_INFO, "reply keepalive message received");
        }
        break;
    }
}

static void process_msg(msg_t* msg)
{
    void* buffer = NULL;
    unsigned short len;
    int sys;
    size_t room_id;

    if (!check_msg(&qtun->client, msg)) return;

    if (msg->syscontrol)
    {
        client_process_sys(msg);
    }
    else if (msg->zone.clip)
    {
        if (!process_clip_msg(qtun->localfd, &qtun->client, msg, &room_id)) goto end;
    }
    else if (parse_msg(msg, &sys, &buffer, &len, &room_id))
    {
        ssize_t written;
#ifdef WIN32
        WriteFile(qtun->localfd, buffer, len, &written, NULL);
#else
        written = write(qtun->localfd, buffer, len);
#endif
        SYSLOG(LOG_INFO, "write local length: %ld", written);
    }
    else
        SYSLOG(LOG_WARNING, "Parse message error");
end:
    if (buffer) pool_room_free(&qtun->pool, room_id);
    if (!qtun->use_udp)
    {
        qtun->client.status = (qtun->client.status & ~CLIENT_STATUS_WAITING_BODY) | CLIENT_STATUS_WAITING_HEADER;
        qtun->client.want = sizeof(msg_t);
        qtun->client.read = qtun->client.buffer;
        qtun->client.buffer_len = qtun->client.want;
    }
    ++qtun->msg_ttl;
}

static int client_process(int max, fd_set* set)
{
    msg_group_t* group;
    if (LOCAL_HAVE_DATA(set))
    {
        unsigned char buffer[2048];
        ssize_t readen;

#ifdef WIN32
        ReadFile(qtun->localfd, buffer, sizeof(buffer), &readen, NULL);
#else
        readen = read(qtun->localfd, buffer, sizeof(buffer));
#endif
        if (readen > 0)
        {
            group = new_msg_group(buffer, (unsigned short)readen);
            if (group)
            {
                ssize_t written = send_msg_group(&qtun->client, group);
                msg_group_free(group);
                SYSLOG(LOG_INFO, "send msg length: %ld", written);
            }
        }
    }
    if (FD_ISSET(qtun->remotefd, set))
    {
        ssize_t rc = qtun->use_udp ? udp_read(qtun->remotefd, qtun->recv_buffer, qtun->recv_buffer_len, NULL, NULL)
                                  : read_pre(qtun->remotefd, qtun->client.read, qtun->client.want);
        if (rc == 0)
        {
            SYSLOG(LOG_ERR, "connection closed");
            return RETURN_CONNECTION_CLOSED;
        }
        else if (rc < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return RETURN_OK;
            perror("read");
            return RETURN_READ_ERROR;
        }
        else if (qtun->use_udp) // use udp
        {
            process_msg((msg_t*)qtun->recv_buffer);
        }
        else // use tcp
        {
            qtun->client.read += rc;
            qtun->client.want -= rc;
            if (qtun->client.want == 0)
            {
                if (IS_CLIENT_STATUS_WAITING_HEADER(qtun->client.status))
                {
                    size_t len = msg_data_length((msg_t*)qtun->client.buffer);
                    if (len)
                    {
                        msg_t* msg = (msg_t*)qtun->client.buffer;
                        qtun->client.status = (qtun->client.status & ~CLIENT_STATUS_WAITING_HEADER) | CLIENT_STATUS_WAITING_BODY;
                        if (msg->zone.clip)
                        {
                            if (msg->zone.last && (len % qtun->client.max_length)) qtun->client.want = len % qtun->client.max_length;
                            else qtun->client.want = qtun->client.max_length;
                        }
                        else qtun->client.want = len;
                        qtun->client.buffer_len = sizeof(msg_t) + qtun->client.want;
                        qtun->client.buffer = pool_room_realloc(&qtun->pool, RECV_ROOM_IDX, qtun->client.buffer_len);
                        if (qtun->client.buffer == NULL)
                        {
                            SYSLOG(LOG_ERR, "Not enough memory");
                            exit(1);
                        }
                        qtun->client.read = ((msg_t*)qtun->client.buffer)->data;
                    }
                    else process_msg((msg_t*)qtun->client.buffer);
                }
                else process_msg((msg_t*)qtun->client.buffer);
            }
        }
    }
    return RETURN_OK;
}

void client_loop(fd_type remotefd, local_fd_type localfd)
{
    fd_set set;
    int max;
    int keepalive_send = 0;
    int rc;

    qtun->remotefd = remotefd;
    qtun->localfd = localfd;
    if (!qtun->use_udp)
    {
        qtun->client.status = CLIENT_STATUS_NORMAL | CLIENT_STATUS_WAITING_HEADER;
        qtun->client.want = sizeof(msg_t);
        qtun->client.buffer = qtun->client.read = pool_room_realloc(&qtun->pool, RECV_ROOM_IDX, qtun->client.want);
        qtun->client.buffer_len = qtun->client.want;
    }

    qtun->keepalive_replyed = 1;
    while (1)
    {
        struct timeval tv = {0, 1};
        FD_ZERO(&set);
        FD_SET(remotefd, &set);
#ifdef WIN32
        max = remotefd;
#else
        FD_SET(localfd, &set);
        max = remotefd > localfd ? remotefd : localfd;
#endif

        if (qtun->keepalive_replyed && (time(NULL) - qtun->keepalive) > KEEPALIVE_INTERVAL)
        {
            msg_t* msg = new_keepalive_msg(1);
            write_c(&qtun->client, msg, sizeof(msg_t));
            SYSLOG(LOG_INFO, "send keepalive message");
            qtun->keepalive = (unsigned int)time(NULL);
            qtun->keepalive_replyed = 0;
            pool_room_free(&qtun->pool, MSG_ROOM_IDX);
            keepalive_send = 1;
        }

        max = select(max + 1, &set, NULL, NULL, &tv);
        rc = client_process(max, &set);
        switch (rc)
        {
        case RETURN_CONNECTION_CLOSED:
        case RETURN_READ_ERROR:
            pool_room_free(&qtun->pool, RECV_ROOM_IDX);
            return;
        }

        if (keepalive_send && !qtun->keepalive_replyed && (time(NULL) - qtun->keepalive) > KEEPALIVE_TIMEOUT)
        {
            SYSLOG(LOG_INFO, "keepalive reply timeouted, connection closed");
            pool_room_free(&qtun->pool, RECV_ROOM_IDX);
            return;
        }
        checkout_ttl(&qtun->client.recv_msg_groups);
    }
}

