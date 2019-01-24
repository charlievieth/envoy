#!/usr/bin/env python3

# This reads the _compile_command files :generate_compile_commands_action
# generates a outputs a compile_commands.json file at the top of the source
# tree for things like clang-tidy to read.

# Overall usage directions: run Bazel with
# --experimental_action_listener=//tools/actions:generate_compile_commands_listener
# for all the files you want to use clang-tidy with and then run this script.
# After that, `clang-tidy build_tests/gflags.cc` should work.

import sys
import pathlib
import json
import os.path
import subprocess


def get_command(path, command_directory):
    """
    Args:
      path: The pathlib.Path to _compile_command file.
      command_directory: The directory commands are run from.
    Returns a string to stick in compile_commands.json.
    """
    with path.open("r") as f:
        command = json.load(f)
        command["directory"] = command_directory,
        return command
    return None


def get_compile_commands(path, command_directory):
    """
    Args:
      path: A directory pathlib.Path to look for _compile_command files under.
      command_directory: The directory commands are run from.
    Yields strings to stick in compile_commands.json.
    """
    for f in path.iterdir():
        if f.is_dir():
            yield from get_compile_commands(f, command_directory)
        elif f.name.endswith("_compile_command"):
            command = get_command(f, command_directory)
            if command:
                yield command


def main(argv):
    source_path = os.path.join(os.path.dirname(__file__), "../..")
    action_outs = os.path.join(
        source_path,
        "bazel-bin/../extra_actions",
        "tools/actions/generate_compile_commands_action",
    )
    bazel_cmd = ("bazel", "info", "execution_root")
    command_directory = (
        subprocess.check_output(bazel_cmd, cwd=source_path)
        .decode("utf-8")
        .rstrip()
    )
    commands = get_compile_commands(
        pathlib.Path(action_outs),
        command_directory,
    )
    cmds = [{
        "command": x["command"],
        "source": x["source"],
        "directory": x["directory"],
    } for x in commands]
    with open(os.path.join(source_path, "compile_commands.json"), "w") as f:
        json.dump(cmds, f, indent=4, sort_keys=True)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
