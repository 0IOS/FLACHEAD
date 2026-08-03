# CMake toolchain for Raspberry Pi Zero W (Raspberry Pi OS Lite, armhf).
#
# Pi Zero W: Broadcom BCM2835, ARM1176JZF-S core (ARMv6, VFP, no NEON).
# Sysroot not required: Raspberry Pi OS ships the same glibc/multilib ABI as
# Debian armhf, so the compiler must be the armhf GNU triplet below.
#
# Install a cross toolchain, e.g. on Fedora:
#   sudo dnf install arm-linux-gnueabihf-gcc-c++
# On Debian/Ubuntu:
#   sudo apt install g++-arm-linux-gnueabihf
#
# Build:
#   cmake -S . -B build-pi -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/pi-zero-w.cmake \
#         -DCMAKE_BUILD_TYPE=Release
#   cmake --build build-pi -j
#
# SDL3 + SDL3_ttf built for armhf must be findable on the host; point CMake at
# a sysroot with CMAKE_FIND_ROOT_PATH if you keep cross libraries elsewhere.
# The configure only succeeds when SDL is found, so the toolchain must be
# exercised on a host that has the armhf libraries (or a sysroot).

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ARM11 / Pi Zero W tuning. -march=armv6zk enables the ARM1176JZF-S ISA;
# VFP is always present on the Pi, hard float is the OS default ABI.
set(CMAKE_C_FLAGS "-march=armv6zk -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS "-march=armv6zk -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard")
