﻿#ifndef _MSG_H_
#define _MSG_H_

#include "../struct/link.h"

#define SYS_LOGIN                    1 // req带本地虚拟IP，服务端收到req时查看IP是否存在，并挑一个未被使用的IP返回
#define SYS_PING                     2 // 不带data

#define MSG_PROCESS_COMPRESS_HANDLER 1
#define MSG_PROCESS_ENCRYPT_HANDLER  2

#define MSG_COMPRESS_GZIP_ID         1

#define MSG_ENCRYPT_AES_ID           1
#define MSG_ENCRYPT_DES_ID           2

#define MSG_OP_LEN                   3
#define MSG_OP_MASK                  LEN2MASK(MSG_OP_LEN)

#define MSG_MAX_TTL 200

#define SYSCONTROL_MASK(op, mask1, mask2) (((op & MSG_OP_MASK) << 2) | ((mask1 & 1) << 1) | (mask2 & 1))
#define MAKE_SYS_OP(op, req) (((op & MSG_OP_MASK) << 1) | (req & 1))
#define CHECK_SYS_OP(src, op, req) (((src >> 1) & MSG_OP_MASK) == op && (src & 1) == req)
#define GET_SYS_OP(src) ((src >> 1) & MSG_OP_MASK)
#define IS_SYS_REQUEST(src) (src & 1)
#define IS_SYS_REPLY(src) ((src & 1) == 0)

#pragma pack(1)
typedef struct
{
    unsigned char syscontrol : 1; // 是否是系统消息
    unsigned char compress   : 4; // 压缩算法
    unsigned char encrypt    : 3; // 加密算法
    unsigned int  ident;          // 序号
    unsigned int  sec;            // 发包时间
    unsigned int  usec   : 20;    // 精确到微秒，little-endian
    unsigned int  len    : 12;    // 长度 = len * 16 + pfx
    unsigned char pfx    : 4;     // pfx
    unsigned char sys    : 4;     // for sys msg
    struct
    {
        unsigned short unused : 1;  // 未使用
        unsigned short clip   : 1;  // 是否需要分片
        unsigned short last   : 1;  // 是否是最后一个分片
        unsigned short idx    : 13; // 分片位置
    } zone;
    unsigned short checksum;       // 校验和
    unsigned char  data[];         // 数据
} msg_t;

typedef struct
{
    unsigned short major_version;
    unsigned char  minor_version : 4;
    unsigned char  revision_version : 4;
    unsigned int   ip;
    unsigned int   gateway;
    unsigned char  mask;
    unsigned short internal_mtu;
    unsigned char  signature[31];
    unsigned char  dhcp;
} sys_login_msg_t;
#pragma pack()

typedef struct
{
    int (*do_handler)(const void*, const unsigned int, void**, unsigned int*);
    int (*undo_handler)(const void*, const unsigned int, void**, unsigned int*);
    size_t room_id;
} msg_process_handler_t;

typedef struct {
    unsigned int   ident;
    unsigned short idx;
    unsigned char  used;
} msg_state_t;

extern link_t msg_process_handlers;

extern int gzip_compress(const void* src, const unsigned int src_len, void** dst, unsigned int* dst_len);
extern int gzip_decompress(const void* src, const unsigned int src_len, void** dst, unsigned int* dst_len);
extern int aes_encrypt(const void* src, const unsigned int src_len, void** dst, unsigned int* dst_len);
extern int aes_decrypt(const void* src, const unsigned int src_len, void** dst, unsigned int* dst_len);
extern int des_encrypt(const void* src, const unsigned int src_len, void** dst, unsigned int* dst_len);
extern int des_decrypt(const void* src, const unsigned int src_len, void** dst, unsigned int* dst_len);

extern void init_msg_process_handler();
extern int append_msg_process_handler(
    int type,
    int id,
    size_t room_id,
    int (*do_handler)(const void*, const unsigned int, void**, unsigned int*),
    int (*undo_handler)(const void*, const unsigned int, void**, unsigned int*)
);
extern size_t msg_data_length(const msg_t* msg);
extern int process_asc(void* src, unsigned int src_len, void** dst, unsigned int* dst_len, int* want_free, size_t* room_id);
extern msg_t* new_login_msg(unsigned int ip, unsigned int gateway, unsigned char mask, unsigned char request, unsigned char dhcp, const unsigned char signature[31]);
extern msg_t* new_keepalive_msg(unsigned char request);
extern int parse_msg(const msg_t* input, int* sys, void** output, unsigned short* output_len, size_t* room_id);
extern int parse_login_reply_msg(const msg_t* input, sys_login_msg_t** login, size_t* room_id);

#endif
