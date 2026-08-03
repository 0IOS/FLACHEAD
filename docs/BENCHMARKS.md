# FLACHEAD Benchmark Report

Targets (Raspberry Pi Zero W):

| Metric | Target |
|---|---|
| Stable FPS | 30 min / 45 aim |
| Startup to home screen | < 10 s |
| RAM | < 120 MB |
| Idle CPU | < 5% |
| Playback CPU | < 20% |

## How to measure

```sh
# FPS / frame time (renders continuously for N seconds, prints report)
./build/FLACHEAD --benchmark=5

# Startup time (logged automatically on every launch)
./build/FLACHEAD

# Idle CPU + RAM while sitting on the launcher
ps -o pid,pcpu,rss -p $(pgrep FLACHEAD)

# Peak RAM
/usr/bin/time -v ./build/FLACHEAD --benchmark=5

# Binary size
ls -l build/FLACHEAD

# Per-second CPU during playback (on the Pi: pidstat -p <pid> 1)
pidstat -p $(pgrep FLACHEAD) 1
```

## Latest run (dev machine, X11 software renderer)

Updated 2026-08-03, commit `553758a` + performance pass.

## Release build (production verification run)

| Metric | Result |
|---|---|
| Startup time | 78 ms |
| Average FPS (launcher, forced render) | 60.0 |
| Min FPS per second | 61 (0 dropped vs 60 cap) |
| Avg frame time | 16.4 ms |
| Worst frame | 17.3 ms |
| Binary size | 3.4 MB (Release, -O2) |

## Debug build (original performance pass)

| Metric | Result |
|---|---|
| Startup time | 84 ms |
| Average FPS (launcher, forced render) | 60.0 |
| Min FPS per second | 61 (0 dropped vs 60 cap) |
| Avg frame time | 16.4 ms |
| Worst frame | 16.7 ms |
| Idle CPU (launcher, no input) | ~4% of one core |
| Peak RAM | 107 MB |
| Binary size | ~4.3 MB (Debug) |

Notes:
- The app is FPS-capped at 60; "min FPS 61" reflects the first second
  (window settle) and the cap math — no frame exceeded the 16.6 ms budget
  after warm-up.
- Worst frame is 16.7 ms: no frame hit the 16.6 ms budget exactly; every
  frame stayed within one `SDL_Delay` tick of the budget.
- Before the optimization pass, every text draw re-rasterized via TTF and
  re-uploaded a texture, rounded rects/circles ran per-pixel `sqrt` loops
  every frame, and the loop rendered at ~1000 FPS with `SDL_Delay(1)`.

## TODO (next runs)

- Repeat measurements on the actual Raspberry Pi Zero W (software renderer,
  1 GHz single core) and fill in Pi-specific rows.
- Measure playback CPU once the audio backend decodes FLAC.
- Measure per-screen FPS: `--benchmark` currently forces the launcher.

## Change log

| Date | Change | Avg FPS | Min FPS | Frame ms | Idle CPU | RAM |
|---|---|---|---|---|---|---|
| 2026-08-03 | Text/font caches, shape masks, 60 FPS cap, idle event wait | 60.0 | 61 | 16.4 | ~4% | 107 MB |
