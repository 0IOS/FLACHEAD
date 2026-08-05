# Cross-compiling FLACHEAD for Raspberry Pi Zero W

The Pi Zero W is an ARM11 (ARMv6, VFP, no NEON), running Raspberry Pi OS
Lite (armhf, hard float). Everything must be tuned for that core.

## Toolchain

The CMake toolchain file `cmake/toolchains/pi-zero-w.cmake` sets:

- `arm-linux-gnueabihf` compiler triplet
- `-march=armv6zk -mtune=arm1176jzf-s` (ARM1176JZF-S core)
- `-mfpu=vfp -mfloat-abi=hard` (Pi always has VFP; hard float matches the OS)

Install the cross compiler:

```sh
# Fedora
sudo dnf install arm-linux-gnueabihf-gcc-c++

# Debian / Ubuntu / Raspberry Pi OS (on a desktop host)
sudo apt install g++-arm-linux-gnueabihf
```

## SDL3 for armhf

`find_package(SDL3 CONFIG)` and `SDL3_ttf` must succeed against armhf
libraries. Options:

1. Cross-compile SDL3 + SDL3_ttf yourself and install to a sysroot:
   ```sh
   export SYSROOT=$HOME/rpi-sysroot
   # build SDL3 and SDL3_ttf for armhf, then:
   #   cmake --install ... --prefix $SYSROOT
   ```
2. Or copy the armhf SDL packages from the Pi onto the host and point CMake
   at them with `CMAKE_FIND_ROOT_PATH` (see the toolchain file).

The app also links **sqlite3** (`libsqlite3-dev` for armhf) — include it in
the sysroot or on the Pi. If the tests are cross-compiled
(`-DFLACHEAD_BUILD_TESTS=ON`), the armhf sqlite3 and the test binaries are
needed on the Pi too.

## Build

```sh
cmake -S . -B build-pi \
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/pi-zero-w.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_FIND_ROOT_PATH="$SYSROOT"
cmake --build build-pi -j
```

Size optimization (`-ffunction-sections -fdata-sections -Wl,--gc-sections
-s`) is on by default in Release builds; disable with
`-DFLACHEAD_SIZE_OPT=OFF`.

## Deploy

```sh
scp build-pi/FLACHEAD pi@<ip>:/opt/flachead/FLACHEAD
```

Run `tools/pi-audit.sh` on the Pi to produce the resource report
(see `docs/PI_RUNBOOK.md`).

## Cross checks available without a Pi

```sh
# Make sure the code compiles for the ARM core you will ship to
arm-linux-gnueabihf-g++ -std=c++20 -march=armv6zk -mtune=arm1176jzf-s \
    -mfpu=vfp -mfloat-abi=hard -fsyntax-only src/main.cpp
```

The toolchain file cannot be exercised until armhf SDL3 is present on the
host. Until then, every change is also validated with the strict warning
flags described in `docs/PI_RUNBOOK.md`.
