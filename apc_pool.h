/*
  +----------------------------------------------------------------------+
  | APC                                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Gopal Vijayaraghavan <gopalv@yahoo-inc.com>                 |
  +----------------------------------------------------------------------+

   This software was contributed to PHP by Yahoo! Inc. in 2008.

   Future revisions and derivatives of this source code must acknowledge
   Yahoo! Inc. as the original contributor of this module by
   leaving this note intact in the source code.

   All other licensing and usage conditions are those of the PHP Group.

 */

/* $Id: apc_pool.h 307048 2011-01-03 23:53:17Z kalle $ */

#ifndef APC_POOL_H
#define APC_POOL_H

#include "apc.h"
#include "apc_sma.h"

/* #define APC_POOL_DEBUG 1 */

typedef enum {
    APC_UNPOOL         = 0x0,
    APC_SMALL_POOL     = 0x1,
    APC_MEDIUM_POOL    = 0x2,
    APC_LARGE_POOL     = 0x3,
    APC_POOL_SIZE_MASK = 0x7,   /* waste a bit */
#if APC_POOL_DEBUG
    APC_POOL_REDZONES  = 0x08,
    APC_POOL_SIZEINFO  = 0x10,
    APC_POOL_OPT_MASK  = 0x18
#endif
} apc_pool_type;

#if APC_POOL_DEBUG
#define APC_POOL_HAS_SIZEINFO(pool) ((pool->type & APC_POOL_SIZEINFO)!=0)
#define APC_POOL_HAS_REDZONES(pool) ((pool->type & APC_POOL_REDZONES)!=0)
#else
/* let gcc optimize away the optional features */
#define APC_POOL_HAS_SIZEINFO(pool) (0)
#define APC_POOL_HAS_REDZONES(pool) (0)
#endif

typedef struct _apc_pool apc_pool;

/* 
 pcleanup is called during apc_pool_destroy before free
*/
typedef void  (*apc_pcleanup_t)(apc_pool *pool TSRMLS_DC);
/*
 palloc is called to allocate memory from the pool
*/
typedef void* (*apc_palloc_t)(apc_pool *pool, size_t size TSRMLS_DC);
/*
 pfree is called to free memory from the pool, and during apc_pool_destroy
*/
typedef void  (*apc_pfree_t) (apc_pool *pool, void* p TSRMLS_DC);
/*
 protect is called to protect a block allocated from the pool
*/
typedef void* (*apc_protect_t)  (void *p);
/*
 unprotect is called to unprotect a block allocated from the pool
*/
typedef void* (*apc_unprotect_t)(void *p);

/* {{{ structure definition: apc_pool */ 
struct _apc_pool {
	/* denotes the size and debug flags for a pool */
    apc_pool_type   type;
	
	/* handler functions */
    apc_malloc_t    allocate;
    apc_free_t      deallocate;

    apc_palloc_t    palloc;
    apc_pfree_t     pfree;

	apc_protect_t   protect;
	apc_unprotect_t unprotect;

    apc_pcleanup_t  cleanup;

	/* total */
    size_t          size;
	/* remaining */
    size_t          used;

    /* apc_realpool and apc_unpool add more here */
}; /* }}} */

/* {{{ enum definition: apc_copy_type */
/* APC_COPY_IN should be used when copying into APC 
   APC_COPY_OUT should be used when copying out of APC */
typedef enum _apc_copy_type {
    APC_NO_COPY = 0,
    APC_COPY_IN,
    APC_COPY_OUT,
	APC_COPY_OTHER
} apc_copy_type; /* }}} */

/* {{{ enum definition: apc_context_type 
	APC_CONTEXT_SHARE should be used to create contexts using shared memory 
	APC_CONTEXT_NOSHARE should be used to create contexts using standard allocators */
typedef enum _apc_context_type {
	APC_CONTEXT_NONE = 0,
    APC_CONTEXT_SHARE,
	APC_CONTEXT_NOSHARE
} apc_context_type; /* }}} */

/* {{{ struct definition: apc_context_t */
typedef struct _apc_context_t {
    apc_pool *pool;
    apc_copy_type copy;
    unsigned int force_update:1;
} apc_context_t; /* }}} */

/*
 performs alignment checking on pool internals
*/
extern void apc_pool_init();

/*
 apc_pool_create creates a pool of the specified type, setting the handlers passed on the pool, returns apc_pool*
*/
extern apc_pool* apc_pool_create(apc_pool_type pool_type,
                                 apc_malloc_t allocate,
                                 apc_free_t deallocate,
                                 apc_protect_t protect,
                                 apc_unprotect_t unprotect TSRMLS_DC);

/* apc_pool_alloc performs alloc, using the handler provided to apc_pool_create */
#define apc_pool_alloc(pool, size) ((void *) pool->palloc(pool, size TSRMLS_CC))

/* apc_pool_alloc performs free, using the handler provided to apc_pool_create */
#define apc_pool_free(pool, ptr) ((void)   pool->pfree (pool, ptr TSRMLS_CC))

/* apc_pool_alloc performs mprotect, using the handler provided to apc_pool_create, where applicable */
#define apc_pool_protect(pool, ptr) (pool->protect ? (pool)->protect((ptr)) : (ptr))

/* apc_pool_alloc performs munprotect, using the handler provided to apc_pool_create, where applicable */
#define apc_pool_unprotect(pool, ptr) (pool->unprotect ? (pool)->unprotect((ptr)) : (ptr))

/*
 apc_pool_destroy first calls apc_cleanup_t set during apc_pool_create, then apc_free_t
*/
extern void apc_pool_destroy(apc_pool* pool TSRMLS_DC);

/*
 apc_pmemcpy performs memcpy using resources provided by pool
*/
extern void* apc_pmemcpy(const void* p, 
                         size_t n, 
                         apc_pool* pool TSRMLS_DC);

/*
 apc_pstrdup performs strdup using resources provided by pool
*/
extern void* apc_pstrdup(const char* s, 
                         apc_pool* pool TSRMLS_DC);

#endif

