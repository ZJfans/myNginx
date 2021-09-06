
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)

#define NGX_POOL_ALIGNMENT       16
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)


typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

/* 自定义清理回调的数据结构   1. 每个work线程都会有一个“ 大 ”内存池  
                            2.当有一个connection时，会在“大”的内存池中创建“ 中 ”内存池  
   大 -  中  -  小           3.当这个连接有一个request时，会在“ 中 ”的内存池创建小的内存池 */
                            
    ngx_pool_cleanup_pt   handler;  /* 清理内存的回调函数 */
    void                 *data;     /* 指向需要清理的数据地址 */
    ngx_pool_cleanup_t   *next;     /* 指向下一个清理结构 */
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;

struct ngx_pool_large_s {
    ngx_pool_large_t     *next;   /* 指向下一块存储大数据内存块的指针 */
    void                 *alloc;  /* 指向本块内存的指针？？？？ */
};


typedef struct {
    u_char               *last;    /* 当前内存池未使用内存初始地址 */
    u_char               *end;     /* 内存池的结束地址 */
    ngx_pool_t           *next;    /* 指向下一个内存池的指针 */
    ngx_uint_t            failed;  /* 失败次数 ---- 分配内存时失败的次数 */
}  ngx_pool_data_t;


struct ngx_pool_s {
    ngx_pool_data_t       d;        /* 内存池数据区域 */ 
    size_t                max;      /* 最大每次可分配的内存值 */
    ngx_pool_t           *current;  /* 指向当前内存池地址 */
    ngx_chain_t          *chain;    /* 缓冲区链表 */
    ngx_pool_large_t     *large;    /* 存储大数据的链表 */
    ngx_pool_cleanup_t   *cleanup;  /* 清除内存块分配的内存 */
    ngx_log_t            *log;      /* 日志 */
};


typedef struct {
    ngx_fd_t              fd;
    u_char               *name;
    ngx_log_t            *log;
} ngx_pool_cleanup_file_t;


ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
void ngx_destroy_pool(ngx_pool_t *pool);
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);


ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);
void ngx_pool_cleanup_file(void *data);
void ngx_pool_delete_file(void *data);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
