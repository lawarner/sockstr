#!/bin/bash

ToolChainDir=$HOME/toolchains/android-toolchain

function die {
    echo "ERROR: $1"
    exit 2
}

test -d "${NDK_ROOT}" || die "Set NDK_ROOT to location of the NDK."

cd "$NDK_ROOT/build/tools"
./make-standalone-toolchain.sh --arch=arm --ndk-dir=${NDK_ROOT} --install-dir=${ToolChainDir} --platform=android-21 --system=linux-x86_64
# --toolchain=arm-linux-androideabi-4.8
