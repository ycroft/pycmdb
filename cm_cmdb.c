/*
 * =====================================================================================
 *
 *       Filename:  cm_cmdb.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2019年01月06日 20时49分05秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yin Yuhao (), yyh18ball@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include "cm_cmdb.h"
#include "stdio.h"
#include "stdlib.h"
// for macro offsetof
#include "stddef.h"

#include "unistd.h"

/* these api can be found in cm_raw_list.h */
/* i can not publish all source code */
struct dl
{
    struct dl * prev;
    struct dl * next;
};

static inline
void list_add(struct dl * head,
        struct dl * node)
{
    head->prev->next = node;
    node->prev = head->prev;
    head->prev = node;
    node->next = head;
}

static inline
void list_del(struct dl * node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

static inline
void list_init(struct dl * node)
{
    node->prev = node;
    node->next = node;
}

#define list_entry(n, cls, m) (cls *)(n - offsetof(cls, m))

/* end of cm_raw_list.h */

/* these api can be found in cm_base.h */
/* i can not publish all source code */
#define CmMalloc        malloc
#define CmFree          free
#define CmObjAlloc(cls) (cls *)CmMalloc(sizeof(cls))

#define CmLogInfo(ret, ...)     printf(__VA_ARGS__)
#define CmLogError(ret, ...)    printf(__VA_ARGS__)
/* end of cm_base.h */

typedef struct
{
    uint32_t ip;        /// ipv4 only, in big ending
    uint16_t port;      /// port
    uint8_t * sndrBuf;  /// message buf
} CM_CMDB_DEAMON;

typedef struct
{
#define DEFAULT_SPACE (64)
    uint32_t space;
    struct dl * bkt;
} CM_CMDB_IDX_HASH;

typedef struct
{
    const char * ws;
    CM_CMDB_DEAMON deamon;
} CM_CMDB_CTX;

char __cmdBuf[1024];

uint32_t CmCmdbCreateDeamon(CM_CMDB_CTX * ctx)
{
    pid_t pid;

    pid = fork();
    if (-1 == pid)
    {
        CmLogError(pid, "fork fail");
        return -1;
    }

    if (0 != pid)
    {
        CmLogInfo(0, "get new pid %d", pid);
        return 0;
    }

    sprintf(__cmdBuf, "%s/cmdeamon.py --workspace %s --addr %u:%u",
            ctx->ws, ctx->ws, ctx->deamon.ip, ctx->deamon.port);

    execlp("python", __cmdBuf, NULL);

    return 0;
}

CM_CMDB CmCmdbCreateStorage(const char * dir,
        uint32_t ip,
        uint16_t port)
{
    CM_CMDB_CTX * ctx;
    uint32_t retCode;
    
    ctx = CmObjAlloc(CM_CMDB_CTX);
    if (! ctx)
    {
        CmLogError(0, "oom");
        return NULL;
    }

    ctx->ws = dir;

    ctx->deamon.ip = ip;
    ctx->deamon.port = port;

    // 64K buf for message
    ctx->deamon.sndrBuf = (uint8_t *)CmMalloc(64 * 1024);
    if (! ctx->deamon.sndrBuf)
    {
        CmLogError(0, "oom");
        CmFree(ctx);
        return NULL;
    }

    retCode = CmCmdbCreateDeamon(ctx);
    if (0 != retCode)
    {
        CmLogError(retCode, "deamon start fail");
        CmFree(ctx);
        return NULL;
    }

    return ctx;
}

