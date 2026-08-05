# FLACHEAD Architecture

Target hardware: **Raspberry Pi Zero W** (single-core ARM11 @ 1 GHz, 512 MB RAM).

FLACHEAD is a FLAC music player (a "digital audio player" UI) that renders a
full music interface with SDL3, decodes FLAC through libmpv, and keeps its
library in SQLite. Performance is a first-class requirement. See
`BENCHMARKS.md` for the measured numbers. The rules below are non-negotiable:

1. **Audio playback always has priority.** Never block the audio thread; the
   render loop must never stall audio.
2. **30 FPS minimum, 45 FPS target.** If a screen cannot hold 30 FPS, it must
   degrade animation quality, not drop frames.
3. **Idle CPU under 5%, playback CPU under 20%.**
4. **RAM under 120 MB.**

## Runtime wiring (`app::Application`)

`Application` owns the services and the frame loop. Construction order matters:
`m_EventBus` is declared **first**, so it is destroyed **last** — screens
subscribe to it during their lifetimes, and teardown must never touch a freed
EventBus.

- `m_Database` — SQLite database (`~/.flachead/flachead.db`).
- `m_EventBus` — sole communication channel between subsystems.
- `m_AudioService` — owns the audio backend (libmpv, or null in tests).
- `m_LibraryService` — scan + query facade over the songs table.
- `m_Playback` — playback state machine + queue.
- `m_Playlists` — playlist CRUD.
- `m_SettingsManager` — typed key/value settings backed by the `settings` table.
- `m_ScreenManager` — stack of screens, populated from factories.
- `m_AppContext` — bundle of service pointers handed to every DAP screen.

### SetupServices (boot)

1. Create `~/.flachead`, open the DB, run migrations.
2. `SettingsManager.Initialize(db, bus)` — loads persisted settings.
3. `Playback.Initialize()`.
4. Install hooks: volume changes persist through settings
   (`audio.volume`), each started track is counted as played
   (`LibraryService::MarkPlayed`).
5. **Session restore**: volume, repeat mode and shuffle are restored from
   settings (`audio.volume`, `playback.repeat`, `playback.shuffle`).
6. Scan root from `library.scan_roots`, defaulting to `$HOME/Music`; if one
   exists, `StartScan` runs in the background.
7. Register the screen factories and push the `home` screen.

### Frame loop (`Application::Run`)

- `Time::Update()` → animators → `AppManager.Tick` →
  `AudioService.PollBackendEvents()` → `Playback.Update(delta)` →
  `ScreenManager.Update(delta)`.
  Polling the backend and advancing playback is what lets transport events
  reach the screens; without it the UI never learns about track changes or
  end-of-queue.
- Input is polled through `input::InputBackend` (SDL events or synthesized
  GPIO key events). `SDL_EVENT_QUIT` and `SDL_EVENT_TERMINATING` set the exit
  flag.
- Rendering happens only when something changed (input handled, resize, minute
  tick, screen `NeedsRender`, or benchmark mode). Otherwise the loop blocks in
  `SDL_WaitEventTimeout(50 ms)` — static screens burn ~0% CPU. The GPIO input
  backend polls every ~20 ms instead, so button latency stays under one frame.
- Frame scheduling is capped at 60 FPS and adaptive: a 0.1-alpha EMA of pure
  render time steps the target 60 → 45 → 30 FPS when it exceeds a tier budget
  (and back up when it stays comfortably under). `--benchmark` forces the 60
  FPS tier so runs are comparable, and prints a report at exit.

### Shutdown

`Shutdown()` runs on quit and after SIGTERM (systemd). Order: stop playback,
shut down audio, **wait for the scanner thread** (`LibraryService.WaitForScan`),
shut down app manager / resources / renderer / fonts / window, close the DB,
quit TTF. The app never exits by dying to a signal.

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
- Album art: `library::AlbumArtExtractor` finds and caches cover images
  (`cover.jpg`, `folder.png`, embedded tags) under `~/.flachead/cache/art`
  during a scan. The current DAP screens render album art as colored
  placeholder blocks keyed by album name; decoding the cached covers into
  textures is the next step.

## DAP screens (`dap::`)

Every screen derives from `DapScreen`, which carries the `AppContext` service
bundle and provides the shared chrome: status bar, header, footer hints, list
rows, empty state and a now-playing bar.

- `DapScreen::Subscribe(type[, handler])` registers a bus subscription for the
  screen's lifetime; arrivals mark the screen dirty → `RefreshData()` + render.
- `RefreshData()` reloads the screen's cached list from the library / playlists
  on enter and after subscribed events.
- Screens navigate with `Ctx().navigate("screenname")` and pass parameters
  through `AppContext::pending` (artist, album, playlist id), read in the
  target's `RefreshData`.

### Screens

- `HomeScreen` (`home`) — now-playing hero + navigation; Esc quits the app.
- `PlaybackScreens.cpp` — `nowplaying` (seek, volume, favorite, shuffle,
  repeat, prev/next, queue), `queue`, `scan` (progress + start/cancel).
- `LibraryScreens.cpp` — `songs`, `albums` (grid), `album` (play album +
  tracks), `artists`, `artist`, `search`, `favorites`, `recent` (recently
  added via `LibraryService::RecentlyAdded`).
- `PlaylistScreens.cpp` — `playlists` (create/delete), `playlist` (play
  playlist, remove track).
- `SettingsScreen.cpp` (`dapsettings`) — volume (live + persisted), scan
  folder editing, default shuffle/repeat toggles, rescan, About with the audio
  backend name.

## Event bus (`events::EventBus`)

Thread-safe pub/sub. Events are dispatched synchronously on the publishing
thread. `Type` covers playback (started/paused/resumed/stopped/finished,
track changed/loaded/progress, error), queue/shuffle/repeat changes, library
scan lifecycle, playlist changes, settings changes, and theme/display events.

Data flow: audio backend → `AudioService` → bus → `PlaybackController` →
bus → screens. The UI never talks to playback/library internals directly.

## Library (`library::`)

- `LibraryScanner` walks the scan roots on a **worker thread**, extracts
  metadata with ffprobe (and taglib when compiled in), extracts embedded album
  art to the art cache, and upserts `songs` rows. It publishes
  `LibraryScanProgress` (`intValue` processed, `doubleValue` total) and
  `LibraryScanFinished`. `WaitForScan()` joins the thread at shutdown.
- `LibraryService` is the read/write facade: search, album/artist grouping
  (computed with `GROUP BY` from `songs`), favorites, play counting, and
  `RecentlyAdded`.
- Never scan storage during playback; the scan runs on its own thread and
  writes are batched.

## Playback (`playback::`)

- `QueueManager` — the track queue with repeat (off/all/one) and shuffle.
- `PlaylistEngine` — playlist CRUD over `playlists` / `playlist_tracks`.
- `PlaybackController` — the state machine + UI-facing API:

  ```
  Stopped -> Loading -> Playing <-> Paused
  Playing -> Seeking -> Playing
  Playing -> Finished (end of queue)
  Playing/Loading -> Error
  ```

  Volume goes through a store hook (persisted to settings); started tracks go
  through a play hook (play counting). Progress events are throttled to 1 Hz
  so subscribers get periodic updates without bus spam.

## Audio backend (`audio::`)

- `audio::AudioService` owns an `audio::AudioBackend` and forwards all media
  operations. The UI must only ever talk to `AudioService` / `PlaybackController`.
- The production backend is **libmpv** (`MpvBackend`), loaded with `dlopen`
  against the mpv C API: FLAC decode, gapless playback. `NullBackend`
  simulates playback (240 s virtual tracks, seek, pause) so the app builds and
  runs without an audio library and the test suite is deterministic.
- Backend contract: methods return in milliseconds, decoding/IO happens on the
  backend's own threads, volume is linear 0.0–1.0, and the render loop never
  blocks on a backend call. The backend publishes transport events
  (track changed, end of file, error) that `PollBackendEvents` drains.

## Input (`input::`)

- `input::InputBackend` is the only source of `SDL_Event`s. Screens implement
  `HandleEvent(const SDL_Event&)` and never know where input came from.
- `SdlInputBackend` (default) pumps the SDL event queue. `GpioInputBackend`
  (Pi, `--input=gpio`) reads `/sys/class/gpio` buttons and synthesizes SDL key
  events; it requires no extra dependencies and falls back to SDL input with a
  warning when GPIO is unavailable.

## Display (`system::`)

- `system::DisplayBackend` is the final stage that receives rendered frames:
  `Create/Destroy/Present/Name`. `SdlDisplayBackend` (default) presents to the
  window. A future SPI/DRM backend for the Pi panel replaces it via
  `Renderer::SetDisplayBackend`; it must never block in `Present()`.
- Renderer caches and draw calls are independent of the display backend.

## Storage

- SQLite is the single store (library, playlists, settings, history). Schema
  is versioned via `PRAGMA user_version` and migrated on open
  (`Database::Migrate`).
- `services::StorageManager` stages writes in memory for SD-card-friendly
  flushing; settings and library writes already batch at the SQL level.

## Threading

- UI, screens, playback state machine and DB access run on the main thread.
- The library scanner runs on a worker thread and must be joined at shutdown
  before the DB closes.
- The audio backend (libmpv) runs on its own threads; the UI never blocks on a
  backend call.
- The EventBus is mutex-protected and safe to publish/subscribe from any
  thread.

## Boot

- Target: < 10 s to home screen. Startup allocations are limited to SDL/TTF
  init, window, renderer, theme, the home screen, and the DB + service wiring.
  Startup time is logged automatically on every launch.
- As a systemd service on the framebuffer console the app boots straight into
  the music UI with no desktop; see `docs/BOOT_INTEGRATION.md`.
