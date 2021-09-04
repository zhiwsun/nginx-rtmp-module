cmake_minimum_required(VERSION 3.20)
project(nginx_rtmp_module C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(dash)
include_directories(hls)

add_executable(nginx_rtmp_module
        dash/ngx_rtmp_dash_module.c
        dash/ngx_rtmp_mp4.c
        dash/ngx_rtmp_mp4.h
        hls/ngx_rtmp_hls_module.c
        hls/ngx_rtmp_mpegts.c
        hls/ngx_rtmp_mpegts.h
        ngx_rtmp.c
        ngx_rtmp.h
        ngx_rtmp_access_module.c
        ngx_rtmp_amf.c
        ngx_rtmp_amf.h
        ngx_rtmp_auto_push_module.c
        ngx_rtmp_bandwidth.c
        ngx_rtmp_bandwidth.h
        ngx_rtmp_bitop.c
        ngx_rtmp_bitop.h
        ngx_rtmp_cmd_module.c
        ngx_rtmp_cmd_module.h
        ngx_rtmp_codec_module.c
        ngx_rtmp_codec_module.h
        ngx_rtmp_control_module.c
        ngx_rtmp_core_module.c
        ngx_rtmp_eval.c
        ngx_rtmp_eval.h
        ngx_rtmp_exec_module.c
        ngx_rtmp_flv_module.c
        ngx_rtmp_handler.c
        ngx_rtmp_handshake.c
        ngx_rtmp_init.c
        ngx_rtmp_limit_module.c
        ngx_rtmp_live_module.c
        ngx_rtmp_live_module.h
        ngx_rtmp_log_module.c
        ngx_rtmp_mp4_module.c
        ngx_rtmp_netcall_module.c
        ngx_rtmp_netcall_module.h
        ngx_rtmp_notify_module.c
        ngx_rtmp_play_module.c
        ngx_rtmp_play_module.h
        ngx_rtmp_proxy_protocol.c
        ngx_rtmp_proxy_protocol.h
        ngx_rtmp_receive.c
        ngx_rtmp_record_module.c
        ngx_rtmp_record_module.h
        ngx_rtmp_relay_module.c
        ngx_rtmp_relay_module.h
        ngx_rtmp_send.c
        ngx_rtmp_shared.c
        ngx_rtmp_stat_module.c
        ngx_rtmp_streams.h
        ngx_rtmp_version.h)
