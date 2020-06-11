#ifndef WF_ADAPTER_IMPL_CLIENT_PROTOCOL_H
#define WF_ADAPTER_IMPL_CLIENT_PROTOCOL_H

#include "webfuse/adapter/client_callback.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct lws_protocols;

typedef void
wf_client_protocol_callback_fn(
    void * user_data,
    int reason,
    void * arg);

struct wf_client_protocol
{
    wf_client_callback_fn * callback;
    void * user_data;
};

extern void
wf_impl_client_protocol_init(
    struct wf_client_protocol * protocol,
    wf_client_callback_fn * callback,
    void * user_data);

extern void
wf_impl_client_protocol_cleanup(
    struct wf_client_protocol * protocol);

extern void
wf_impl_client_protocol_callback(
    struct wf_client_protocol * protocol,
    int reason,
    void * arg);

extern void
wf_impl_client_protocol_init_lws(
    struct wf_client_protocol * protocol,
    struct lws_protocols * lws_protocol);

extern void
wf_impl_client_protocol_connect(
    struct wf_client_protocol * protocol,
    char const * url);

extern void
wf_impl_client_protocol_disconnect(
    struct wf_client_protocol * protocol);

#ifdef __cplusplus
}
#endif


#endif
