# .ycm_extra_conf.py for nvim source code.
#
# Notes:
#  - replace '-iquote .' with '-iquote include'
#  - remove -o and -c flags

import os
import ycm_core

DIR_OF_THIS_SCRIPT = os.path.abspath(os.path.dirname(__file__))

database = ycm_core.CompilationDatabase(DIR_OF_THIS_SCRIPT)

extra_flags = [
    '-iquote',
    'source',
]


def GetCompilationInfoForFile(filename):
    if not database:
        return None
    return database.GetCompilationInfoForFile(filename)


# It seems YCM does not resolve directories correctly. This function will
# adjust paths in the compiler flags to be absolute
def FixDirectories(args, compiler_working_dir):
    def adjust_path(path):
        return os.path.abspath(os.path.join(compiler_working_dir, path))

    adjust_next_arg = False
    new_args = []
    for arg in args:
        if adjust_next_arg:
            arg = adjust_path(arg)
            adjust_next_arg = False
        else:
            # removed: -iquote
            for dir_flag in ['-I', '-isystem', '-o', '-c']:
                if arg.startswith(dir_flag):
                    if arg != dir_flag:
                        # flag and path are concatenated in same arg
                        path = arg[len(dir_flag):]
                        new_path = adjust_path(path)
                        arg = '{0}{1}'.format(dir_flag, new_path)
                    else:
                        # path is specified in next argument
                        adjust_next_arg = True
        new_args.append(arg)
    return new_args


def FindCorrespondingSourceFile(filename):
    basename, extension = os.path.splitext(filename)
    if extension in ['.h', '.hxx', '.hpp', '.hh']:
        for extension in ['.cc', '.cpp', '.cxx', '.c']:
            replacement_file = basename + extension
            if os.path.exists(replacement_file):
                return replacement_file
    return filename


def Settings(**kwargs):
    if kwargs['language'] == 'cfamily':
        # If the file is a header, try to find the corresponding source file and
        # retrieve its flags from the compilation database if using one. This is
        # necessary since compilation databases don't have entries for header files.
        # In addition, use this source file as the translation unit. This makes it
        # possible to jump from a declaration in the header file to its definition
        # in the corresponding source file.
        filename = FindCorrespondingSourceFile(kwargs['filename'])

        # TODO (CEV): add some sane default flags
        if not database:
            return {}

        compilation_info = database.GetCompilationInfoForFile(filename)
        if not compilation_info.compiler_flags_:
            return {}

        # Bear in mind that compilation_info.compiler_flags_ does NOT return a
        # python list, but a 'list-like' StringVec object.
        final_flags = FixDirectories(list(compilation_info.compiler_flags_),
                                     compilation_info.compiler_working_dir_)
        return {
            'flags': final_flags,
            'include_paths_relative_to_dir': compilation_info.compiler_working_dir_,
            'override_filename': filename,
        }
    return {}

# def FlagsForFile(filename):
#     if database:
#         # If the file is a header, try to find the corresponding source file and
#         # retrieve its flags from the compilation database if using one. This is
#         # necessary since compilation databases don't have entries for header files.
#         # In addition, use this source file as the translation unit. This makes it
#         # possible to jump from a declaration in the header file to its definition
#         # in the corresponding source file.
#         # filename = FindCorrespondingSourceFile(filename)
#
#         compilation_info = database.GetCompilationInfoForFile(filename)
#         if compilation_info and compilation_info.compiler_flags_:
#             # Add flags not needed for clang-the-binary,
#             # but needed for libclang-the-library (YCM uses this last one).
#
#             # flags = FixDirectories(list(compilation_info.compiler_flags_),
#             #                        compilation_info.compiler_working_dir_)
#             return {
#                 'flags': list(compilation_info.compiler_flags_),
#                 # 'flags': flags,
#                 'do_cache': True,
#             }
#     return {}
