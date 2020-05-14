#new_local_repository(
    #name = "gtest",
    #path = "/usr/local/",
    #build_file = "gtest.BUILD"
#)

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
git_repository(
    name = "gtest",
    remote = "https://github.com/google/googletest",
    branch = "v1.10.x",
)
