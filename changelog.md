# webfuse changelog

## 0.5.1 _(Sat Oct 17 2020)_

*   __Fix:__ Fix compatibilty issue with libwebsockets 4.1.3 (removed obsolete ping-pong interval)

## 0.5.0 _(Sun Jul 19 2020)_

*   __Feature:__ Remove dependency to libjansson
*   __Feature:__ Increase performace
*   __Fix:__ Fix deadlock reading files larger that a few KBytes (Isseue #82)

## 0.4.0 _(Sun Jul 05 2020)_

*   __Breaking Change:__ Remove CMake support (change build system to meson)
*   __Breaking Change:__ Make argument credentials const in `wf_authenticate_fn`
*   __Breaking Change:__ Moved provider library into [separate project](https://github.com/falk-werner/webfuse-provider)
*   __Breaking Change:__Renamed library (libwebfuse_adapter -> libwebfuse)
*   __Feature:__ Add adapter client implementation
*   __Feature:__ Allow system to choose port of webfuse server (by setting port in `wf_server_config` to 0)
*   __Fix:__ Fix meson build failure when gtest or jansson are not installed
*   __Fix:__ Fix crash if libfuse is not available at runtime

## 0.3.0 _(Fri Jun 05 2020)_

*   __Breaking Change:__ Remove argument `timeout_ms` in  `wf_server_service` and `wfp_client_service`
*   __Breaking Change:__ Remove `wfp_client_ontimer`
*   __Feature:__ Add meson build support
*   __Feature:__ Add `wf_server_interrupt` and `wfp_client_interrupt`

## 0.2.0 _(Sun Mar 01 2020)_

*   __Feature:__ Add authentication support of provider
*   __Feature:__ Add API documentation (doxygen)
*   __Feature:__ Add `mountpoint_factory`
*   __Feature:__ Add option to build adapter and provider libraries separately
*   __Fix:__ Fix crash of example when using option `-h`
*   __Fix:__ Fix error in `static_filesystem` providen when adding multiple files to same directory

## 0.1.0 _(Sat Apr 27 2019)_

*   initial version
