# FLACHEAD Architecture

Target hardware: **Raspberry Pi Zero W** (single-core ARM11 @ 1 GHz, 512 MB RAM).

Performance is a first-class requirement. See `BENCHMARKS.md` for the measured
numbers. The rules below are non-negotiable:

1. **Audio playback always has priority.** Never block the audio thread; the
   render loop must never stall audio.
2. **30 FPS minimum, 45 FPS target.** If a screen cannot hold 30 FPS, it must
   degrade animation quality, not drop frames.
3. **Idle CPU under 5%, playback CPU under 20%.**
4. **RAM under 120 MB.**

## Rendering pipeline

- `core::Renderer` owns the SDL renderer and all GPU-side caches.
- **Text is cached.** `DrawText` rasterizes via TTF only on cache miss; the
  glyph texture is keyed by a 64-bit hash of (font object, color, string).
  Cache is capped at 256 entries with FIFO eviction. Never call TTF inside the
  hot path on a hit.
- **Fonts are cached by path + size** in `graphics::FontManager` (transparent
  `std::map` lookup, no per-call string allocation). `ui::Canvas::DrawText`
  goes through it — never create a `Font` per draw call.
- **Shapes are cached as tinted mask textures.** Rounded-rect corners, discs
  and rings are pre-rendered to RGBA masks once per radius (capped at 32 radii
  each) and drawn with color/alpha modulation. A rounded rect costs 3 fill
  rects + 4 textured corners instead of O(radius) per-pixel loops with `sqrt`.
  Opaque fills use `SDL_BLENDMODE_NONE`; only translucent fills blend.
- **No per-frame heap allocation in the render path.** Time labels use
  `snprintf` into stack buffers; strings that repeat every frame are cached in
  the owning screen and only rebuilt when the displayed value changes.

## Frame loop (`app::Application::Run`)

- Event-driven with an idle sleep: when no events arrived, the active screen
  does not need a redraw, the minute has not changed and the window was not
  resized, the loop blocks in `SDL_WaitEventTimeout(50 ms)` instead of
  rendering. Static screens therefore burn ~0% CPU while idle.
- `Screen::NeedsRender()` (default `false`) reports whether the screen has
  active animation:
  - launcher: only during the enter transition
  - music/video: only while "playing"
  - gallery: only during fullscreen transitions
  - notes: only while the cursor is visible (3 Hz blink)
  - power: only while a countdown runs
- Rendering is capped at 60 FPS (`SDL_Delay` to a 16.6 ms budget).
- A clock changes once a minute; the status bar redraw is triggered by minute
  change, not by per-frame polling.
- `--benchmark[=seconds]` forces continuous rendering and prints a report
  (avg/min/max FPS, frame times) at exit.

## Memory

- Text cache: 256 glyphs max (~2 MB worst case).
- Shape masks: 4 caches x 32 radii, radii capped at 96 (~200 KB worst case).
- Screens are created lazily on first push (`ScreenManager` factories);
  boot only allocates the launcher.

## Storage

- `services::StorageManager` stages writes in memory; `Flush()` batches them
  to the SD card. Never write per key press / per frame.

## Threading

- Everything runs on the main thread today; SDL audio (when a backend is
  attached) runs on SDL's own thread and must never be blocked by the UI.
- Heavy work (library scans, album-art decode) must move to worker threads
  before it is added; never scan storage during playback.

## Boot

- Target: < 10 s to home screen. Startup allocations are limited to SDL/TTF
  init, window, renderer, theme, and the launcher screen. Startup time is
  logged automatically.
