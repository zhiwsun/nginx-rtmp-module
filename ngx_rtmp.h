
/*
 * Copyright (C) Roman Arutyunyan
 */


#ifndef _NGX_RTMP_H_INCLUDED_
#define _NGX_RTMP_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <nginx.h>

#include "ngx_rtmp_amf.h"
#include "ngx_rtmp_bandwidth.h"


typedef struct {
    void                  **main_conf;
    void                  **srv_conf;
    void                  **app_conf;
} ngx_rtmp_conf_ctx_t;


typedef struct {
    u_char                  sockaddr[NGX_SOCKADDRLEN];
    socklen_t               socklen;

    /* server ctx */
    ngx_rtmp_conf_ctx_t    *ctx;

    unsigned                bind:1;
    unsigned                wildcard:1;
#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned                ipv6only:2;
#endif
    unsigned                so_keepalive:2;
    unsigned                proxy_protocol:1;
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                     tcp_keepidle;
    int                     tcp_keepintvl;
    int                     tcp_keepcnt;
#endif
} ngx_rtmp_listen_t;


typedef struct {
    ngx_rtmp_conf_ctx_t    *ctx;
    ngx_str_t               addr_text;
    unsigned                proxy_protocol:1;
} ngx_rtmp_addr_conf_t;


typedef struct {
    in_addr_t               addr;
    ngx_rtmp_addr_conf_t    conf;
} ngx_rtmp_in_addr_t;


typedef struct {
    struct in6_addr         addr6;
    ngx_rtmp_addr_conf_t    conf;
} ngx_rtmp_in6_addr_t;



typedef struct {
    void                   *addrs;
    ngx_uint_t              naddrs;
} ngx_rtmp_port_t;


typedef struct {
    int                     family;
    in_port_t               port;
    ngx_array_t             addrs;       /* array of ngx_rtmp_conf_addr_t */
} ngx_rtmp_conf_port_t;


typedef struct {
    struct sockaddr        *sockaddr;
    socklen_t               socklen;

    ngx_rtmp_conf_ctx_t    *ctx;

    unsigned                bind:1;
    unsigned                wildcard:1;
#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned                ipv6only:2;
#endif
    unsigned                so_keepalive:2;
    unsigned                proxy_protocol:1;
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                     tcp_keepidle;
    int                     tcp_keepintvl;
    int                     tcp_keepcnt;
#endif
} ngx_rtmp_conf_addr_t;


#define NGX_RTMP_VERSION                3

#define NGX_LOG_DEBUG_RTMP              NGX_LOG_DEBUG_CORE

#define NGX_RTMP_DEFAULT_CHUNK_SIZE     128


/* RTMP message types */
// 0x01 通知对端，更新最大可接受ChunkSize的大小，默认128 bytes
#define NGX_RTMP_MSG_CHUNK_SIZE         1
// 0x02 通知对端，终止处理指定<csid>信道的后续消息
#define NGX_RTMP_MSG_ABORT              2
// 0x03 由 receiver 发送，当首次收到数据大小等于 WindowsAckSize 消息设置的窗口大小时发送给 sender，连接稳定
#define NGX_RTMP_MSG_ACK                3
// 0x04 用户控制消息
#define NGX_RTMP_MSG_USER               4
// 0x05 由 sender 发送，用来设定 receiver 首次有效数据传输到来之后，用于等待确定传输稳定的窗口大小，单位 bytes
#define NGX_RTMP_MSG_ACK_SIZE           5
// 0x06 有 receiver 发送，根据已经收到但未确认的消息数量来通知 sender 控制发送带宽，收到消息须要发送 WindowAckSize 应答
#define NGX_RTMP_MSG_BANDWIDTH          6
// 0x07
#define NGX_RTMP_MSG_EDGE               7
// 0x08 RTMP音频数据包
#define NGX_RTMP_MSG_AUDIO              8
// 0x09 RTMP视频数据包
#define NGX_RTMP_MSG_VIDEO              9
// 0x0F AMF3编码消息，音视频 MetaData 消息，配置详情
#define NGX_RTMP_MSG_AMF3_META          15
// 0x10 AMF3编码消息，共享对象消息（携带用户详情）
#define NGX_RTMP_MSG_AMF3_SHARED        16
// 0x11 AMF3编码消息，RTMP命令消息，可能涉及用户数据
#define NGX_RTMP_MSG_AMF3_CMD           17
// 0x12 AFM0编码消息，音视频 MetaData 消息，配置详情
#define NGX_RTMP_MSG_AMF_META           18
// 0x13 AMF0编码消息，共享对象消息（携带用户详情）
#define NGX_RTMP_MSG_AMF_SHARED         19
// 0x14 AMF0编码消息，RTMP命令消息，可能涉及用户数据
#define NGX_RTMP_MSG_AMF_CMD            20
// 0x16 整合消息
#define NGX_RTMP_MSG_AGGREGATE          22
//
#define NGX_RTMP_MSG_MAX                22

// 消息块最大大小，单位bytes
#define NGX_RTMP_MAX_CHUNK_SIZE         10485760

#define NGX_RTMP_CONNECT                NGX_RTMP_MSG_MAX + 1
#define NGX_RTMP_DISCONNECT             NGX_RTMP_MSG_MAX + 2
#define NGX_RTMP_HANDSHAKE_DONE         NGX_RTMP_MSG_MAX + 3
#define NGX_RTMP_MAX_EVENT              NGX_RTMP_MSG_MAX + 4


/* RMTP control message types */
#define NGX_RTMP_USER_STREAM_BEGIN      0
#define NGX_RTMP_USER_STREAM_EOF        1
#define NGX_RTMP_USER_STREAM_DRY        2
#define NGX_RTMP_USER_SET_BUFLEN        3
#define NGX_RTMP_USER_RECORDED          4
#define NGX_RTMP_USER_PING_REQUEST      6
#define NGX_RTMP_USER_PING_RESPONSE     7
#define NGX_RTMP_USER_UNKNOWN           8
#define NGX_RTMP_USER_BUFFER_END        31


/* Chunk header:
 *   max 3  basic header
 * + max 11 message header
 * + max 4  extended header (timestamp) */
#define NGX_RTMP_MAX_CHUNK_HEADER       18


// RTMP协议头部
// +--------------+----------------+--------------------+--------------+
// | Basic Header | Message Header | Extended Timestamp |  Chunk Data  |
// +--------------+----------------+--------------------+--------------+
// |<------------------- Chunk Header ----------------->|
typedef struct {
    // basic_header: fmt + csid
    uint32_t                csid;       /* chunk stream id */
    // message_header: timestamp, mlen, message_type, msid
    uint32_t                timestamp;  /* timestamp (delta) */
    uint32_t                mlen;       /* message length */
    uint8_t                 type;       /* message type id */
    uint32_t                msid;       /* message stream id */
} ngx_rtmp_header_t;


typedef struct {
    ngx_rtmp_header_t       hdr;
    uint32_t                dtime;      /* delta time */
    uint32_t                len;        /* current fragment length */
    uint8_t                 ext;
    ngx_chain_t            *in;
} ngx_rtmp_stream_t;


typedef struct {
    uint32_t                signature;  /* "RTMP" */ /* <-- FIXME wtf */

    ngx_event_t             close;

    void                  **ctx;
    void                  **main_conf;
    void                  **srv_conf;
    void                  **app_conf;

    ngx_str_t              *addr_text;
    int                     connected;

    ngx_queue_t             posted_dry_events;

    /* client buffer time in msec */
    uint32_t                buflen;
    uint32_t                ack_size;

    /* connection parameters */
    ngx_str_t               app;
    ngx_str_t               args;
    ngx_str_t               flashver;
    ngx_str_t               swf_url;
    ngx_str_t               tc_url;
    uint32_t                acodecs;
    uint32_t                vcodecs;
    ngx_str_t               page_url;

    /* handshake data */
    ngx_buf_t              *hs_buf;
    u_char                 *hs_digest;
    unsigned                hs_old:1;
    ngx_uint_t              hs_stage;

    /* connection timestamps */
    ngx_msec_t              epoch;
    ngx_msec_t              peer_epoch;
    ngx_msec_t              base_time;
    uint32_t                current_time;

    /* ping */
    ngx_event_t             ping_evt;
    unsigned                ping_active:1;
    unsigned                ping_reset:1;

    /* auto-pushed? */
    unsigned                auto_pushed:1;
    unsigned                relay:1;
    unsigned                static_relay:1;

    /* input stream 0 (reserved by RTMP spec) is used as free chain link */

    ngx_rtmp_stream_t      *in_streams;
    uint32_t                in_csid;
    ngx_uint_t              in_chunk_size;
    ngx_pool_t             *in_pool;
    uint32_t                in_bytes;
    uint32_t                in_last_ack;

    ngx_pool_t             *in_old_pool;
    ngx_int_t               in_chunk_size_changing;

    ngx_connection_t       *connection;

    /* circular buffer of RTMP message pointers */
    ngx_msec_t              timeout;
    uint32_t                out_bytes;
    size_t                  out_pos, out_last;
    ngx_chain_t            *out_chain;
    u_char                 *out_bpos;
    unsigned                out_buffer:1;
    size_t                  out_queue;
    size_t                  out_cork;
    ngx_chain_t            *out[0];
} ngx_rtmp_session_t;


/*
 * RTMP事件处理函数
 *
 * handler result code:
 *  NGX_ERROR - error
 *  NGX_OK    - success, may continue
 *  NGX_DONE  - success, input parsed, reply sent; need no more calls on this event
 */
typedef ngx_int_t (*ngx_rtmp_handler_pt)(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_chain_t *in);


// rtmp_amf 消息中指令处理 handler 映射
typedef struct {
    ngx_str_t               name;
    // 函数指针，指向具体的指令处理函数，方法定义要足够通用
    ngx_rtmp_handler_pt     handler;
} ngx_rtmp_amf_handler_t;


typedef struct {
    ngx_array_t             servers;    /* ngx_rtmp_core_srv_conf_t */
    ngx_array_t             listen;     /* ngx_rtmp_listen_t */

    // events[26] 下标是事件 evt，对应的动态数组 ngx_array_t[evt] 是处理函数链表
    // 事件 evt 对应 ngx_rtmp_header_t->type，是RTMP协议中头部 MessageHeader的message_type字段
    ngx_array_t             events[NGX_RTMP_MAX_EVENT];

    ngx_hash_t              amf_hash;
    ngx_array_t             amf_arrays;

    // ngx_rtmp_amf_handler_t - AMF消息指令的回调函数列表
    ngx_array_t             amf;
} ngx_rtmp_core_main_conf_t;


/* global main conf for stats */
extern ngx_rtmp_core_main_conf_t   *ngx_rtmp_core_main_conf;


typedef struct ngx_rtmp_core_srv_conf_s {
    ngx_array_t             applications; /* ngx_rtmp_core_app_conf_t */

    ngx_msec_t              timeout;
    ngx_msec_t              ping;
    ngx_msec_t              ping_timeout;
    ngx_flag_t              so_keepalive;
    ngx_int_t               max_streams;

    ngx_uint_t              ack_window;

    ngx_int_t               chunk_size;
    ngx_pool_t             *pool;
    ngx_chain_t            *free;
    ngx_chain_t            *free_hs;
    size_t                  max_message;
    ngx_flag_t              play_time_fix;
    ngx_flag_t              publish_time_fix;
    ngx_flag_t              busy;
    size_t                  out_queue;
    size_t                  out_cork;
    ngx_msec_t              buflen;

    ngx_rtmp_conf_ctx_t    *ctx;
} ngx_rtmp_core_srv_conf_t;


typedef struct {
    ngx_array_t             applications; /* ngx_rtmp_core_app_conf_t */
    ngx_str_t               name;
    void                  **app_conf;
} ngx_rtmp_core_app_conf_t;


typedef struct {
    ngx_str_t              *client;
    ngx_rtmp_session_t     *session;
} ngx_rtmp_error_log_ctx_t;


// RTMP模块定义的一些通用的回调函数埋点
typedef struct {
    // ngx_rtmp.c # ngx_rtmp_block
    ngx_int_t             (*preconfiguration)(ngx_conf_t *cf);
    ngx_int_t             (*postconfiguration)(ngx_conf_t *cf);

    void                 *(*create_main_conf)(ngx_conf_t *cf);
    char                 *(*init_main_conf)(ngx_conf_t *cf, void *conf);

    void                 *(*create_srv_conf)(ngx_conf_t *cf);
    char                 *(*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);

    void                 *(*create_app_conf)(ngx_conf_t *cf);
    char                 *(*merge_app_conf)(ngx_conf_t *cf, void *prev, void *conf);
} ngx_rtmp_module_t;


// RTMP模块类型，ngx_module_t.type的枚举
#define NGX_RTMP_MODULE                 0x504D5452     /* "RTMP" */

// rtmp{} 根节点下的配置
#define NGX_RTMP_MAIN_CONF              0x02000000
// rtmp.server{}
#define NGX_RTMP_SRV_CONF               0x04000000
// rtmp.application{}
#define NGX_RTMP_APP_CONF               0x08000000
#define NGX_RTMP_REC_CONF               0x10000000


#define NGX_RTMP_MAIN_CONF_OFFSET  offsetof(ngx_rtmp_conf_ctx_t, main_conf)
#define NGX_RTMP_SRV_CONF_OFFSET   offsetof(ngx_rtmp_conf_ctx_t, srv_conf)
#define NGX_RTMP_APP_CONF_OFFSET   offsetof(ngx_rtmp_conf_ctx_t, app_conf)


#define ngx_rtmp_get_module_ctx(s, module)     (s)->ctx[module.ctx_index]
#define ngx_rtmp_set_ctx(s, c, module)         s->ctx[module.ctx_index] = c;
#define ngx_rtmp_delete_ctx(s, module)         s->ctx[module.ctx_index] = NULL;


#define ngx_rtmp_get_module_main_conf(s, module)                             \
    (s)->main_conf[module.ctx_index]
#define ngx_rtmp_get_module_srv_conf(s, module)  (s)->srv_conf[module.ctx_index]
#define ngx_rtmp_get_module_app_conf(s, module)  ((s)->app_conf ? \
    (s)->app_conf[module.ctx_index] : NULL)

#define ngx_rtmp_conf_get_module_main_conf(cf, module)                       \
    ((ngx_rtmp_conf_ctx_t *) cf->ctx)->main_conf[module.ctx_index]
#define ngx_rtmp_conf_get_module_srv_conf(cf, module)                        \
    ((ngx_rtmp_conf_ctx_t *) cf->ctx)->srv_conf[module.ctx_index]
#define ngx_rtmp_conf_get_module_app_conf(cf, module)                        \
    ((ngx_rtmp_conf_ctx_t *) cf->ctx)->app_conf[module.ctx_index]


#ifdef NGX_DEBUG
char* ngx_rtmp_message_type(uint8_t type);
char* ngx_rtmp_user_message_type(uint16_t evt);
#endif

// 初始化连接，RTMP握手
void ngx_rtmp_init_connection(ngx_connection_t *c);
ngx_rtmp_session_t * ngx_rtmp_init_session(ngx_connection_t *c, ngx_rtmp_addr_conf_t *addr_conf);
void ngx_rtmp_finalize_session(ngx_rtmp_session_t *s);
void ngx_rtmp_handshake(ngx_rtmp_session_t *s);
void ngx_rtmp_client_handshake(ngx_rtmp_session_t *s, unsigned async);
void ngx_rtmp_free_handshake_buffers(ngx_rtmp_session_t *s);
void ngx_rtmp_cycle(ngx_rtmp_session_t *s);
void ngx_rtmp_reset_ping(ngx_rtmp_session_t *s);
ngx_int_t ngx_rtmp_fire_event(ngx_rtmp_session_t *s, ngx_uint_t evt, ngx_rtmp_header_t *h, ngx_chain_t *in);

ngx_int_t ngx_rtmp_set_chunk_size(ngx_rtmp_session_t *s, ngx_uint_t size);


/* Bit reverse: we need big-endians in many places  */
void * ngx_rtmp_rmemcpy(void *dst, const void* src, size_t n);


#define ngx_rtmp_rcpymem(dst, src, n) \
    (((u_char*)ngx_rtmp_rmemcpy(dst, src, n)) + (n))


static ngx_inline uint16_t
ngx_rtmp_r16(uint16_t n)
{
    return (n << 8) | (n >> 8);
}


static ngx_inline uint32_t
ngx_rtmp_r32(uint32_t n)
{
    return (n << 24) | ((n << 8) & 0xff0000) | ((n >> 8) & 0xff00) | (n >> 24);
}


static ngx_inline uint64_t
ngx_rtmp_r64(uint64_t n)
{
    return (uint64_t) ngx_rtmp_r32((uint32_t) n) << 32 | ngx_rtmp_r32((uint32_t) (n >> 32));
}


// 接收消息
ngx_int_t ngx_rtmp_receive_message(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_chain_t *in);
ngx_int_t ngx_rtmp_protocol_message_handler(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_chain_t *in);
ngx_int_t ngx_rtmp_user_message_handler(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_chain_t *in);
ngx_int_t ngx_rtmp_aggregate_message_handler(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_chain_t *in);
ngx_int_t ngx_rtmp_amf_message_handler(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_chain_t *in);
ngx_int_t ngx_rtmp_amf_shared_object_handler(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_chain_t *in);


/* Shared output buffers */

/* Store refcount in negative bytes of shared buffer */

#define NGX_RTMP_REFCOUNT_TYPE              uint32_t
#define NGX_RTMP_REFCOUNT_BYTES             sizeof(NGX_RTMP_REFCOUNT_TYPE)

#define ngx_rtmp_ref(b)                     \
    *((NGX_RTMP_REFCOUNT_TYPE*)(b) - 1)

#define ngx_rtmp_ref_set(b, v)              \
    ngx_rtmp_ref(b) = v

#define ngx_rtmp_ref_get(b)                 \
    ++ngx_rtmp_ref(b)

#define ngx_rtmp_ref_put(b)                 \
    --ngx_rtmp_ref(b)

ngx_chain_t * ngx_rtmp_alloc_shared_buf(ngx_rtmp_core_srv_conf_t *cscf);
void ngx_rtmp_free_shared_chain(ngx_rtmp_core_srv_conf_t *cscf, ngx_chain_t *in);
ngx_chain_t * ngx_rtmp_append_shared_bufs(ngx_rtmp_core_srv_conf_t *cscf, ngx_chain_t *head, ngx_chain_t *in);

#define ngx_rtmp_acquire_shared_chain(in)   \
    ngx_rtmp_ref_get(in);                   \


// 发送消息
void ngx_rtmp_prepare_message(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_rtmp_header_t *lh, ngx_chain_t *out);
ngx_int_t ngx_rtmp_send_message(ngx_rtmp_session_t *s, ngx_chain_t *out, ngx_uint_t priority);


// 注意这里的优先级，priority = 0 优先级最高
#define NGX_RTMP_LIMIT_SOFT         0
#define NGX_RTMP_LIMIT_HARD         1
#define NGX_RTMP_LIMIT_DYNAMIC      2


// 协议控制消息
ngx_chain_t * ngx_rtmp_create_chunk_size(ngx_rtmp_session_t *s, uint32_t chunk_size);
ngx_chain_t * ngx_rtmp_create_abort(ngx_rtmp_session_t *s, uint32_t csid);
ngx_chain_t * ngx_rtmp_create_ack(ngx_rtmp_session_t *s, uint32_t seq);
ngx_chain_t * ngx_rtmp_create_ack_size(ngx_rtmp_session_t *s, uint32_t ack_size);
ngx_chain_t * ngx_rtmp_create_bandwidth(ngx_rtmp_session_t *s, uint32_t ack_size, uint8_t limit_type);

ngx_int_t ngx_rtmp_send_chunk_size(ngx_rtmp_session_t *s, uint32_t chunk_size);
ngx_int_t ngx_rtmp_send_abort(ngx_rtmp_session_t *s, uint32_t csid);
ngx_int_t ngx_rtmp_send_ack(ngx_rtmp_session_t *s, uint32_t seq);
ngx_int_t ngx_rtmp_send_ack_size(ngx_rtmp_session_t *s, uint32_t ack_size);
ngx_int_t ngx_rtmp_send_bandwidth(ngx_rtmp_session_t *s, uint32_t ack_size, uint8_t limit_type);


// 用户控制消息
ngx_chain_t * ngx_rtmp_create_stream_begin(ngx_rtmp_session_t *s, uint32_t msid);
ngx_chain_t * ngx_rtmp_create_stream_eof(ngx_rtmp_session_t *s, uint32_t msid);
ngx_chain_t * ngx_rtmp_create_stream_dry(ngx_rtmp_session_t *s, uint32_t msid);
ngx_chain_t * ngx_rtmp_create_set_buflen(ngx_rtmp_session_t *s, uint32_t msid, uint32_t buflen_msec);
ngx_chain_t * ngx_rtmp_create_recorded(ngx_rtmp_session_t *s, uint32_t msid);
ngx_chain_t * ngx_rtmp_create_ping_request(ngx_rtmp_session_t *s, uint32_t timestamp);
ngx_chain_t * ngx_rtmp_create_ping_response(ngx_rtmp_session_t *s, uint32_t timestamp);

ngx_int_t ngx_rtmp_send_stream_begin(ngx_rtmp_session_t *s, uint32_t msid);
ngx_int_t ngx_rtmp_send_stream_eof(ngx_rtmp_session_t *s, uint32_t msid);
ngx_int_t ngx_rtmp_send_stream_dry(ngx_rtmp_session_t *s, uint32_t msid);
ngx_int_t ngx_rtmp_send_set_buflen(ngx_rtmp_session_t *s, uint32_t msid, uint32_t buflen_msec);
ngx_int_t ngx_rtmp_send_recorded(ngx_rtmp_session_t *s, uint32_t msid);
ngx_int_t ngx_rtmp_send_ping_request(ngx_rtmp_session_t *s, uint32_t timestamp);
ngx_int_t ngx_rtmp_send_ping_response(ngx_rtmp_session_t *s, uint32_t timestamp);


// AMF消息发送/接收
ngx_int_t ngx_rtmp_append_amf(ngx_rtmp_session_t *s, ngx_chain_t **first, ngx_chain_t **last, ngx_rtmp_amf_elt_t *elts, size_t nelts);
ngx_int_t ngx_rtmp_receive_amf(ngx_rtmp_session_t *s, ngx_chain_t *in, ngx_rtmp_amf_elt_t *elts, size_t nelts);

ngx_chain_t * ngx_rtmp_create_amf(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_rtmp_amf_elt_t *elts, size_t nelts);
ngx_int_t ngx_rtmp_send_amf(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_rtmp_amf_elt_t *elts, size_t nelts);

/* AMF status sender */
ngx_chain_t * ngx_rtmp_create_status(ngx_rtmp_session_t *s, char *code, char* level, char *desc);
ngx_chain_t * ngx_rtmp_create_play_status(ngx_rtmp_session_t *s, char *code, char* level, ngx_uint_t duration, ngx_uint_t bytes);
ngx_chain_t * ngx_rtmp_create_sample_access(ngx_rtmp_session_t *s);

ngx_int_t ngx_rtmp_send_status(ngx_rtmp_session_t *s, char *code, char* level, char *desc);
ngx_int_t ngx_rtmp_send_play_status(ngx_rtmp_session_t *s, char *code, char* level, ngx_uint_t duration, ngx_uint_t bytes);
ngx_int_t ngx_rtmp_send_sample_access(ngx_rtmp_session_t *s);


/* Frame types */
#define NGX_RTMP_VIDEO_KEY_FRAME            1
#define NGX_RTMP_VIDEO_INTER_FRAME          2
#define NGX_RTMP_VIDEO_DISPOSABLE_FRAME     3


static ngx_inline ngx_int_t
ngx_rtmp_get_video_frame_type(ngx_chain_t *in)
{
    return (in->buf->pos[0] & 0xf0) >> 4;
}


static ngx_inline ngx_int_t
ngx_rtmp_is_codec_header(ngx_chain_t *in)
{
    return in->buf->pos + 1 < in->buf->last && in->buf->pos[1] == 0;
}


extern ngx_rtmp_bandwidth_t                 ngx_rtmp_bw_out;
extern ngx_rtmp_bandwidth_t                 ngx_rtmp_bw_in;


extern ngx_uint_t                           ngx_rtmp_naccepted;
extern ngx_queue_t                          ngx_rtmp_init_queue;

extern ngx_uint_t                           ngx_rtmp_max_module;
extern ngx_module_t                         ngx_rtmp_core_module;


#endif /* _NGX_RTMP_H_INCLUDED_ */
