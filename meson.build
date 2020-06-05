project('webfuse', 'c', 'cpp', version: '0.3.0', license: 'LGPL-3.0+')

without_adapter = get_option('without_adapter')
without_provider = get_option('without_provider')
without_tests = get_option('without_tests') or without_adapter or without_provider

libwebsockets_dep = dependency('libwebsockets', version: '>=4.0.13', required: false)
if not libwebsockets_dep.found()
	cmake = import('cmake')
	libwebsockets = cmake.subproject('libwebsockets')
	libwebsockets_dep = libwebsockets.dependency('websockets_shared')
endif

jansson_dep = dependency('jansson', version: '>=2.11', fallback: ['jansson', 'jansson_dep'])

pkg_config = import('pkgconfig')

# Webfuse core

inc_dir = include_directories('include')
private_inc_dir = include_directories('include', 'lib')

webfuse_core = static_library('webfuse_core',
    'lib/webfuse/core/slist.c',
	'lib/webfuse/core/message.c',
	'lib/webfuse/core/message_queue.c',
	'lib/webfuse/core/status.c',
	'lib/webfuse/core/string.c',
	'lib/webfuse/core/base64.c',
	'lib/webfuse/core/lws_log.c',
	'lib/webfuse/core/json_util.c',
    'lib/webfuse/core/timer/manager.c',
    'lib/webfuse/core/timer/timepoint.c',
    'lib/webfuse/core/timer/timer.c',
	'lib/webfuse/core/jsonrpc/proxy.c',
	'lib/webfuse/core/jsonrpc/proxy_variadic.c',
	'lib/webfuse/core/jsonrpc/server.c',
	'lib/webfuse/core/jsonrpc/method.c',
	'lib/webfuse/core/jsonrpc/request.c',
	'lib/webfuse/core/jsonrpc/response.c',
	'lib/webfuse/core/jsonrpc/error.c',
    c_args: ['-fvisibility=hidden'],
    include_directories: private_inc_dir,    
    dependencies: [jansson_dep, libwebsockets_dep])

webfuse_core_dep = declare_dependency(
    include_directories: inc_dir,
    link_with: webfuse_core,
	dependencies: [jansson_dep, libwebsockets_dep])

install_subdir('include/webfuse/core', install_dir: 'include/webfuse')

# Webfuse provider

if not without_provider

webfuse_provider_static = static_library('webfuse_provider',
	'lib/webfuse/provider/api.c',
	'lib/webfuse/provider/impl/url.c',
	'lib/webfuse/provider/impl/client.c',
	'lib/webfuse/provider/impl/client_config.c',
	'lib/webfuse/provider/impl/client_protocol.c',
	'lib/webfuse/provider/impl/provider.c',
	'lib/webfuse/provider/impl/request.c',
	'lib/webfuse/provider/impl/dirbuffer.c',
	'lib/webfuse/provider/impl/credentials.c',
	'lib/webfuse/provider/impl/operation/lookup.c',
	'lib/webfuse/provider/impl/operation/getattr.c',
	'lib/webfuse/provider/impl/operation/readdir.c',
	'lib/webfuse/provider/impl/operation/open.c',
	'lib/webfuse/provider/impl/operation/close.c',
	'lib/webfuse/provider/impl/operation/read.c',
    c_args: ['-fvisibility=hidden'],
    include_directories: private_inc_dir,
    dependencies: [webfuse_core_dep])

webfuse_provider_static_dep = declare_dependency(
    include_directories: inc_dir,
    link_with: webfuse_provider_static,
	dependencies: [webfuse_core_dep])

webfuse_provider = shared_library('webfuse_provider',
    'lib/webfuse/provider/api.c',
    version: meson.project_version(),
    c_args: ['-fvisibility=hidden', '-DWFP_API=WFP_EXPORT'],
    include_directories: private_inc_dir,
    dependencies: [webfuse_provider_static_dep],
	install: true)

webfuse_provider_dep = declare_dependency(
	include_directories: inc_dir,
	link_with: [webfuse_provider],
	dependencies: [libwebsockets_dep, jansson_dep])

install_headers('include/webfuse_provider.h', subdir: 'webfuse')
install_subdir('include/webfuse/provider', install_dir: 'include/webfuse')

pkg_config.generate(
    libraries: [webfuse_provider, jansson_dep, libwebsockets_dep],
    subdirs: '.',
    version: meson.project_version(),
    name: 'libwebfuse_provider',
    filebase: 'webfuse_provider',
    description: 'Provider library for websockets filesystem')

endif

#  Webfuse adapter

if not without_adapter

libfuse_dep = dependency('fuse3', version: '>=3.8.0', fallback: ['fuse3', 'libfuse_dep'])

webfuse_adapter_static = static_library('webfuse_adapter',
	'lib/webfuse/adapter/api.c',
	'lib/webfuse/adapter/impl/filesystem.c',
	'lib/webfuse/adapter/impl/server.c',
	'lib/webfuse/adapter/impl/server_config.c',
	'lib/webfuse/adapter/impl/server_protocol.c',
	'lib/webfuse/adapter/impl/session.c',
	'lib/webfuse/adapter/impl/session_manager.c',
	'lib/webfuse/adapter/impl/authenticator.c',
	'lib/webfuse/adapter/impl/authenticators.c',
	'lib/webfuse/adapter/impl/credentials.c',
	'lib/webfuse/adapter/impl/mountpoint.c',
	'lib/webfuse/adapter/impl/mountpoint_factory.c',
	'lib/webfuse/adapter/impl/operation/context.c',
	'lib/webfuse/adapter/impl/operation/lookup.c',
	'lib/webfuse/adapter/impl/operation/getattr.c',
	'lib/webfuse/adapter/impl/operation/readdir.c',
	'lib/webfuse/adapter/impl/operation/open.c',
	'lib/webfuse/adapter/impl/operation/close.c',
	'lib/webfuse/adapter/impl/operation/read.c',
    c_args: ['-fvisibility=hidden'],
    include_directories: private_inc_dir,
    dependencies: [webfuse_core_dep, libfuse_dep])

webfuse_adapter_static_dep = declare_dependency(
	include_directories: inc_dir,
	link_with: [webfuse_adapter_static],
	dependencies: [webfuse_core_dep, libfuse_dep])

webfuse_adapter = shared_library('webfuse_adapter',
    'lib/webfuse/adapter/api.c',
    version: meson.project_version(),
    c_args: ['-fvisibility=hidden', '-DWF_API=WF_EXPORT'],
    include_directories: private_inc_dir,
    dependencies: [webfuse_adapter_static_dep, libfuse_dep],
	install: true)

webfuse_adapter_dep = declare_dependency(
	include_directories: inc_dir,
	link_with: [webfuse_adapter],
	dependencies: [libfuse_dep, libwebsockets_dep, jansson_dep])

install_headers('include/webfuse_adapter.h', subdir: 'webfuse')
install_subdir('include/webfuse/adapter', install_dir: 'include/webfuse')

pkg_config.generate(
    libraries: [webfuse_adapter, jansson_dep, libwebsockets_dep, libfuse_dep],
    subdirs: '.',
    version: meson.project_version(),
    name: 'libwebfuse_adapter',
    filebase: 'webfuse_adapter',
    description: 'Websockets filesystem server library')

endif

# Unit Tests

if not without_tests

gtest_dep = dependency('gtest', version: '>=1.10.0', fallback: ['gtest', 'gtest_dep']) 
gmock_main_dep = dependency('gmock_main', version: '>=1.10.0', fallback: ['gtest', 'gmock_main_dep'])

fscheck = executable('fs_check',
	'test/webfuse/tests/integration/fs_check.c')

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
	'test/webfuse/tests/core/jsonrpc/mock_timer_callback.cc',
	'test/webfuse/tests/core/jsonrpc/mock_timer.cc',
	'test/webfuse/tests/core/jsonrpc/test_is_request.cc',
	'test/webfuse/tests/core/jsonrpc/test_request.cc',
	'test/webfuse/tests/core/jsonrpc/test_is_response.cc',
	'test/webfuse/tests/core/jsonrpc/test_response.cc',
	'test/webfuse/tests/core/jsonrpc/test_server.cc',
	'test/webfuse/tests/core/jsonrpc/test_proxy.cc',
	'test/webfuse/tests/core/jsonrpc/test_response_parser.cc',
	'test/webfuse/tests/core/timer/test_timepoint.cc',
	'test/webfuse/tests/core/timer/test_timer.cc',
	'test/webfuse/utils/tempdir.cc',
	'test/webfuse/utils/file_utils.cc',
	'test/webfuse/utils/timeout_watcher.cc',
	'test/webfuse/utils/path.c',
	'test/webfuse/utils/static_filesystem.c',
	'test/webfuse/utils/ws_server.cc',
	'test/webfuse/mocks/fake_invokation_context.cc',
	'test/webfuse/mocks/mock_authenticator.cc',
	'test/webfuse/mocks/mock_request.cc',
	'test/webfuse/mocks/mock_provider_client.cc',
	'test/webfuse/mocks/mock_provider.cc',
	'test/webfuse/mocks/mock_fuse.cc',
	'test/webfuse/mocks/mock_operation_context.cc',
	'test/webfuse/mocks/mock_jsonrpc_proxy.cc',
	'test/webfuse//tests/core/test_util.cc',
	'test/webfuse/tests/core/test_container_of.cc',
	'test/webfuse/tests/core/test_string.cc',
	'test/webfuse/tests/core/test_slist.cc',
	'test/webfuse/tests/core/test_base64.cc',
	'test/webfuse/tests/core/test_status.cc',
	'test/webfuse/tests/core/test_message.cc',
	'test/webfuse/tests/core/test_message_queue.cc',
	'test/webfuse/tests/adapter/test_server.cc',
	'test/webfuse/tests/adapter/test_server_config.cc',
	'test/webfuse/tests/adapter/test_credentials.cc',
	'test/webfuse/tests/adapter/test_authenticator.cc',
	'test/webfuse/tests/adapter/test_authenticators.cc',
	'test/webfuse/tests/adapter/test_mountpoint.cc',
	'test/webfuse/tests/adapter/test_fuse_req.cc',
	'test/webfuse/tests/adapter/operation/test_context.cc',
	'test/webfuse/tests/adapter/operation/test_open.cc',
	'test/webfuse/tests/adapter/operation/test_close.cc',
	'test/webfuse/tests/adapter/operation/test_read.cc',
	'test/webfuse/tests/adapter/operation/test_readdir.cc',
	'test/webfuse/tests/adapter/operation/test_getattr.cc',
	'test/webfuse/tests/adapter/operation/test_lookup.cc',
	'test/webfuse/tests/provider/test_url.cc',
	'test/webfuse/tests/provider/test_client_protocol.cc',
	'test/webfuse/tests/provider/operation/test_close.cc',
	'test/webfuse/tests/provider/operation/test_getattr.cc',
	'test/webfuse/tests/provider/operation/test_lookup.cc',
	'test/webfuse/tests/provider/operation/test_open.cc',
	'test/webfuse/tests/provider/operation/test_read.cc',
	'test/webfuse/tests/provider/operation/test_readdir.cc',
	'test/webfuse/tests/integration/test_lowlevel.cc',
	'test/webfuse/tests/integration/test_integration.cc',
	'test/webfuse/tests/integration/file.cc',
	'test/webfuse/tests/integration/server.cc',
	'test/webfuse/tests/integration/provider.cc',
	link_args: [
		'-Wl,--wrap=wf_timer_manager_create',
		'-Wl,--wrap=wf_timer_manager_dispose',
		'-Wl,--wrap=wf_timer_manager_check',
		'-Wl,--wrap=wf_timer_create',
		'-Wl,--wrap=wf_timer_dispose',
		'-Wl,--wrap=wf_timer_start',
		'-Wl,--wrap=wf_timer_cancel',
		'-Wl,--wrap=wf_impl_operation_context_get_proxy',
		'-Wl,--wrap=wf_jsonrpc_proxy_vinvoke',
		'-Wl,--wrap=wf_jsonrpc_proxy_vnotify',
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
		webfuse_adapter_static_dep,
		webfuse_provider_static_dep,
		webfuse_core_dep,
		libwebsockets_dep,
		libfuse_dep,
		jansson_dep,
		gtest_dep,
		gmock_main_dep,
		test_certs_dep
	])

test('alltests', alltests)

endif