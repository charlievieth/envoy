# This is the implementation of a Bazel extra_action which generates
# _compile_command files for generate_compile_commands.py to consume.

import sys
import json

import third_party.bazel.protos.extra_actions_base_pb2 as extra_actions_base_pb2


EXCLUDED_OPTIONS = [
    "-isysroot __BAZEL_XCODE_SDKROOT__",
    "-std=c++11",
    "-D__DATE__=\"redacted\"",
    "-D__TIMESTAMP__=\"redacted\"",
    "-D__TIME__=\"redacted\"",
]


def fix_options(options):
    return [x for x in options if x not in EXCLUDED_OPTIONS]


def get_cpp_command(cpp_compile_info):
    options = ["g++"]
    options += list(cpp_compile_info.compiler_option)
    return {
        "command": fix_options(options),
        "source": cpp_compile_info.source_file,
    }


def main(argv):
    action = extra_actions_base_pb2.ExtraActionInfo()
    with open(argv[1], "rb") as f:
        action.MergeFromString(f.read())
        command = get_cpp_command(
            action.Extensions[
                extra_actions_base_pb2.CppCompileInfo.cpp_compile_infoi
            ]
        )
    with open(argv[2], "w") as f:
        json.dump(command, f)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
