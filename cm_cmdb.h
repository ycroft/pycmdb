/*
 * =====================================================================================
 *
 *       Filename:  cm_cmdb.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2018年12月02日 22时46分54秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yin Yuhao (), yyh18ball@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef _CM_CMDB_H_
#define _CM_CMDB_H_

#include "stdint.h"

typedef void * CM_CMDB;

// you have 2 way to create cmdb
//
// 1. use CmCmdbCreateStorage to create local database.
//      it will return db handle. you do not need to connect to it.
//
// 2. use CmCmdbConnectStorage to connect to a exist database
//

// @deprecated
// CM_CMDB CmCmdbCreateStorage(const char * dir,
//         uint32_t ip,
//         uint16_t port);

CM_CMDB CmCmdbConnectStorage(uint32_t ip,
        uint16_t port);

// completely remove db files
void CmCmdbDestroyStorage(CM_CMDB db);

typedef void * CM_CMDB_IDX;

typedef void * CM_CMDB_REC;

typedef enum
{
    CM_CMDB_IDX_TYPE_HASH = 0,
    CM_CMDB_IDX_TYPE_AVL,
    CM_CMDB_IDX_TYPE_LLRBT,
    CM_CMDB_IDX_TYPE_NUM
} CM_CMDB_IDX_TYPE;

CM_CMDB_IDX CmCmdbCreateIndex(uint16_t idxNo,
        CM_CMDB_IDX_TYPE type);

void CmCmdbDestroyIndex(uint16_t idxNo);

typedef struct
{
    void * ref;
    uint32_t len;
} CM_CMDB_VALUE;

uint32_t CmCmdbWrite(CM_CMDB db,
        CM_CMDB_IDX idx,
        CM_CMDB_VALUE * key,
        CM_CMDB_VALUE * record);

CM_CMDB_REC CmCmdbQuery(CM_CMDB db,
        CM_CMDB_IDX idx,
        CM_CMDB_VALUE * key);

uint32_t CmCmdbObj2Value(CM_CMDB db,
        CM_CMDB_REC recObj,
        CM_CMDB_VALUE * outRecord);

uint32_t CmCmdbRemove(CM_CMDB db,
        CM_CMDB_REC record);

// non-block interface
// task will be executed silently
uint32_t CmCmdbFlush(CM_CMDB db);

#endif /* _CM_CMDB_H_ */

