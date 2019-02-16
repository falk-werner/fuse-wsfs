#ifndef _WSFSP_CLIENT_PROTOCOL_H
#define _WSFSP_CLIENT_PROTOCOL_H

#include "wsfs/api.h"

struct wsfsp_client_protocol;
struct wsfsp_provider;
struct lws_protocols;

#ifdef __cplusplus
extern "C"
{
#endif

extern WSFSP_API struct wsfsp_client_protocol * wsfsp_client_protocol_create(
    struct wsfsp_provider const * provider,
    void * user_data);

extern WSFSÜ_API void wsfsp_client_protocol_dispose(
    struct wsfsp_client_protocol * protocol);

extern WSFSÜ_API void wsfsp_client_protocol_init_lws(
    struct wsfs_provider_client_protocol * protocol,
    struct lws_protocols * lws_protocol);

#ifdef __cplusplus
}
#endif



#endif
