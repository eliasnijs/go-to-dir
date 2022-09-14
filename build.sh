#!/bin/sh


# 1. File information
input="src/main.c"
output="build/build"

# 2. Flags

# 2.1 Debug Flags
debug_dflags="-DENABLE_ASSERT=1 -DENABLE_DEBUG_MESSAGES=1"
debug_wflags="-Wno-unused-variable -Wno-unused-function"
debug_flags="-std=c99 -g -fsanitize=address -Wall ${debug_wflags} ${debug_dflags}"

# 2.2 Release Flags
release_dflags=""
release_wflags="-Wno-unused-variable -Wno-unused-function"
release_flags="-std=c99 -O0 -Wall ${release_wflags} ${release_dflags}"

# 3. Linker
libs="-lm"

# 4. Compiler 
compiler="gcc"

# Command
if [ "$1" = "release" ]; then
  echo compiling release build...
  ${compiler} ${release_flags} -o ${output} ${input} ${libs}
  echo finished compiling...
else
  echo compiling debug build...
  ${compiler} ${debug_flags} -o ${output} ${input} ${libs}
  echo finished compiling
fi
