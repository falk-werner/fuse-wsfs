#include <gtest/gtest.h>

#include <jansson.h>

#include "webfuse/server.h"
#include "webfuse/server_config.h"
#include "webfuse/test_util/server.hpp"
#include "webfuse/test_util/ws_client.hpp"
#include "webfuse/test_util/file.hpp"
#include "webfuse/mocks/mock_invokation_handler.hpp"
#include "webfuse/protocol_names.h"
#include "webfuse/mocks/open_matcher.hpp"
#include "webfuse/mocks/getattr_matcher.hpp"
#include "webfuse/mocks/lookup_matcher.hpp"
#include "webfuse/mocks/readdir_matcher.hpp"

using webfuse_test::MockInvokationHander;
using webfuse_test::WsClient;
using webfuse_test::Server;
using webfuse_test::File;
using webfuse_test::GetAttr;
using webfuse_test::Open;
using webfuse_test::Lookup;
using webfuse_test::ReadDir;
using testing::StrEq;
using testing::_;
using testing::AnyNumber;
using testing::AtMost;
using testing::Return;
using testing::Invoke;

namespace
{
struct wf_mountpoint *
create_mountpoint(
    char const * filesystem,
    void * user_data)
{
    (void) filesystem;
    (void) user_data;

    return nullptr;
}

}

TEST(server, create_dispose)
{
    mkdir("test", 0700);

    struct wf_server_config * config = wf_server_config_create();
    wf_server_config_set_mountpoint_factory(config, &create_mountpoint, nullptr);
    struct wf_server * server = wf_server_create(config);
    ASSERT_NE(nullptr, server);

    wf_server_dispose(server);
    wf_server_config_dispose(config);

    rmdir("test");
}

TEST(server, connect)
{
    Server server;
    MockInvokationHander handler;
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, add_filesystem)
{
    Server server;
    MockInvokationHander handler;
    EXPECT_CALL(handler, Invoke(StrEq("lookup"), _)).Times(AnyNumber());
    EXPECT_CALL(handler, Invoke(StrEq("getattr"), GetAttr(1))).Times(AnyNumber())
        .WillOnce(Return("{\"mode\": 420, \"type\": \"dir\"}"));
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"add_filesystem\", \"params\": [\"test\"], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * result = json_object_get(response, "result");
    ASSERT_TRUE(json_is_object(result));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    std::string base_dir = server.GetBaseDir();
    ASSERT_TRUE(File(base_dir).isDirectory());
    File file(base_dir + "/test");
    ASSERT_TRUE(file.isDirectory());

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, add_filesystem_fail_missing_param)
{
    Server server;
    MockInvokationHander handler;
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"add_filesystem\", \"params\": [], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * error = json_object_get(response, "error");
    ASSERT_TRUE(json_is_object(error));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, add_filesystem_fail_invalid_name_type)
{
    Server server;
    MockInvokationHander handler;
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"add_filesystem\", \"params\": [42], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * error = json_object_get(response, "error");
    ASSERT_TRUE(json_is_object(error));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, add_filesystem_fail_invalid_name)
{
    Server server;
    MockInvokationHander handler;
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"add_filesystem\", \"params\": [\"invalid_1/name\"], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * error = json_object_get(response, "error");
    ASSERT_TRUE(json_is_object(error));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}


TEST(server, authenticate)
{
    Server server;
    MockInvokationHander handler;
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"authenticate\", \"params\": [\"username\", {\"username\": \"bob\", \"password\": \"secret\"}], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * result = json_object_get(response, "result");
    ASSERT_TRUE(json_is_object(result));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, authenticate_fail_missing_params)
{
    Server server;
    MockInvokationHander handler;
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"authenticate\", \"params\": [], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * error = json_object_get(response, "error");
    ASSERT_TRUE(json_is_object(error));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, authenticate_fail_invalid_type)
{
    Server server;
    MockInvokationHander handler;
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"authenticate\", \"params\": [42, {\"username\": \"bob\", \"password\": \"secret\"}], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * error = json_object_get(response, "error");
    ASSERT_TRUE(json_is_object(error));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, authenticate_fail_invalid_credentials)
{
    Server server;
    MockInvokationHander handler;
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"authenticate\", \"params\": [\"username\", 42], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * error = json_object_get(response, "error");
    ASSERT_TRUE(json_is_object(error));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, authenticate_fail_missing_credentials)
{
    Server server;
    MockInvokationHander handler;
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"authenticate\", \"params\": [\"username\"], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * error = json_object_get(response, "error");
    ASSERT_TRUE(json_is_object(error));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, read)
{
    Server server;
    MockInvokationHander handler;
    EXPECT_CALL(handler, Invoke(StrEq("lookup"), _)).Times(AnyNumber());
    EXPECT_CALL(handler, Invoke(StrEq("lookup"), Lookup(1, "a.file"))).Times(1)
        .WillOnce(Return("{\"inode\": 2, \"mode\": 420, \"type\": \"file\", \"size\": 1}"));
    EXPECT_CALL(handler, Invoke(StrEq("getattr"), GetAttr(1))).Times(AnyNumber())
        .WillOnce(Return("{\"mode\": 420, \"type\": \"dir\"}"));
    EXPECT_CALL(handler, Invoke(StrEq("open"), Open(2))).Times(1)
        .WillOnce(Return("{\"handle\": 42}"));
    EXPECT_CALL(handler, Invoke(StrEq("read"), _)).Times(1)
        .WillOnce(Return("{\"data\": \"*\", \"format\": \"identity\", \"count\": 1}"));
    EXPECT_CALL(handler, Invoke(StrEq("close"), _)).Times(AtMost(1));
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"add_filesystem\", \"params\": [\"test\"], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * result = json_object_get(response, "result");
    ASSERT_TRUE(json_is_object(result));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    std::string base_dir = server.GetBaseDir();
    ASSERT_TRUE(File(base_dir).isDirectory());
    File file(base_dir + "/test/a.file");
    ASSERT_TRUE(file.hasContents("*"));

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, read_large_file_contents)
{
    Server server;
    MockInvokationHander handler;
    EXPECT_CALL(handler, Invoke(StrEq("lookup"), _)).Times(AnyNumber());
    EXPECT_CALL(handler, Invoke(StrEq("lookup"), Lookup(1, "a.file"))).Times(1)
        .WillOnce(Return("{\"inode\": 2, \"mode\": 420, \"type\": \"file\", \"size\": 4096}"));
    EXPECT_CALL(handler, Invoke(StrEq("getattr"), GetAttr(1))).Times(AnyNumber())
        .WillRepeatedly(Return("{\"mode\": 420, \"type\": \"dir\"}"));
    EXPECT_CALL(handler, Invoke(StrEq("getattr"), GetAttr(2))).Times(AnyNumber())
        .WillRepeatedly(Return("{\"mode\": 420, \"type\": \"file\", \"size\": 4096}"));
    EXPECT_CALL(handler, Invoke(StrEq("open"), Open(2))).Times(1)
        .WillOnce(Return("{\"handle\": 42}"));
    EXPECT_CALL(handler, Invoke(StrEq("read"), _)).Times(AnyNumber())
        .WillRepeatedly(Invoke([](char const *, json_t * params) {
            int offset = json_integer_value(json_array_get(params, 3));
            int length = json_integer_value(json_array_get(params, 4));

            int remaining = (offset < 4096) ? 4096 - offset : 0;
            int count = (length < remaining) ? length : remaining;

            std::string data = std::string(count, '*');

            json_t * result = json_object();
            json_object_set_new(result, "data", json_string(data.c_str()));
            json_object_set_new(result, "format", json_string("identity"));
            json_object_set_new(result, "count", json_integer(count));

            char * result_text = json_dumps(result, 0);
            std::string result_str = result_text;
            free(result_text);
            json_decref(result);

            return result_str;
        })); 
    EXPECT_CALL(handler, Invoke(StrEq("close"), _)).Times(AtMost(1));
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"add_filesystem\", \"params\": [\"test\"], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * result = json_object_get(response, "result");
    ASSERT_TRUE(json_is_object(result));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    std::string base_dir = server.GetBaseDir();
    ASSERT_TRUE(File(base_dir).isDirectory());
    File file(base_dir + "/test/a.file");
    std::string contents(4096, '*');
    ASSERT_TRUE(file.hasContents(contents));

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, read_large_file)
{
    Server server;
    MockInvokationHander handler;
    EXPECT_CALL(handler, Invoke(StrEq("lookup"), _)).Times(AnyNumber());
    EXPECT_CALL(handler, Invoke(StrEq("lookup"), Lookup(1, "a.file"))).Times(1)
        .WillOnce(Return("{\"inode\": 2, \"mode\": 420, \"type\": \"file\", \"size\": 1024000}"));
    EXPECT_CALL(handler, Invoke(StrEq("getattr"), GetAttr(1))).Times(AnyNumber())
        .WillRepeatedly(Return("{\"mode\": 420, \"type\": \"dir\"}"));
    EXPECT_CALL(handler, Invoke(StrEq("getattr"), GetAttr(2))).Times(AnyNumber())
        .WillRepeatedly(Return("{\"mode\": 420, \"type\": \"file\", \"size\": 1024000}"));
    EXPECT_CALL(handler, Invoke(StrEq("open"), Open(2))).Times(1)
        .WillOnce(Return("{\"handle\": 42}"));
    EXPECT_CALL(handler, Invoke(StrEq("read"), _)).Times(AnyNumber())
        .WillRepeatedly(Invoke([](char const *, json_t * params) {
            int offset = json_integer_value(json_array_get(params, 3));
            int length = json_integer_value(json_array_get(params, 4));

            int remaining = (offset < 1024000) ? 1024000 - offset : 0;
            int count = (length < remaining) ? length : remaining;

            std::string data = std::string(count, '*');

            json_t * result = json_object();
            json_object_set_new(result, "data", json_string(data.c_str()));
            json_object_set_new(result, "format", json_string("identity"));
            json_object_set_new(result, "count", json_integer(count));

            char * result_text = json_dumps(result, 0);
            std::string result_str = result_text;
            free(result_text);
            json_decref(result);

            return result_str;
        })); 
    EXPECT_CALL(handler, Invoke(StrEq("close"), _)).Times(AtMost(1));
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"add_filesystem\", \"params\": [\"test\"], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * result = json_object_get(response, "result");
    ASSERT_TRUE(json_is_object(result));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    std::string base_dir = server.GetBaseDir();
    ASSERT_TRUE(File(base_dir).isDirectory());
    File file(base_dir + "/test/a.file");
    ASSERT_TRUE(file.readAll());

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}

TEST(server, readdir)
{
    Server server;
    MockInvokationHander handler;
    EXPECT_CALL(handler, Invoke(StrEq("lookup"), _)).Times(AnyNumber());
    EXPECT_CALL(handler, Invoke(StrEq("getattr"), GetAttr(1))).Times(AnyNumber())
        .WillOnce(Return("{\"mode\": 420, \"type\": \"dir\"}"));
    EXPECT_CALL(handler, Invoke(StrEq("readdir"), ReadDir(1))).Times(1)
        .WillOnce(Return("[{\"name\": \"foo\", \"inode\": 23}]"));
    WsClient client(handler, WF_PROTOCOL_NAME_PROVIDER_CLIENT);

    auto connected = client.Connect(server.GetPort(), WF_PROTOCOL_NAME_ADAPTER_SERVER);
    ASSERT_TRUE(connected);

    std::string response_text = client.Invoke("{\"method\": \"add_filesystem\", \"params\": [\"test\"], \"id\": 42}");
    json_t * response = json_loads(response_text.c_str(), 0, nullptr);
    ASSERT_TRUE(json_is_object(response));
    json_t * result = json_object_get(response, "result");
    ASSERT_TRUE(json_is_object(result));
    json_t * id = json_object_get(response, "id");
    ASSERT_EQ(42, json_integer_value(id));
    json_decref(response);

    std::string base_dir = server.GetBaseDir();
    ASSERT_TRUE(File(base_dir).isDirectory());
    File file(base_dir + "/test");
    ASSERT_TRUE(file.hasSubdirectory("foo"));

    auto disconnected = client.Disconnect();
    ASSERT_TRUE(disconnected);
}