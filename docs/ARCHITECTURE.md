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
- `m_BackgroundJobs` — `services::BackgroundJobManager` (single worker thread,
  priority-ordered jobs, main-thread completion via `Update()`).
- `m_Notifications` — `services::NotificationManager` (bounded history, drains
  to overlay toasts through an injected hook).
- `m_Memory` — `services::MemoryManager` (sampled current/peak RSS, optional
  soft/hard budgets with edge-triggered crossing callbacks).
- `m_Transitions` — `shell::ScreenTransitionManager` (fade overlay on every
  push/pop).
- `m_ScreenManager` — stack of screens, populated from factories.
- `m_AppContext` — bundle of service pointers handed to every DAP screen.
- `m_ShellServices` — bundle handed to every shell screen (renderer, themes,
  overlays, wallpaper, animations, focus, screens, services, quit hook).

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
   exists, `StartScan` runs in the background. `LibraryScanFinished` posts a
   "N tracks" notification.
7. Start `BackgroundJobManager`, initialize `MemoryManager`, register the
   screen factories, and push the `home` screen.

### Frame loop (`Application::Run`)

- `Time::Update()` → animators → `AppManager.Tick` →
  `AudioService.PollBackendEvents()` → `Playback.Update(delta)` →
  `ScreenManager.Update(delta)` → `BackgroundJobs.Update()` →
  `Notifications.Update()` → `Memory.Update()` → `Transitions.Update(delta)`.
  Polling the backend and advancing playback is what lets transport events
  reach the screens; without it the UI never learns about track changes or
  end-of-queue.
- Input is polled through `input::InputBackend` (SDL events or synthesized
  GPIO key events). Raw SDL events go to `InputManager.HandleEvent`, which
  feeds the gesture recognizer, maps keys to `Command`s, and implements the
  center/home-button semantics (tap → launcher, double-tap → home, hold →
  task overview). A single home tap emits a command-carrying `InputEvent`
  (launcher) inside the double-tap window so the shell can show the
  "double-tap for Home" hint. `SDL_EVENT_QUIT` and `SDL_EVENT_TERMINATING`
  set the exit flag.
- Rendering happens only when something changed (input handled, resize, minute
  tick, screen `NeedsRender`, transition active, or benchmark mode). Otherwise
  the loop blocks in `SDL_WaitEventTimeout(50 ms)` — static screens burn ~0%
  CPU. The GPIO input backend polls every ~20 ms instead, so button latency
  stays under one frame.
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
  during a scan. `graphics::ImageCodec` decodes PNG (libpng), JPEG (libjpeg)
  and BMP (SDL) into textures; the ambient home screen draws the cover and
  mirrors it into the wallpaper with a crossfade.

## Operating environment (`shell::`)

The shell is the appliance layer on top of the legacy DAP screens. All shell
screens derive from `ShellScreen`, which hosts a widget tree, a per-screen
focus scope, the wallpaper and overlay layers, and routes commands. Rendering
order is wallpaper → widget tree → overlays. Shell screens are the only ones
that consume the semantic input pipeline (gestures, d-pad focus commands,
home-button commands); legacy DAP screens keep their raw-SDL path.

Every screen renders inside the **persistent three-region chrome**
(`ShellChrome`): a top status strip (title, clock, battery), the primary
content region, and a bottom navigation bar (Back / Apps / Home / Search /
Queue) that dispatches through the command center so taps get the same
fullscreen fade as d-pad navigation. Geometry is derived from the **240×320
portrait reference** (`ShellChrome::kReferenceWidth`) and scales up on larger
windows; `ui::Canvas` applies the same scale to typography via `SetScale`.

- `ShellScreen` — base for the operating-environment screens. It owns the
  chrome, stores the last view size for nav-bar hit-testing, and lays its
  widget tree out in `ShellChrome::ContentArea()` only. `IsChromeScreen()`
  marks it so the application's Back handling applies.
- `LegacyChromeScreen` — wraps a legacy DAP screen so it inherits the
  persistent bottom nav bar too. The inner screen keeps its own
  status/header/footer as the content region (rendered at reduced height) and
  forwards lifecycle/raw events; nav-bar taps are intercepted in
  `OnInputEvent`.

- `AmbientHomeScreen` (`home`) — full-player home: cover art (also the
  wallpaper), transport, favorites/shuffle/repeat, app shortcuts. Renders only
  when playback state actually changes.
- `LauncherScreen` (`launcher`) — two-column app grid that opens the DAP
  screens by name and stays beneath them.
- `TaskOverviewScreen` (`taskoverview`) — reversed stack cards with pop-to
  navigation, plus Home/Search/Settings.
- `UniversalSearchScreen` (`universal_search`) — `TextField` + on-screen
  keyboard, live top-6 results that play on tap.
- `SettingsScreen` (`settings`) — volume slider, shuffle/repeat defaults,
  accent-color tiles + dark/light toggle (persisted as `theme.accent` /
  `theme.dark`), library rescan, About. The legacy `dapsettings` factory
  remains registered.
- `SystemScreen` (`system`) — live backend/track/scan-root/memory info (labels
  refresh on a 0.5 s throttle) plus Restart/Shut-down through the quit hook.

Shell services (`ShellServices`) bundle `AppContext`, the renderer, theme,
overlays, wallpaper, animations, focus, screens, the AO-4 services and a quit
hook, so shell screens never touch `Application` directly. Screen pushes and
pops run a fullscreen fade (`ScreenTransitionManager`); because the fade needs
continuous frames, the render loop's needs-render test includes it.

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

- `HomeScreen` (legacy `home`) — replaced on the stack by the shell's
  `AmbientHomeScreen`, which owns the `home` key now; Esc still quits.
- `PlaybackScreens.cpp` — `nowplaying` (seek, volume, favorite, shuffle,
  repeat, prev/next, queue), `queue`, `scan` (progress + start/cancel).
- `LibraryScreens.cpp` — `songs`, `albums` (grid), `album` (play album +
  tracks), `artists`, `artist`, `search`, `favorites`, `recent` (recently
  added via `LibraryService::RecentlyAdded`).
- `PlaylistScreens.cpp` — `playlists` (create/delete), `playlist` (play
  playlist, remove track).
- `SettingsScreen.cpp` (legacy `dapsettings`) — kept for its advanced options;
  the shell `settings` screen covers the common ones.

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
- `InputManager` turns raw events into semantic `InputEvent`s (tap,
  double-tap, hold, swipe, drag, key) and `Command`s. Touch coordinates arrive
  normalized and are scaled by `SetWindowSize`. Its center/home-button
  semantics: single tap → launcher (signalled early so the shell can hint),
  double-tap → home, hold → task overview. The double-tap check requires a
  prior valid release so the first-ever tap at tick 0 cannot be misread.
- Gesture state is driven per-frame by `InputManager.Update()` (holds and the
  single-tap disambiguation timer) and `GestureRecognizer.Update()`.

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
- `BackgroundJobManager` runs one worker thread; job `work` bodies may touch
  anything the scanner does, but their `complete` callbacks are drained by
  `Update()` on the main thread so they can touch UI services.
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
