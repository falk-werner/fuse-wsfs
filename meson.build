project('webfuse', 'c', 'cpp', version: '0.5.1', license: 'LGPL-3.0+')

without_tests = get_option('without_tests')

libwebsockets_dep = dependency('libwebsockets', version: '>=4.0.0', required: false)
if not libwebsockets_dep.found()
	cmake = import('cmake')
	libwebsockets = cmake.subproject('libwebsockets')
	libwebsockets_dep = libwebsockets.dependency('websockets_shared')
endif

libfuse_dep = dependency('fuse3', version: '>=3.8.0', fallback: ['fuse3', 'libfuse_dep'])

pkg_config = import('pkgconfig')

inc_dir = include_directories('include')
private_inc_dir = include_directories('include', 'lib')

webfuse_static = static_library('webfuse',
	'lib/webfuse/api.c',
    'lib/webfuse/impl/util/slist.c',
	'lib/webfuse/impl/util/base64.c',
	'lib/webfuse/impl/util/buffer.c',
	'lib/webfuse/impl/util/lws_log.c',
	'lib/webfuse/impl/util/json_util.c',
	'lib/webfuse/impl/util/url.c',
    'lib/webfuse/impl/timer/manager.c',
    'lib/webfuse/impl/timer/timepoint.c',
    'lib/webfuse/impl/timer/timer.c',
	'lib/webfuse/impl/json/writer.c',
	'lib/webfuse/impl/json/node.c',
	'lib/webfuse/impl/json/doc.c',
	'lib/webfuse/impl/json/reader.c',
	'lib/webfuse/impl/json/parser.c',
	'lib/webfuse/impl/jsonrpc/proxy.c',
	'lib/webfuse/impl/jsonrpc/proxy_request_manager.c',
	'lib/webfuse/impl/jsonrpc/proxy_variadic.c',
	'lib/webfuse/impl/jsonrpc/server.c',
	'lib/webfuse/impl/jsonrpc/method.c',
	'lib/webfuse/impl/jsonrpc/request.c',
	'lib/webfuse/impl/jsonrpc/response.c',
	'lib/webfuse/impl/jsonrpc/response_writer.c',
	'lib/webfuse/impl/jsonrpc/error.c',
	'lib/webfuse/impl/message.c',
	'lib/webfuse/impl/message_queue.c',
	'lib/webfuse/impl/status.c',
	'lib/webfuse/impl/filesystem.c',
	'lib/webfuse/impl/server.c',
	'lib/webfuse/impl/server_config.c',
	'lib/webfuse/impl/server_protocol.c',
	'lib/webfuse/impl/session.c',
	'lib/webfuse/impl/session_manager.c',
	'lib/webfuse/impl/authenticator.c',
	'lib/webfuse/impl/authenticators.c',
	'lib/webfuse/impl/credentials.c',
	'lib/webfuse/impl/mountpoint.c',
	'lib/webfuse/impl/mountpoint_factory.c',
	'lib/webfuse/impl/operation/context.c',
	'lib/webfuse/impl/operation/lookup.c',
	'lib/webfuse/impl/operation/getattr.c',
	'lib/webfuse/impl/operation/readdir.c',
	'lib/webfuse/impl/operation/open.c',
	'lib/webfuse/impl/operation/close.c',
	'lib/webfuse/impl/operation/read.c',
	'lib/webfuse/impl/client.c',
	'lib/webfuse/impl/client_protocol.c',
	'lib/webfuse/impl/client_tlsconfig.c',
    c_args: ['-fvisibility=hidden'],
    include_directories: private_inc_dir,
    dependencies: [libfuse_dep, libwebsockets_dep])

webfuse_static_dep = declare_dependency(
	include_directories: inc_dir,
	link_with: [webfuse_static],
	dependencies: [libfuse_dep, libwebsockets_dep])

webfuse = shared_library('webfuse',
    'lib/webfuse/api.c',
    version: meson.project_version(),
    c_args: ['-fvisibility=hidden', '-DWF_API=WF_EXPORT'],
    include_directories: private_inc_dir,
    dependencies: [webfuse_static_dep, libfuse_dep, libwebsockets_dep],
	install: true)

webfuse_dep = declare_dependency(
	include_directories: inc_dir,
	link_with: [webfuse],
	dependencies: [libfuse_dep, libwebsockets_dep])

install_subdir('include/webfuse', install_dir: 'include')

pkg_config.generate(
    libraries: [webfuse],
	requires: ['fuse3', 'libwebsockets'],
    subdirs: '.',
    version: meson.project_version(),
    name: 'libwebfuse',
    filebase: 'webfuse',
    description: 'Websockets filesystem adapter library')

# Unit Tests

if not without_tests

gtest_dep = dependency('gtest', version: '>=1.10.0', fallback: ['gtest', 'gtest_dep']) 
gmock_main_dep = dependency('gmock_main', version: '>=1.10.0', fallback: ['gtest', 'gmock_main_dep'])

fscheck = executable('fs_check',
	'test/webfuse/fs_check/main.c')

openssl = find_program('openssl')
test_server_certs = custom_target('test_server_certs',
	output: ['server-key.pem', 'server-cert.pem'],
	command: [openssl, 'req', '-x509', '-newkey', 'rsa:4096', '-keyout', 'server-key.pem', '-out', 'server-cert.pem', '-days', '365', '-nodes', '-batch', '-subj', '/CN=localhost'])
test_client_certs = custom_target('test_client_certs',
	output: ['client-key.pem', 'client-cert.pem'],
	command: [openssl, 'req', '-x509', '-newkey', 'rsa:4096', '-keyout', 'client-key.pem', '-out', 'client-cert.pem', '-days', '365', '-nodes', '-batch', '-subj', '/CN=localhost'])

test_certs_dep = declare_dependency(
	sources: [test_server_certs, test_client_certs])

alltests = executable('alltests',
	'test/webfuse/json/test_writer.cc',
	'test/webfuse/json/test_doc.cc',
	'test/webfuse/json/test_node.cc',
	'test/webfuse/json/test_reader.cc',
	'test/webfuse/json/test_parser.cc',
	'test/webfuse/jsonrpc/mock_timer_callback.cc',
	'test/webfuse/jsonrpc/mock_timer.cc',
	'test/webfuse/jsonrpc/test_is_request.cc',
	'test/webfuse/jsonrpc/test_request.cc',
	'test/webfuse/jsonrpc/test_is_response.cc',
	'test/webfuse/jsonrpc/test_response.cc',
	'test/webfuse/jsonrpc/test_server.cc',
	'test/webfuse/jsonrpc/test_proxy.cc',
	'test/webfuse/jsonrpc/test_response_parser.cc',
	'test/webfuse/timer/test_timepoint.cc',
	'test/webfuse/timer/test_timer.cc',
	'test/webfuse/test_util/mountpoint_factory.cc',
	'test/webfuse/test_util/tempdir.cc',
	'test/webfuse/test_util/server.cc',
	'test/webfuse/test_util/server_protocol.cc',
	'test/webfuse/test_util/ws_server.cc',
	'test/webfuse/test_util/ws_client.cc',
	'test/webfuse/test_util/adapter_client.cc',
	'test/webfuse/test_util/file.cc',
	'test/webfuse/test_util/lws_test_environment.cc',
	'test/webfuse/test_util/json_doc.cc',
	'test/webfuse/mocks/mock_authenticator.cc',
	'test/webfuse/mocks/mock_fuse.cc',
	'test/webfuse/mocks/mock_operation_context.cc',
	'test/webfuse/mocks/mock_jsonrpc_proxy.cc',
	'test/webfuse/mocks/mock_adapter_client_callback.cc',
	'test/webfuse/util/test_util.cc',
	'test/webfuse/util/test_container_of.cc',
	'test/webfuse/util/test_slist.cc',
	'test/webfuse/util/test_base64.cc',
	'test/webfuse/util/test_buffer.cc',
	'test/webfuse/util/test_url.cc',
	'test/webfuse/test_status.cc',
	'test/webfuse/test_message.cc',
	'test/webfuse/test_message_queue.cc',
	'test/webfuse/test_server.cc',
	'test/webfuse/test_server_protocol.cc',
	'test/webfuse/test_server_config.cc',
	'test/webfuse/test_credentials.cc',
	'test/webfuse/test_authenticator.cc',
	'test/webfuse/test_authenticators.cc',
	'test/webfuse/test_mountpoint.cc',
	'test/webfuse/test_fuse_req.cc',
	'test/webfuse/operation/test_context.cc',
	'test/webfuse/operation/test_open.cc',
	'test/webfuse/operation/test_close.cc',
	'test/webfuse/operation/test_read.cc',
	'test/webfuse/operation/test_readdir.cc',
	'test/webfuse/operation/test_getattr.cc',
	'test/webfuse/operation/test_lookup.cc',
	'test/webfuse/test_client.cc',
	'test/webfuse/test_client_tlsconfig.cc',
	link_args: [
		'-Wl,--wrap=wf_impl_timer_manager_create',
		'-Wl,--wrap=wf_impl_timer_manager_dispose',
		'-Wl,--wrap=wf_impl_timer_manager_check',
		'-Wl,--wrap=wf_impl_timer_create',
		'-Wl,--wrap=wf_impl_timer_dispose',
		'-Wl,--wrap=wf_impl_timer_start',
		'-Wl,--wrap=wf_impl_timer_cancel',
		'-Wl,--wrap=wf_impl_operation_context_get_proxy',
		'-Wl,--wrap=wf_impl_jsonrpc_proxy_vinvoke',
		'-Wl,--wrap=wf_impl_jsonrpc_proxy_vnotify',
		'-Wl,--wrap=fuse_req_userdata',
		'-Wl,--wrap=fuse_reply_open',
		'-Wl,--wrap=fuse_reply_err',
		'-Wl,--wrap=fuse_reply_buf',
		'-Wl,--wrap=fuse_reply_attr',
		'-Wl,--wrap=fuse_reply_entry',
		'-Wl,--wrap=fuse_req_ctx'
	],
	include_directories: [private_inc_dir, 'test'],
	dependencies: [
		webfuse_static_dep,
		libwebsockets_dep,
		libfuse_dep,
		gtest_dep,
		gmock_main_dep,
		test_certs_dep
	])

test('alltests', alltests)

endif