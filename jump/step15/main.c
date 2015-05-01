#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if defined(unix) && defined(HAVE_LINUX_TCP_H)
#include <linux/tcp.h>
#endif

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include "getopt.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include <openssl/aes.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "library/common.h"
#include "library/library.h"
#include "library/script.h"

#include "network/network.h"

#include "main.h"

#ifdef HAVE_EXECINFO_H
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
        syslog(LOG_ERR, "%s\n", strings[i]);
    }

    free(strings);
    exit(1);
}
#endif

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
#ifdef HAVE_EXECINFO_H
    signal(SIGSEGV, crash_sig);
    signal(SIGABRT, crash_sig);
    signal(SIGPIPE, SIG_IGN);
#endif

#ifdef WIN32
    HANDLE localfd;
    WSADATA wsa;
    enum_device_t devs[MAX_DEVICE_COUNT];
#else
    int localfd;
#endif
    char cmd[1024];
    int remotefd;
    library_conf_t conf;
    int opt;
    struct in_addr a;

    struct option long_options[] = {
        { "conf", 1, NULL, 'c' },
        { NULL,   0, NULL,  0  }
    };
    char short_options[512] = {0};
    longopt2shortopt(long_options, sizeof(long_options) / sizeof(struct option), short_options);
#ifdef HAVE_SYSLOG_H
    openlog(argv[0], LOG_PERROR | LOG_CONS | LOG_PID, LOG_LOCAL0);
#endif

    qtun = calloc(sizeof(*qtun), 1);

#ifdef WIN32
    remotefd = -1;
    localfd = INVALID_HANDLE_VALUE;
#else
    localfd = remotefd = -1;
#endif
    {
        char path[MAX_PATH] = {0};
#ifdef WIN32
        strcpy(path, argv[0]);
#elif defined(__APPLE__)
        char tmp_path[sizeof(path)] = {0};
        uint32_t len = sizeof(path);
        if (_NSGetExecutablePath(tmp_path, &len) == -1) {
            perror("_NSGetExecutablePath");
            return 1;
        }
        if (readlink(tmp_path, path, sizeof(path)) == -1) {
            if (errno == EINVAL) strcpy(path, tmp_path);
            else {
                perror("readlink");
                return 1;
            }
        }
#else
        if (readlink("/proc/self/exe", path, sizeof(path)) == -1)
        {
            perror("readlink");
            return 1;
        }
#endif
        init_path(path);
    }
    conf_init(&conf);

    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'c':
            {
                char* path = realpath(optarg, NULL);
                if (path == NULL) {
                    perror("realpath");
                    return 1;
                }
                strcpy(conf.conf_file, path);
                free(path);
            }
            break;
        default:
            fprintf(stderr, "param error\n");
            return 1;
        }
    }

#ifdef WIN32
    {
        size_t count = enum_devices(devs);
        if (count == 0)
        {
            fprintf(stderr, "have no QTun Virtual Adapter\n");
            return 1;
        }
        else if (count == 1)
        {
            strcpy(conf.dev_symbol, devs[0].dev_path);
            strcpy(conf.dev_name, devs[0].dev_name);
        }
        else
        {
            size_t i;
            char str[20] = { 0 };
            int n = -1;
            printf("Have Adapters:\n");
            for (i = 0; i < count; ++i)
            {
                printf("%lu: %s\n", i + 1, devs[i].dev_name);
            }
            printf("Choose One[1]: ");
            while (n == -1)
            {
                if (str[0] == '\n' && str[1] == 0) n = 1;
                else
                {
                    if (!is_int(str, sizeof(str))) continue;
                    n = atoi(str);
                    if (n < 1 || n > (int)count)
                    {
                        fprintf(stderr, "Invalid Number must >= 1 and <= %lu\n", count);
                        n = -1;
                        continue;
                    }
                }
            }
            strcpy(conf.dev_symbol, devs[n].dev_path);
            strcpy(conf.dev_name, devs[n].dev_name);
        }
    }
#endif
    
    init_lua();
    show_logo();
    script_load_config(qtun->lua, &conf, conf.conf_file);

#ifdef WIN32
    if (strlen(conf.dev_symbol) == 0)
    {
        fprintf(stderr, "Missing param [-e] or [--device]\n");
        return 1;
    }
#endif

#ifdef WIN32
    localfd = tun_open(conf.dev_symbol);
    if (localfd == INVALID_HANDLE_VALUE) return 1;
    fprintf(stdout, "%s opened\n", conf.dev_name);
#else
    memset(qtun->dev_name, 0, IFNAMSIZ);
    localfd = tun_open(qtun->dev_name);
    if (localfd == -1) return 1;
    syslog(LOG_INFO, "%s opened\n", qtun->dev_name);
#endif
    a.s_addr = conf.localip;

#ifdef WIN32
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    if (strlen(conf.server) == 0)
    {
        if (conf.netmask == 0 || conf.netmask > 31)
        {
#ifdef WIN32
            WSACleanup();
#endif

            fprintf(stderr, "netmask must > 0 and <= 31\n");
            return 1;
        }
        library_init(conf);
        if (conf.localip == 0)
        {
            fprintf(stderr, "localip is zero\n");
            return 1;
        }
        remotefd = bind_and_listen(conf.server_port);
        if (remotefd == -1)
        {
#ifdef WIN32
            WSACleanup();
#endif
            return 1;
        }
#ifdef WIN32
        {
            a.s_addr = conf.localip;
            sprintf(cmd, "netsh interface ip set address name=\"%s\" static %s %s", conf.dev_name, inet_ntoa(a), STR_LEN2MASK(conf.netmask));
            SYSTEM_EXIT(cmd);
        }
#elif defined(__APPLE__)
        {
            sprintf(cmd, "ifconfig %s %s/%u up", qtun->dev_name, inet_ntoa(a), conf.netmask);
            SYSTEM_EXIT(cmd);
            a.s_addr = conf.localip & LEN2MASK(conf.netmask);
            sprintf(cmd, "route add -net %s/%u %s", inet_ntoa(a), conf.netmask, inet_ntoa(a));
            SYSTEM_EXIT(cmd);
        }
#else
        {
            sprintf(cmd, "ifconfig %s %s/%u up", qtun->dev_name, inet_ntoa(a), conf.netmask);
            SYSTEM_EXIT(cmd);
            a.s_addr = conf.localip & LEN2MASK(conf.netmask);
            sprintf(cmd, "route add -net %s/%u dev %s", inet_ntoa(a), conf.netmask, qtun->dev_name);
            SYSTEM_EXIT(cmd);
        }
#endif
        server_loop(remotefd, localfd);
    }
    else
    {
#ifdef unix
        unsigned char mask;
#endif
        int inited = 0;
        library_init(conf);

        while (1)
        {
            remotefd = connect_server(conf.server, conf.server_port);
            if (remotefd == -1)
            {
                SLEEP(5);
                continue;
            }
            a.s_addr = qtun->localip;
            if (conf.localip == 0)
            {
                fprintf(stderr, "localip is zero\n");
                return 1;
            }
            if (!inited)
            {
#ifdef WIN32
                {
                    sprintf(cmd, "netsh interface ip set address name=\"%s\" static %s %s", conf.dev_name, inet_ntoa(a), STR_LEN2MASK(conf.netmask));
                    SYSTEM_EXIT(cmd);
                }
#elif defined(__APPLE__)
                {
                    char ip1[16], ip2[16];
                    a.s_addr = qtun->localip;
                    strcpy(ip1, inet_ntoa(a));
                    a.s_addr = qtun->client.local_ip;
                    strcpy(ip2, inet_ntoa(a));
                    sprintf(cmd, "ifconfig %s inet %s %s up", qtun->dev_name, ip1, ip2);
                    SYSTEM_EXIT(cmd);
                    mask = netmask();
                    a.s_addr = qtun->localip & LEN2MASK(mask);
                    sprintf(cmd, "route add -net %s/%u %s", inet_ntoa(a), mask, ip2);
                    SYSTEM_EXIT(cmd);
                }
#else
                {
                    sprintf(cmd, "ifconfig %s %s up", qtun->dev_name, inet_ntoa(a));
                    SYSTEM_EXIT(cmd);
                    mask = netmask();
                    a.s_addr = qtun->localip & LEN2MASK(mask);
                    sprintf(cmd, "route add -net %s/%u dev %s", inet_ntoa(a), mask, qtun->dev_name);
                    SYSTEM_EXIT(cmd);
                }
#endif
                inited = 1;
            }
            client_loop(remotefd, localfd);
            close(remotefd);
            SYSLOG(LOG_WARNING, "retry");
        }
    }
#ifdef WIN32
    WSACleanup();
#endif

#ifdef HAVE_SYSLOG_H
    closelog();
#endif

    library_free();
    return 0;
}

