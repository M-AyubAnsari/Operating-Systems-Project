#!/bin/bash
# ====================================================================
#  Build Script for Linux / Android NDK (Target and Instrumentation)
#  Author: Muhammad Ayub Ansari
# ====================================================================

set -e

echo "[BUILD] Checking target compilation environment..."

# Detect if Android NDK is configured
if [ -n "$ANDROID_NDK_HOME" ] || [ -n "$NDK" ]; then
    NDK_PATH="${ANDROID_NDK_HOME:-$NDK}"
    echo "[BUILD] Android NDK detected at: $NDK_PATH"
    
    # Target Android API 34 x86_64 emulator or arm64 device
    TOOLCHAIN="$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/bin"
    if [ ! -d "$TOOLCHAIN" ]; then
        TOOLCHAIN="$NDK_PATH/toolchains/llvm/prebuilt/windows-x86_64/bin"
    fi
    
    TARGET_CC="$TOOLCHAIN/x86_64-linux-android34-clang"
    if [ -f "$TARGET_CC" ]; then
        echo "[BUILD] Cross-compiling for Android x86_64 Emulator..."
        $TARGET_CC -O2 -Wall -I../src target_app.c -lsqlite3 -llog -o ../target_app_android
        $TARGET_CC -O2 -Wall -I../src exit_controller.c -o ../exit_controller_android
        $TARGET_CC -O2 -Wall -I../src experiment_runner.c -o ../experiment_runner_android
        echo "[BUILD] Android binaries ready: target_app_android, exit_controller_android"
        exit 0
    fi
fi

# Native Linux compilation fallback
echo "[BUILD] Compiling natively with gcc/clang..."
CC="${CC:-gcc}"
$CC -O2 -Wall -I../src target_app.c -lsqlite3 -o ../target_app
$CC -O2 -Wall -I../src exit_controller.c -o ../exit_controller
$CC -O2 -Wall -I../src experiment_runner.c -o ../experiment_runner

echo "[BUILD] Compilation complete!"
