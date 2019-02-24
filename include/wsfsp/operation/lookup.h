#ifndef WSFSP_OPERATION_LOOKUP_H
#define WSFSP_OPERATION_LOOKUP_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "wsfsp/api.h"

struct wsfsp_request;

typedef void wsfsp_lookup_fn(
    struct wsfsp_request * request,
    ino_t parent,
    char const * name,
    void * user_data);

#ifdef __cplusplus
extern "C"
{
#endif

extern WSFSP_API void wsfsp_respond_lookup(
    struct wsfsp_request * request,
    struct stat const * stat);


#ifdef __cplusplus
}
#endif

#endif