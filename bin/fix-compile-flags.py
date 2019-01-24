#!/usr/bin/env python

# TODO:
#  - replace '-iquote .' with '-iquote include'
#  - replace __BAZEL_XCODE_SDKROOT__

import json
from typing import List, FrozenSet


SUPPORTED_LANGUAGE_STANDARDS = frozenset([
    'c++03', 'c++11', 'c++14', 'c++98', 'c11', 'c17', 'c18', 'c89',
    'c90', 'c99', 'gnu++03', 'gnu++11', 'gnu++14', 'gnu++1y',
    'gnu++98', 'gnu11', 'gnu17', 'gnu18', 'gnu89', 'gnu90', 'gnu99',
])

PREFIXES = frozenset(['-D__DATE__=', '-D__TIMESTAMP__=', '-D__TIME__='])
SDK_ROOT = '/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk'


def remove_arg_pair(flag: str, args: List[str], debug: bool = False) -> List[str]:
    removed: List[str] = []
    while flag in args:
        n = args.index(flag)
        if n + 2 <= len(args):
            removed += args[n: n + 2]
            del args[n: n + 2]
        else:
            removed += args[n: n + 1]
            del args[n]
    if debug:
        print(f'removed ({flag}): {removed}')
    return args


def remove_prefix(text: str, prefix: str):
    if text.startswith(prefix):
        return text[len(prefix):]
    return text


def fix_std_flag(std: str, args: List[str]) -> List[str]:
    std = remove_prefix(std, '-std=')
    if std not in SUPPORTED_LANGUAGE_STANDARDS:
        raise ValueError(f'invalid std: {std}')
    args = [x for x in args if not x.startswith('-std=')]
    args.append('-std=' + std)
    return args


def add_clang_language_flag(language: str, args: List[str]) -> List[str]:
    if language not in ('c', 'c++', 'objc', 'cuda'):
        raise ValueError(f'invalid language: {language}')
    args = remove_arg_pair('-x', args)
    args += ['-x', language]
    return args


def remove_flag_prefix(prefixes: FrozenSet[str], args: List[str]) -> List[str]:
    def has_prefix(s: str):
        for prefix in prefixes:
            if s.startswith(prefix):
                return True
        return False
    args = [x for x in args if not has_prefix(x)]
    return args


def replace_envoy_cc(cc: str, args: List[str]) -> List[str]:
    if len(args) == 0:
        raise ValueError(f'invalid args: {args}')
    args[0] = cc
    return args


def replace_string(old: str, new: str, args: List[str]) -> List[str]:
    if old in args:
        args[args.index(old)] = new
    return args


def fix_ubunt_flags(args: List[str]) -> List[str]:
    args = remove_arg_pair('-c', args)
    args = remove_arg_pair('-o', args)
    args = fix_std_flag('c++14', args)
    args = add_clang_language_flag('c++', args)
    args = remove_flag_prefix(PREFIXES, args)
    args = replace_envoy_cc('clang', args)
    args = replace_string('__BAZEL_XCODE_SDKROOT__', SDK_ROOT, args)
    return args


COMPILE_COMMANDS_FILE = '/Users/cvieth/src/envoy-private/envoy/compile_commands.json'

with open(COMPILE_COMMANDS_FILE, 'r') as fp:
    commands = json.load(fp)


for cmd in commands:
    args = fix_ubunt_flags(cmd['command'].split(' '))
    cmd['command'] = ' '.join(args)


with open(COMPILE_COMMANDS_FILE + '.bak', 'w') as fp:
    json.dump(commands, fp, indent=4, sort_keys=True)


# args = [
#     '-Wno-free-nonheap-object',
#     '-fno-omit-frame-pointer',
#     '-std=c++0x',
#     '-std=c++14',
#     '-x',
#     'cxx',
#     '-O3',
#     '-I',
#     'SOME_DIR',
#     '-D__DATE__=\"redacted\"',
#     '-D__TIMESTAMP__=\"redacted\"',
#     '-D__TIME__=\"redacted\"',
# ]


# args = fix_std_flag('-std=c++14', args)
# print(f'args: {args}')

# args = add_clang_language_flag('c++', args)
# print(f'args: {args}')

# prefixes = [
#     '-D__DATE__=',
#     '-D__TIMESTAMP__=',
#     '-D__TIME__=',
# ]
# args = remove_flag_prefix(prefixes, args)
# print(f'args: {args}')

# # /home/cev/.cache/bazel/_bazel_cev/0d98ffd96e20e1a74336e148d66a4eae/external/local_config_cc/extra_tools/envoy_cc_wrapper
# # external/local_config_cc/wrapped_clang
