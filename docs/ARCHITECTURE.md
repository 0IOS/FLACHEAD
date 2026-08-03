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
- Rendering is capped at 60 FPS (`SDL_Delay` to the current tier's frame
  budget).
- **Adaptive frame tier.** The loop measures pure render time (exclusive of the
  cap delay) and feeds a 0.1-alpha EMA. If the EMA exceeds the current tier's
  budget by 10% for 30 consecutive frames, the target steps down 60 → 45 → 30
  FPS; if it stays under the next tier's budget by 25% for 30 frames, it steps
  back up. The 30 FPS floor matches the hard minimum requirement. The tier only
  changes while actively rendering — the idle path is unaffected.
  `--benchmark` forces the 60 FPS tier so runs stay comparable.
- A clock changes once a minute; the status bar redraw is triggered by minute
  change, not by per-frame polling.
- `--benchmark[=seconds]` forces continuous rendering at the 60 FPS tier and
  prints a report (avg/min/max FPS, frame times) at exit.

## Memory

- Text cache: 256 glyphs max (~2 MB worst case).
- Shape masks: 4 caches x 32 radii, radii capped at 96 (~200 KB worst case).
- Screens are created lazily on first push (`ScreenManager` factories);
  boot only allocates the launcher.

## Storage

- `services::StorageManager` stages writes in memory; `Flush()` batches them
  to the SD card. Never write per key press / per frame.

## Threading

- Everything runs on the main thread today; the audio backend (when attached)
  runs on its own threads and must never be blocked by the UI.
- Heavy work (library scans, album-art decode) must move to worker threads
  before it is added; never scan storage during playback.

## Input (`input::`)

- `input::InputBackend` is the only source of `SDL_Event`s. Screens implement
  `HandleEvent(const SDL_Event&)` and never know where input came from.
- `SdlInputBackend` (default) pumps the SDL event queue. `GpioInputBackend`
  (Pi, `--input=gpio`) reads `/sys/class/gpio` buttons and synthesizes SDL
  key events; it requires no extra dependencies and falls back to SDL input
  with a warning when GPIO is unavailable.
- **Input responsiveness is a hard constraint.** The GPIO idle path polls every
  ~20 ms instead of the 50 ms event wait, so button latency stays under one
  frame even when nothing needs redrawing.

## Display (`system::`)

- `system::DisplayBackend` is the final stage that receives rendered frames:
  `Create/Destroy/Present/Name`. `SdlDisplayBackend` (default) presents to the
  window. A future SPI/DRM backend for the Pi panel replaces it via
  `Renderer::SetDisplayBackend`; it must never block in `Present()`.
- Renderer caches and draw calls are independent of the display backend.

## Audio backend (`audio::`)

- `audio::AudioService` owns an `audio::AudioBackend` and forwards all media
  operations. The UI must only ever talk to `AudioService`.
- The production backend is **libmpv** (FLAC decode, gapless playback, video
  sync, subtitles). `audio::NullBackend` satisfies the interface today so the
  app builds and runs without an audio library; the seam is:
  `OpenMedia/Play/Pause/Stop/Seek/SetVolume/GetPositionSeconds/IsPlaying`.
- Backend contract: methods return in milliseconds, decoding/IO happens on the
  backend's own threads, volume is linear 0.0–1.0, and the render loop must
  never block on a backend call. A `libmpv` backend must not be added until
  this contract is met.

## Shutdown

- `SDL_EVENT_QUIT` (window close) and `SDL_EVENT_TERMINATING` (SIGTERM, as
  sent by systemd) both stop the main loop, run `Shutdown()` and flush logs.
  The app never exits by dying to a signal.

## Boot

- Target: < 10 s to home screen. Startup allocations are limited to SDL/TTF
  init, window, renderer, theme, and the launcher screen. Startup time is
  logged automatically.
