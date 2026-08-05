# Raspberry Pi Zero W runbook

FLACHEAD is built and developed on the desktop, but **the Pi is the reference
platform**. Desktop numbers are never final. Every optimization must be
re-measured on a Pi Zero W and recorded in `BENCHMARKS.md`.

## One-shot audit

`tools/pi-audit.sh` produces a timestamped report under `reports/`:

```sh
./tools/pi-audit.sh ./build/FLACHEAD 15
```

It measures, in one run:

1. binary size
2. startup time (logged by the app)
3. FPS / frame time / worst frame (`--benchmark`)
4. peak RSS (`/usr/bin/time -v`)
5. idle CPU + RSS while sitting on the home screen (`pidstat`)

## Report mode (in the binary)

`--benchmark[=seconds]` now also prints a renderer report at exit:

```text
Textures        : 41 (228.2 KB)
Peak RSS        : 110404 KB
```

Texture count + estimated texture memory come from the renderer caches
(text glyphs + shape masks); peak RSS comes from `/proc/self/status`
(`VmHWM`) on Linux.

## Allocation profiling (requires valgrind on the Pi)

```sh
sudo apt install valgrind
valgrind --tool=massif --pages-as-heap=no ./build/FLACHEAD --benchmark=10
ms_print massif.out.* | less
```

Look for: allocation churn per frame (a growing heap during the static
home-screen render means a per-frame leak), and the peak heap byte count.

## CPU profiling (requires `perf`, needs `perf_event_paranoid=1`)

```sh
perf record -g ./build/FLACHEAD --benchmark=10
perf report
```

The benchmark forces the 60 FPS tier and continuous rendering, so a
`perf record` captures a representative render workload (not the idle path).

## Texture memory

Already reported by `--benchmark` ("Textures ... KB"). The text cache is
capped at 256 glyphs (~2 MB worst case); shape masks at 32 radii each
(~200 KB worst case). If the Pi exceeds the RAM budget, shrink
`kTextCacheCap` / `kShapeCacheCap` in `src/core/Renderer.cpp`.

## Startup profiling

Startup time is logged on every launch. To break down the 60 ms (desktop)
into per-phase numbers on the Pi, run:

```sh
SDL_VIDEODRIVER=kmsdrm /usr/bin/time -v ./build/FLACHEAD --benchmark=5 2>&1 | \
    grep -E "Startup time|Maximum resident|Elapsed"
```

## Adaptive tier on the Pi

The frame scheduler degrades 60 → 45 → 30 FPS based on measured render time
(see `docs/ARCHITECTURE.md`). On the Pi, note which tier each screen settles
on and record it in `BENCHMARKS.md`:

```sh
./build/FLACHEAD   # watch the journal for "Adaptive frame tier: NN FPS"
```

If a screen settles below 30 FPS, that screen must reduce animation quality
— it must not drop input or stall audio.

## Strict build (desktop gate, runs on every change)

```sh
cmake -S . -B build-strict -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic"
cmake --build build-strict
```

Warnings are treated as failures during Phase 2.
