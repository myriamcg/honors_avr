#!/usr/bin/env python3
#
# american fuzzy lop++ - custom code formatter
# --------------------------------------------
#
# Written and maintained by Andrea Fioraldi <andreafioraldi@gmail.com>
#
# Copyright 2015, 2016, 2017 Google Inc. All rights reserved.
# Copyright 2019-2023 AFLplusplus Project. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#

import subprocess
import sys
import os
# import re # TODO: for future use
import shutil
import importlib.metadata
import hashlib

# string_re = re.compile('(\\"(\\\\.|[^"\\\\])*\\")') # TODO: for future use

CURRENT_LLVM = os.getenv('LLVM_VERSION', 18)
CLANG_FORMAT_BIN = os.getenv("CLANG_FORMAT_BIN", "")

FORMAT_CACHE_DIR = '.format-cache'
os.makedirs(FORMAT_CACHE_DIR, exist_ok=True)

def check_clang_format_pip_version():
    """
    Check if the correct version of clang-format is installed via pip.

    Returns:
        bool: True if the correct version of clang-format is installed,
        False otherwise.
    """
    # Check if clang-format is installed
    if importlib.util.find_spec('clang_format'):
        # Check if the installed version is the expected LLVM version
        if importlib.metadata.version('clang-format')\
                .startswith(str(CURRENT_LLVM)+'.'):
            return True
        else:
            # Return False, because the clang-format version does not match
            return False
    else:
        # If the 'clang_format' package isn't installed, return False
        return False


with open(".clang-format") as f:
    fmt = f.read()


CLANG_FORMAT_PIP = check_clang_format_pip_version()

if shutil.which(CLANG_FORMAT_BIN) is None:
    CLANG_FORMAT_BIN = f"clang-format-{CURRENT_LLVM}"

if shutil.which(CLANG_FORMAT_BIN) is None \
        and CLANG_FORMAT_PIP is False:
    print(f"[!] clang-format-{CURRENT_LLVM} is needed. Aborted.")
    print(f"Run `pip3 install \"clang-format=={CURRENT_LLVM}.*\"` \
to install via pip.")
    exit(1)

if CLANG_FORMAT_PIP:
    CLANG_FORMAT_BIN = shutil.which("clang-format")

CLANG_FORMAT_VERSION = subprocess.check_output([CLANG_FORMAT_BIN, '--version'])

COLUMN_LIMIT = 80
for line in fmt.split("\n"):
    line = line.split(":")
    if line[0].strip() == "ColumnLimit":
        COLUMN_LIMIT = int(line[1].strip())


def custom_format(filename):
    p = subprocess.Popen([CLANG_FORMAT_BIN, filename], stdout=subprocess.PIPE)
    src, _ = p.communicate()
    src = str(src, "utf-8")

    in_define = False
    last_line = None
    out = ""

    for line in src.split("\n"):
        define_start = False
        if line.lstrip().startswith("#"):
            if line[line.find("#") + 1:].lstrip().startswith("define"):
                define_start = True

        if (
                "/*" in line
                and not line.strip().startswith("/*")
                and line.endswith("*/")
                and len(line) < (COLUMN_LIMIT - 2)
        ):
            cmt_start = line.rfind("/*")
            line = (
                    line[:cmt_start]
                    + " " * (COLUMN_LIMIT - 2 - len(line))
                    + line[cmt_start:]
            )

        define_padding = 0
        if last_line is not None and in_define and last_line.endswith("\\"):
            last_line = last_line[:-1]
            define_padding = max(0, len(last_line[last_line.rfind("\n") + 1:]))

        if (
                last_line is not None
                and last_line.strip().endswith("{")
                and line.strip() != ""
        ):
            line = (" " * define_padding + "\\" if in_define else "") + "\n" + line
        elif (
                last_line is not None
                and last_line.strip().startswith("}")
                and line.strip() != ""
        ):
            line = (" " * define_padding + "\\" if in_define else "") + "\n" + line
        elif (
                line.strip().startswith("}")
                and last_line is not None
                and last_line.strip() != ""
        ):
            line = (" " * define_padding + "\\" if in_define else "") + "\n" + line
        in_define = (define_start or in_define) and line.endswith("\\")

        out += line + "\n"
        last_line = line

    return out


def hash_code_and_formatter(code):
    hasher = hashlib.sha256()

    hasher.update(code.encode())
    hasher.update(CLANG_FORMAT_VERSION)
    with open(__file__, 'rb') as f:
        hasher.update(f.read())

    return hasher.hexdigest()


def custom_format_cached(filename):
    filename_hash = hashlib.sha256(filename.encode()).hexdigest()
    cache_file = os.path.join(FORMAT_CACHE_DIR, filename_hash)

    if os.path.exists(cache_file):
        with open(filename) as f:
            code = f.read()
        code_hash = hash_code_and_formatter(code)
        with open(cache_file) as f:
            if f.read() == code_hash:
                return code

    code = custom_format(filename)

    code_hash = hash_code_and_formatter(code)
    with open(cache_file, 'w') as f:
        f.write(code_hash)

    return code


args = sys.argv[1:]
if len(args) == 0:
    print("Usage: ./format.py [-i] <filename>")
    print()
    print(" The -i option, if specified, let the script to modify in-place")
    print(" the source files. By default the results are written to stdout.")
    print()
    exit(1)

in_place = False
if args[0] == "-i":
    in_place = True
    args = args[1:]

for filename in args:
    code = custom_format_cached(filename)
    if in_place:
        with open(filename, "w") as f:
            f.write(code)
    else:
        print(code)
