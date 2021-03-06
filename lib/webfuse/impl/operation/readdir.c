#include "webfuse/impl/operation/readdir.h"
#include "webfuse/impl/operation/context.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 

#include "webfuse/impl/jsonrpc/proxy.h"
#include "webfuse/impl/json/node.h"
#include "webfuse/impl/util/util.h"
#include "webfuse/impl/util/json_util.h"


#define WF_DIRBUFFER_INITIAL_SIZE 1024

struct wf_impl_dirbuffer
{
	char * data;
	size_t position;
	size_t capacity;
};

static void wf_impl_dirbuffer_init(
	struct wf_impl_dirbuffer * buffer)
{
	buffer->data = malloc(WF_DIRBUFFER_INITIAL_SIZE);
	buffer->position = 0;
	buffer->capacity = WF_DIRBUFFER_INITIAL_SIZE;
}

static void wf_impl_dirbuffer_dispose(
	struct wf_impl_dirbuffer * buffer)
{
	free(buffer->data);
}

static void wf_impl_dirbuffer_add(
	fuse_req_t request,
	struct wf_impl_dirbuffer * buffer,
	char const * name,
	fuse_ino_t inode)
{
	size_t const size = fuse_add_direntry(request, NULL, 0, name, NULL, 0);
	size_t remaining = buffer->capacity - buffer->position;
	while (remaining < size)
	{
		buffer->capacity *= 2;
		buffer->data = realloc(buffer->data, buffer->capacity);
		remaining = buffer->capacity - buffer->position;
	}

	struct stat stat_buffer;
	memset(&stat_buffer, 0, sizeof(struct stat));
	stat_buffer.st_ino = inode;
	fuse_add_direntry(request, 
		&buffer->data[buffer->position], remaining, name,
		&stat_buffer, buffer->position + size);
	buffer->position += size;
}

static size_t wf_impl_min(size_t a, size_t b)
{
	return (a < b) ? a : b;
}

void wf_impl_operation_readdir_finished(
	void * user_data,
	struct wf_json const * result,
	struct wf_jsonrpc_error const * error)
{
	wf_status status = wf_impl_jsonrpc_get_status(error);
	struct wf_impl_operation_readdir_context * context = user_data;

	struct wf_impl_dirbuffer buffer;
	wf_impl_dirbuffer_init(&buffer);

	if ((NULL != result) && (wf_impl_json_is_array(result)))
	{
		size_t const count = wf_impl_json_array_size(result);
		for(size_t i = 0; i < count; i++)
		{
			struct wf_json const * entry = wf_impl_json_array_get(result, i);
			if (wf_impl_json_is_object(entry))
			{
				struct wf_json const * name_holder = wf_impl_json_object_get(entry, "name");
				struct wf_json const * inode_holder = wf_impl_json_object_get(entry, "inode");

				if ((wf_impl_json_is_string(name_holder)) && (wf_impl_json_is_int(inode_holder)))
				{
					char const * name = wf_impl_json_string_get(name_holder);
					fuse_ino_t entry_inode = (fuse_ino_t) wf_impl_json_int_get(inode_holder);
					wf_impl_dirbuffer_add(context->request, &buffer, name, entry_inode);	
				}
				else
				{
					status = WF_BAD_FORMAT;
					break;
				}
			}
			else
			{
				status = WF_BAD_FORMAT;
				break;
			}
		}
	}
	else if (WF_GOOD == status)
	{
		status = WF_BAD_FORMAT;
	}

	if (WF_GOOD == status)
	{
		if (((size_t) context->offset) < buffer.position)
		{
			fuse_reply_buf(context->request, &buffer.data[context->offset],
				wf_impl_min(buffer.position - context->offset, context->size));
		}
		else
		{
			fuse_reply_buf(context->request, NULL, 0);			
		}
		
	}
	else
	{
		fuse_reply_err(context->request, ENOENT);
	}

	wf_impl_dirbuffer_dispose(&buffer);
	free(context);
}

void wf_impl_operation_readdir (
	fuse_req_t request,
	fuse_ino_t inode,
	size_t size,
	off_t offset,
	struct fuse_file_info * WF_UNUSED_PARAM(file_info))
{
    struct wf_impl_operation_context * user_data = fuse_req_userdata(request);
    struct wf_jsonrpc_proxy * rpc = wf_impl_operation_context_get_proxy(user_data);

	if (NULL != rpc)
	{
		struct wf_impl_operation_readdir_context * readdir_context = malloc(sizeof(struct wf_impl_operation_readdir_context));
		readdir_context->request = request;
		readdir_context->size = size;
		readdir_context->offset = offset;

		wf_impl_jsonrpc_proxy_invoke(rpc, &wf_impl_operation_readdir_finished, readdir_context, "readdir", "si", user_data->name, inode);
	}
	else
	{
		fuse_reply_err(request, ENOENT);
	}	
}
