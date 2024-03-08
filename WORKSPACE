load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "catch2",
    sha256 = "b9b592bd743c09f13ee4bf35fc30eeee2748963184f6bea836b146e6cc2a585a",
    strip_prefix = "Catch2-2.13.8",
    urls = ["https://github.com/catchorg/Catch2/archive/v2.13.8.tar.gz"],
)

http_archive(
    name = "trompeloeil",
    sha256 = "4a1d79260c1e49e065efe0817c8b9646098ba27eed1802b0c3ba7d959e4e5e84",
    strip_prefix = "trompeloeil-47",
    urls = ["https://github.com/rollbear/trompeloeil/archive/refs/tags/v47.tar.gz"],
    build_file = "@//:third_party/BUILD.bazel"
)

