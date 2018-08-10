licenses(["notice"])  # Apache 2

# Request buffering and timeout L7 HTTP filter
# Public docs: docs/root/configuration/http_filters/buffer_filter.rst

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

# TODO (CEV): review deps
envoy_cc_library(
    name = "tap_filter_lib",
    srcs = ["tap_filter.cc"],
    hdrs = ["tap_filter.h"],
    deps = [
        "//include/envoy/event:dispatcher_interface",
        "//include/envoy/event:timer_interface",
        "//include/envoy/http:codes_interface",
        "//include/envoy/http:filter_interface",
        "//include/envoy/stats:stats_interface",
        "//include/envoy/stats:stats_macros",
        "//source/common/buffer:buffer_lib",
        "//source/common/common:assert_lib",
        "//source/common/common:enum_to_int",
        "//source/common/http:codes_lib",
        "//source/common/http:header_map_lib",
        "//source/common/http:headers_lib",
        "//source/common/http:utility_lib",
        "//source/extensions/filters/http:well_known_names",
        "@envoy_api//envoy/config/filter/http/tap/v2alpha:tap_cc",
        "@envoy_api//envoy/data/httptap/v2alpha:capture_cc",
    ],
)

# TODO (CEV): review deps
envoy_cc_library(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    deps = [
        "//include/envoy/registry",
        "//source/common/config:filter_json_lib",
        "//source/extensions/filters/http/common:factory_base_lib",
        "//source/extensions/filters/http/tap:tap_filter_lib",
        "//source/extensions/filters/http:well_known_names",
    ],
)
