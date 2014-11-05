#include <arpa/inet.h>
#include <openssl/aes.h>

#include <execinfo.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "library.h"
#include "network.h"
#include "main.h"

#define SYSTEM(cmd) \
do {\
    if (system(cmd) == -1) \
    { \
        perror("system"); \
        exit(1); \
    } \
} while (0)

static void crash_sig(int signum)
{
    void* array[10];
    size_t size;
    char** strings;
    size_t i;

    signal(signum, SIG_DFL);

    size = backtrace(array, sizeof(array) / sizeof(void*));
    strings = (char**)backtrace_symbols(array, size);

    for (i = 0; i < size; ++i)
    {
        fprintf(stderr, "%s\n", strings[i]);
    }

    free(strings);
    exit(1);
}

static void longopt2shortopt(struct option* long_options, size_t count, char* short_options)
{
    size_t i;
    char* ptr = short_options;
    for (i = 0; i < count; ++i)
    {
        if (long_options[i].val)
        {
            *ptr++ = long_options[i].val;
            if (long_options[i].has_arg) *ptr++ = ':';
        }
    }
    *ptr = 0;
}

int main(int argc, char* argv[])
{
    signal(SIGSEGV, crash_sig);
    signal(SIGABRT, crash_sig);

    char name[IFNAMSIZ];
    char cmd[1024];
    int localfd, remotefd;
    library_conf_t conf;
    int opt;
    char* ip = NULL;
    struct in_addr a;

    struct option long_options[] = {
        {"aes",     1, NULL, 'a'},
        {"des",     1, NULL, 'd'},
        {"gzip",    0, NULL, 'g'},
        {"mask",    1, NULL, 'm'},
        {"localip", 1, NULL, 'l'},
        {"server",  1, NULL, 's'},
        {NULL,      0, NULL,   0}
    };
    char short_options[512] = {0};
    longopt2shortopt(long_options, sizeof(long_options) / sizeof(struct option), short_options);

    this.msg_ident    = 0;
    conf.localip      = 0;
    conf.netmask      = 24;
    conf.use_gzip     = 0;
    conf.use_aes      = 0;
    conf.aes_key_file = NULL;
    conf.use_des      = 0;
    conf.des_key_file = NULL;

    memset(&conf, 0, sizeof(conf));
    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'a':
            conf.use_aes = 1;
            conf.aes_key_file = optarg;
            break;
        case 'd':
            conf.use_des = 1;
            conf.des_key_file = optarg;
            break;
        case 'g':
            conf.use_gzip = 1;
            break;
        case 'l':
            conf.localip = inet_addr(optarg);
            break;
        case 's':
            ip = optarg;
            break;
        case 'm':
            conf.netmask = atoi(optarg);
            break;
        default:
            fprintf(stderr, "param error\n");
            return 1;
        }
    }

    if (conf.localip == 0)
    {
        fprintf(stderr, "localip is zero\n");
        return 1;
    }

    memset(name, 0, IFNAMSIZ);
    localfd = tun_open(name);
    if (localfd == -1) return 1;
    fprintf(stdout, "%s opened\n", name);
    a.s_addr = conf.localip;

    if (ip == NULL)
    {
        if (conf.netmask == 0 || conf.netmask > 31)
        {
            fprintf(stderr, "netmask must > 0 and <= 31\n");
            return 1;
        }
        library_init(conf);
        remotefd = bind_and_listen(6687);
        if (remotefd == -1) return 1;
        sprintf(cmd, "ifconfig %s %s up", name, inet_ntoa(a));
        SYSTEM(cmd);
        server_loop(remotefd, localfd);
    }
    else
    {
        library_init(conf);
        remotefd = connect_server(ip, 6687);
        sprintf(cmd, "ifconfig %s %s up", name, inet_ntoa(a));
        SYSTEM(cmd);
        if (remotefd == -1) return 1;
        client_loop(remotefd, localfd);
    }
    return 0;
}