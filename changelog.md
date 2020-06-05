# webfuse changelog

## 0.3.1 _(Fri Jum 05 2020)_

### Fixes

*   Fix meson build failure when gtest and jansson are not installed

## 0.3.0 _(Fri Jun 05 2020)_

### Breaking Changes

*   Remove argument `timeout_ms` in  `wf_server_service` and `wfp_client_service`
*   Remove `wfp_client_ontimer`

### New Features

*   Add meson build support
*   Add `wf_server_interrupt` and `wfp_client_interrupt`

## 0.2.0 _(Sun Mar 01 2020)_

### New Features

*   Add authentication support of provider
*   Add API documentation (doxygen)
*   Add `mountpoint_factory`
*   Add option to build adapter and provider libraries separately

### Fixes

*   Fix crash of example when using option `-h`
*   Fix error in `static_filesystem` providen when adding multiple files to same directory

## 0.1.0 _(Sat Apr 27 2019)_

*   initial version
