#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <syslog.h>

#include "library.h"

#define LEN2MASK(len) ((1 << (len)) - 1)

#define SWAP(a, b) \
({ \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    (void)(&_a == &_b); \
    _a = a; \
    a = b; \
    b = _a; \
})

#define MIN(a, b) \
({ \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    (void)(&_a == &_b); \
    a < b ? a : b; \
})

#define MAX(a, b) \
({ \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    (void)(&_a == &_b); \
    a > b ? a : b; \
})

#define SYSTEM_NORMAL(cmd) \
do {\
    if (system(cmd) == -1) \
    { \
        perror("system"); \
    } \
} while (0)

#define SYSTEM_EXIT(cmd) \
do {\
    if (system(cmd) == -1) \
    { \
        perror("system"); \
        exit(1); \
    } \
} while (0)

#define SYSLOG(level, arg...) \
do { \
if (this.log_level >= level) syslog(level, ##arg); \
} while(0)

#define ROUND_UP(bytes, align) (((bytes) + (align) - 1) & ~((align) - 1))
#define ROUND_DOWN(bytes, align) ((bytes) & ~((align) - 1))

extern unsigned short checksum(void* buffer, size_t len);

extern uint32_t little32(uint32_t n);
extern uint16_t little16(uint16_t n);
extern uint32_t big32(uint32_t n);
extern uint16_t big16(uint16_t n);
extern void cpy_net32(uint32_t src, uint32_t* dst);
extern void cpy_net16(uint16_t src, uint16_t* dst);
extern uint32_t little2host32(uint32_t n);
extern uint16_t little2host16(uint16_t n);

#endif
