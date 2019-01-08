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
#include "string.h"

#include "unistd.h"
#include "sys/socket.h"
#include "netinet/in.h"

#include "pthread.h"

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
#define CmAllocObj(cls) (cls *)CmMalloc(sizeof(cls))

#define CmLogInfo(ret, ...)     printf(__VA_ARGS__); printf("\n");
#define CmLogError(ret, ...)    printf(__VA_ARGS__); printf("\n");
/* end of cm_base.h */

typedef struct
{
    uint32_t ip;        /// ipv4 only, in big ending
    uint16_t port;      /// port
    uint8_t sndrBuf[64 * 1024];
    uint8_t rcvrBuf[64 * 1024];
} CM_CMDB_DEAMON;

typedef struct
{
#define DEFAULT_SPACE (64)
    uint32_t space;
    struct dl * bkt;
} CM_CMDB_IDX_HASH;

typedef struct
{
    CM_CMDB_DEAMON deamon;
} CM_CMDB_CTX;

char __cmdBuf[1024];

uint32_t CmCmdbCreateDeamon(CM_CMDB_CTX * ctx)
{
    pid_t pid;

    (void)ctx;

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

    // sprintf(__cmdBuf, "%s/cmdeamon.py --workspace %s --addr %u:%u",
    //         ctx->ws, ctx->ws, ctx->deamon.ip, ctx->deamon.port);

    // execlp("python", __cmdBuf, NULL);

    return 0;
}

/* @deprecated
 * CM_CMDB CmCmdbCreateStorage(const char * dir,
 *         uint32_t ip,
 *         uint16_t port)
 * {
 * }
 */

uint32_t CmUdpListen(CM_CMDB_CTX * ctx,
        void (* msgRecv)(CM_CMDB_CTX * ctx, uint32_t len))
{
    struct sockaddr_in self;
    struct sockaddr_in peer;

    socklen_t len;
    int skt;
    int size;

    // 8800 is reserved for client
    const uint16_t port = 8800;

    skt = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == skt)
    {
        CmLogError(-1, "create socket fail for port %u", port);
        return -1;
    }

    memset(&self, 0, sizeof(self));

    self.sin_family = AF_INET;
    self.sin_port = htons(port);
    self.sin_addr.s_addr = htonl(INADDR_ANY);

    if (-1 == bind(skt, (struct sockaddr *)&self, sizeof(self)))
    {
        CmLogError(-1, "bind port %u fail", port);
        return -1;
    }

    for (;;)
    {
        size = recvfrom(skt, ctx->deamon.rcvrBuf,
                64 * 1024, 0, (struct sockaddr *)&peer, &len);
        if (size < 0)
        {
            CmLogError(-1, "exception on port %u", port);
            return -1;
        }

        if (msgRecv)
        {
            msgRecv(ctx, len);
        }
    }

    return 0;
}

void CmCmdbClientMsgEntry(CM_CMDB_CTX * ctx,
        uint32_t len)
{
    (void)ctx;
    (void)len;

    return;
}

void * CmCmdbRoutine(void * arg)
{
    CM_CMDB_CTX * ctx = (CM_CMDB_CTX *)arg;

    CmLogInfo(0, "init routine success");

    CmUdpListen(ctx, CmCmdbClientMsgEntry);

    return NULL;
}

uint32_t CmCmdbStartClient(CM_CMDB_CTX * ctx)
{
    pthread_t t;
    int ret;

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4 * 1024);

    ret = pthread_create(&t, &attr, CmCmdbRoutine, ctx);
    if (0 != ret)
    {
        CmLogError(1, "create routine fail");
        return 1;
    }

    return 0;
}

typedef enum
{
    CM_CMDB_CONNECT = 0,
    CM_CMDB_WRITE,
    CM_CMDB_QUERY,
    CM_CMDB_REMOVE,
    CM_CMDB_TASK_TYPE_NUM
} CM_CMDB_TASK;

typedef struct
{
    CM_CMDB_TASK type;

    union
    {
        uint32_t connectResult;
    } content;
} CM_CMDB_TASK_RESULT;

uint32_t CmCmdbTaskSync(CM_CMDB_CTX * ctx,
        CM_CMDB_TASK_RESULT * result)
{
    (void)ctx;
    (void)result;

    return 0;
}

CM_CMDB CmCmdbConnectStorage(uint32_t ip,
        uint16_t port)
{
    CM_CMDB_CTX * ctx;
    uint32_t retCode;

    ctx = CmAllocObj(CM_CMDB_CTX);
    if (! ctx)
    {
        CmLogError(0, "oom");
        return NULL;
    }

    memset(ctx, 0, sizeof(CM_CMDB_CTX));

    ctx->deamon.ip = ip;
    ctx->deamon.port = port;

    retCode = CmCmdbStartClient(ctx);
    if (0 != retCode)
    {
        CmLogError(retCode, "start client fail");
        CmFree(ctx);
        return NULL;
    }

    // retCode = CmCmdbTaskSync(ctx, CM_CMDB_CONNECT);
    // if (0 != retCode)
    // {
    //     CmLogError(retCode, "connect fail");
    //     CmFree(ctx);
    //     return NULL;
    // }

    return ctx;
}

#if 1

int main()
{
    CM_CMDB db;

    db = CmCmdbConnectStorage(0x7f000001, 8805);

    for (;;)
    {
        usleep(1000);
    }

    return 0;
}

#endif

