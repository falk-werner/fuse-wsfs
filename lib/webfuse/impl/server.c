#include "webfuse/server.h"

#include <stdlib.h>
#include <stdbool.h>
#include <libwebsockets.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "webfuse/impl/server_config.h"
#include "webfuse/impl/server_protocol.h"
#include "webfuse/impl/util/lws_log.h"

#define WF_SERVER_PROTOCOL_COUNT 3

struct wf_server
{
    struct wf_server_config config;
    struct wf_server_protocol protocol;
    struct lws_protocols ws_protocols[WF_SERVER_PROTOCOL_COUNT];
    struct lws_context * context;
	struct lws_http_mount mount;
	struct lws_context_creation_info info;
	int port;
};

static bool wf_impl_server_tls_enabled(
	struct wf_server * server)
{
	return ((server->config.key_path != NULL) && (server->config.cert_path != NULL));
}

static struct lws_context * wf_impl_server_context_create(
    struct wf_server * server)
{
	wf_impl_lwslog_disable();

    memset(server->ws_protocols, 0, sizeof(struct lws_protocols) * WF_SERVER_PROTOCOL_COUNT);
    server->ws_protocols[0].name = "http";
    server->ws_protocols[0].callback = lws_callback_http_dummy;
    wf_impl_server_protocol_init_lws(&server->protocol, &server->ws_protocols[1]);

	memset(&server->mount, 0, sizeof(struct lws_http_mount));
	server->mount.mount_next = NULL,
	server->mount.mountpoint = "/",
	server->mount.origin = server->config.document_root,
	server->mount.def = "index.html",
	server->mount.origin_protocol = LWSMPRO_FILE,
	server->mount.mountpoint_len = 1,

	memset(&server->info, 0, sizeof(struct lws_context_creation_info));
	server->info.port = server->config.port;
	server->info.mounts = &server->mount;
	server->info.protocols = server->ws_protocols;
	server->info.vhost_name = server->config.vhost_name;
	server->info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
	server->info.options |= LWS_SERVER_OPTION_EXPLICIT_VHOSTS;

	if (NULL == server->config.document_root)
	{
		// disable http
		server->info.protocols = &server->ws_protocols[1];
		server->info.mounts = NULL;
	}

	if (wf_impl_server_tls_enabled(server))
	{
		server->info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		server->info.ssl_cert_filepath = server->config.cert_path;
		server->info.ssl_private_key_filepath = server->config.key_path;
	}

	struct lws_context * const context = lws_create_context(&server->info);

    struct lws_vhost * const vhost = lws_create_vhost(context, &server->info);
	server->port = lws_get_vhost_port(vhost);

    return context;
}

struct wf_server * wf_impl_server_create(
    struct wf_server_config * config)
{
	struct wf_server * server = NULL;
	
	if (wf_impl_mountpoint_factory_isvalid(&config->mountpoint_factory))
	{
		server = malloc(sizeof(struct wf_server));
		wf_impl_server_protocol_init(&server->protocol, &config->mountpoint_factory);
		wf_impl_server_config_clone(config, &server->config);
		wf_impl_authenticators_move(&server->config.authenticators, &server->protocol.authenticators);				
		server->context = wf_impl_server_context_create(server);
	}

    return server; 
}

void wf_impl_server_dispose(
    struct wf_server * server)
{
    lws_context_destroy(server->context);
    wf_impl_server_protocol_cleanup(&server->protocol);
    wf_impl_server_config_cleanup(&server->config);
    free(server);   
}

bool wf_impl_server_is_operational(
    struct wf_server * server)
{
	return server->protocol.is_operational;
}

void wf_impl_server_service(
    struct wf_server * server)
{
	lws_service(server->context, 0);
}

void wf_impl_server_interrupt(
    struct wf_server * server)
{
	lws_cancel_service(server->context);
}

extern int wf_impl_server_get_port(
    struct wf_server const * server)
{
	return server->port;
}
