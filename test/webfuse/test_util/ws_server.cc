#include "webfuse/test_util/ws_server.hpp"
#include "webfuse/test_util/invokation_handler.hpp"
#include "webfuse/impl/util/lws_log.h"
#include "webfuse/impl/json/node.h"
#include "webfuse/test_util/json_doc.hpp"

#include <libwebsockets.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <sstream>
#include <queue>

using webfuse_test::JsonDoc;

namespace
{

class IServer
{
public:
    virtual ~IServer() = default;
    virtual void OnConnected(lws * wsi) = 0;
    virtual void OnConnectionClosed(lws * wsi) = 0;
    virtual void OnMessageReceived(struct lws * wsi, char const * data, size_t length) = 0;
    virtual void OnWritable(struct lws * wsi) = 0;
};

}

extern "C"
{

static int wf_test_utils_ws_server_callback(
    struct lws * wsi,
    enum lws_callback_reasons reason,
    void * user,
    void * in,
    size_t len)
{
    int result = 0;
    struct lws_protocols const * ws_protocol = lws_get_protocol(wsi);
    auto * server = reinterpret_cast<IServer*>(nullptr != ws_protocol ? ws_protocol->user : nullptr);

    if (nullptr != server)
    {
        switch (reason)
        {
            case LWS_CALLBACK_ESTABLISHED:
                server->OnConnected(wsi);
                break;
            case LWS_CALLBACK_CLOSED:
                server->OnConnectionClosed(wsi);
                break;
            case LWS_CALLBACK_RECEIVE:
                {
                    auto * data = reinterpret_cast<char const *>(in);
                    server->OnMessageReceived(wsi, data, len);
                }
                break;
            case LWS_CALLBACK_SERVER_WRITEABLE:
                server->OnWritable(wsi);
                break;
            default:
                break;
        }
    }

    return result;
}

}


namespace webfuse_test
{

class WsServer::Private : public IServer
{
    Private(Private const &) = delete;
    Private & operator=(Private const &) = delete;
public:
    Private(InvokationHandler & handler, std::string const & protocol, int port, bool enable_tls);
    ~Private();
    std::string const & GetUrl() const;
    void OnConnected(lws * wsi) override;
    void OnConnectionClosed(lws * wsi) override;
    void OnMessageReceived(struct lws * wsi, char const * data, size_t length) override;
    void OnWritable(struct lws * wsi) override;

    void SendMessage(char const * message);
private:
    static void Run(Private * self);

    InvokationHandler & handler_;
    std::string protocol_;
    bool is_shutdown_requested;
    lws * wsi_;
    lws_context * ws_context;
    lws_protocols ws_protocols[2];
	lws_context_creation_info info;
    std::string url;
    std::thread context;
    std::mutex mutex;
    std::queue<std::string> writeQueue;
};

WsServer::WsServer(
    InvokationHandler& handler,
    std::string const & protocol,
    int port,
    bool enable_tls)
: d(new WsServer::Private(handler, protocol, port, enable_tls))
{

}

WsServer::~WsServer()
{
    delete d;
}

std::string const & WsServer::GetUrl() const
{
    return d->GetUrl();
}

void WsServer::SendMessage(char const * message)
{
    d->SendMessage(message);
}

WsServer::Private::Private(
    InvokationHandler & handler,
    std::string const & protocol,
    int port,
    bool enable_tls)
: handler_(handler)
, protocol_(protocol)
, is_shutdown_requested(false)
, wsi_(nullptr)
{
    IServer * server = this;
    memset(ws_protocols, 0, sizeof(struct lws_protocols) * 2 );

    ws_protocols[0].name = protocol_.c_str();
    ws_protocols[0].callback = &wf_test_utils_ws_server_callback;
    ws_protocols[0].per_session_data_size = 0;
    ws_protocols[0].user = reinterpret_cast<void*>(server);

    memset(&info, 0, sizeof(struct lws_context_creation_info));
    info.port = port;
    info.mounts = NULL;
    info.protocols =ws_protocols;
    info.vhost_name = "localhost";
    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
    info.options |= LWS_SERVER_OPTION_EXPLICIT_VHOSTS;

    if (enable_tls)
    {
		info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		info.ssl_cert_filepath = "server-cert.pem";
		info.ssl_private_key_filepath = "server-key.pem";
    }

    ws_context = lws_create_context(&info);

    std::ostringstream stream;
    struct lws_vhost * vhost = lws_create_vhost(ws_context, &info);
    stream <<  (enable_tls ? "wss://" : "ws://")
        << "localhost:" << lws_get_vhost_port(vhost) << "/";
    url = stream.str();

    context = std::thread(&Run, this);
}

WsServer::Private::~Private()
{
    std::unique_lock<std::mutex> lock(mutex);
    is_shutdown_requested = true;
    lock.unlock();

    lws_cancel_service(ws_context);
    context.join();
    lws_context_destroy(ws_context);
}

void WsServer::Private::Run(Private * self)
{
    bool is_running = true;
    while (is_running)
    {
        lws_service(self->ws_context, 0);
        {
            std::unique_lock<std::mutex> lock(self->mutex);
            is_running = !self->is_shutdown_requested;
        }
    }
}

void WsServer::Private::OnConnected(lws * wsi)
{
    std::unique_lock<std::mutex> lock(mutex);
    wsi_ = wsi;
}

void WsServer::Private::OnConnectionClosed(lws * wsi)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (wsi == wsi_)
    {
        wsi_ = nullptr;
    }
}

void WsServer::Private::OnWritable(struct lws * wsi)
{
    bool notify = false;

    {
        std::unique_lock<std::mutex> lock(mutex);

        if (!writeQueue.empty())
        {
            std::string const & message = writeQueue.front();

            unsigned char * data = new unsigned char[LWS_PRE + message.size()];
            memcpy(&data[LWS_PRE], message.c_str(), message.size());
            lws_write(wsi, &data[LWS_PRE], message.size(), LWS_WRITE_TEXT);
            delete[] data;

            writeQueue.pop();
            notify = !writeQueue.empty();
        }
    }

    if (notify)
    {
        lws_callback_on_writable(wsi);
    }
}

void WsServer::Private::SendMessage(char const * message)
{
    lws * wsi = nullptr;

    {
        std::unique_lock<std::mutex> lock(mutex);

        if (nullptr != wsi_)
        {
            writeQueue.push(message);
            wsi = wsi_;
        }
    }

    if (nullptr != wsi)
    {
        lws_callback_on_writable(wsi_);
    }
}

void WsServer::Private::OnMessageReceived(struct lws * wsi, char const * data, size_t length)
{
    (void) wsi;

    JsonDoc doc(std::string(data, length));
    wf_json const * request = doc.root();
    wf_json const * method = wf_impl_json_object_get(request, "method");
    wf_json const * params = wf_impl_json_object_get(request, "params");
    wf_json const * id = wf_impl_json_object_get(request, "id");

    if (wf_impl_json_is_string(method) && wf_impl_json_is_array(params) && wf_impl_json_is_int(id))
    {
        std::ostringstream response;

        response << "{";
        try 
        {
            std::string result_text = handler_.Invoke(wf_impl_json_string_get(method), params);
            response << "\"result\": " << result_text;
        }
        catch (...)
        {
            response << "\"error\": {\"code\": 1}";
        }
        response << ", \"id\": " << wf_impl_json_int_get(id) << "}";

        SendMessage(response.str().c_str());
    }
}

std::string const & WsServer::Private::GetUrl() const
{
    return url;
}


}
