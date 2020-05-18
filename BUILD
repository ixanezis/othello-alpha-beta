load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "main",
    srcs = ["main.cpp", "mask.h", "common.h"],
)

cc_test(
    name = "test-mask",
    srcs = ["test-mask.cpp", "mask.h"],
    deps = ["@gtest//:gtest"],
)

cc_test(
    name = "test-common",
    srcs = ["test-common.cpp", "mask.h", "common.h"],
    deps = ["@gtest//:gtest"],
)

cc_test(
    name = "test-mask10",
    srcs = ["test-mask10.cpp", "mask10.h"],
    deps = ["@gtest//:gtest"],
)
