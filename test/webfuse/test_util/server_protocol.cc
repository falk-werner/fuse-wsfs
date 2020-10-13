#include "webfuse/test_util/server_protocol.hpp"
#include "webfuse/test_util/mountpoint_factory.hpp"
#include "webfuse/test_util/tempdir.hpp"
#include "webfuse/server_protocol.h"
#include "webfuse/credentials.h"

#include <libwebsockets.h>

#include <cstring>
#include <thread>
#include <mutex>

namespace webfuse_test
{

extern "C" bool webfuse_test_server_protocol_authenticate(
    wf_credentials const * credentials,
    void *)
{
    char const * username = wf_credentials_get(credentials, "username");
    char const * password = wf_credentials_get(credentials, "password");

    return ((nullptr != username) && (nullptr != password) && 
        (0 == strcmp("bob", username)) &&
        (0 == strcmp("secret", password)));
}


class ServerProtocol::Private
{
public:
    Private();
    ~Private();
    char const * GetBaseDir();
    int GetPort() const;
private:
    static void Run(Private * self);
    bool is_shutdown_requested;
    TempDir tempdir;
    int port;
    wf_server_protocol * protocol;
    lws_context * context;
    lws_context_creation_info info;
    lws_protocols protocols[2];
    std::thread thread;
    std::mutex mutex;
};

ServerProtocol::ServerProtocol()
: d(new Private())
{

}

ServerProtocol::~ServerProtocol()
{
    delete d;
}

char const * ServerProtocol::GetBaseDir() const
{
    return d->GetBaseDir();
}

int ServerProtocol::GetPort() const
{
    return d->GetPort();
}

ServerProtocol::Private::Private()
: is_shutdown_requested(false)
, tempdir("webfuse_test_server_protocol")
{
    protocol = wf_server_protocol_create(
        &webfuse_test_create_mountpoint, 
        reinterpret_cast<void*>(const_cast<char*>(tempdir.path())));
    
    wf_server_protocol_add_authenticator(protocol, "username",
        &webfuse_test_server_protocol_authenticate, nullptr);

    memset(protocols, 0, 2 * sizeof(lws_protocols));
    wf_server_protocol_init_lws(protocol, &protocols[0]);

    memset(&info, 0, sizeof(lws_context_creation_info));
    info.port = 0;
    info.protocols = protocols;
    info.vhost_name = "localhost";
    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
    info.options |= LWS_SERVER_OPTION_EXPLICIT_VHOSTS;

	context = lws_create_context(&info);

    struct lws_vhost * const vhost = lws_create_vhost(context, &info);
	port = lws_get_vhost_port(vhost);

    thread = std::thread(&Run, this);
}

ServerProtocol::Private::~Private()
{
    std::unique_lock<std::mutex> lock(mutex);
    is_shutdown_requested = true;
    lock.unlock();

    lws_cancel_service(context);
    thread.join();
    lws_context_destroy(context);
    wf_server_protocol_dispose(protocol);
}

char const * ServerProtocol::Private::GetBaseDir()
{
    return tempdir.path();
}

int ServerProtocol::Private::GetPort() const
{
    return port;
}

void ServerProtocol::Private::Run(Private * self)
{
    std::unique_lock<std::mutex> lock(self->mutex);
    while(!self->is_shutdown_requested)
    {
        lock.unlock();
        lws_service(self->context, 0);
        lock.lock();
    }
}

}