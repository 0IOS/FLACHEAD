# FLACHEAD

A headless-friendly **FLAC music player** built for the **Raspberry Pi Zero W**
(single-core ARM11 @ 1 GHz, 512 MB RAM). It renders a full music UI to the
screen, streams FLAC through libmpv, keeps its library in SQLite, and runs as a
systemd service on Raspberry Pi OS Lite — no desktop required.

Development happens on a desktop machine; the Pi is the reference platform.
Performance is a first-class requirement (30 FPS minimum, < 120 MB RAM,
idle CPU < 5%). See `docs/BENCHMARKS.md`.

## Status

All core milestones are implemented and committed. The app builds, all test
suites pass, the library scan indexes a real music folder, and the DAP screens
play back tracks through the audio stack.

| Milestone | Commit | What landed |
|---|---|---|
| M1 core | `1238e4b` | Event bus, path utilities, time formatting, song model |
| M2 database | `6a35e7e` | SQLite wrapper, schema v1, migrations, transactions, tests |
| M3 metadata | `87309d1` | JSON parser, ffprobe extractor, taglib extractor (guarded), tests |
| M4 audio | `ec8db58` | libmpv backend via dlopen, full backend interface, AudioService + event bus, null simulator, tests |
| M5 playback | `e7e93fc` | Queue manager, playlist engine, playback controller state machine, tests |
| M6 library | `a4a316c` | Background scanner, library service, album art cache, tests |
| M7 settings | `ab17af7` | Persistent settings manager backed by the `settings` table, tests |
| M8 DAP | `f8c54a2` | Music player screens + application wiring |
| M9 runtime | — | Boot into the music UI, restore volume / repeat / shuffle / scan root |

M10 (this documentation) is the current milestone.

## Quick start

### Dependencies

- SDL3 and SDL3_ttf (`find_package(SDL3 CONFIG)` / `SDL3_ttf`), plus optional
  SDL3_image
- sqlite3
- C++20 compiler
- `ffprobe` on `PATH` for metadata extraction during scans

### Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Build with tests

```sh
cmake -S . -B build -DFLACHEAD_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

There are 7 test suites covering the database, metadata extraction, library
scanning, settings, audio, playback, and the full application wiring
(`tests/`).

### Run

```sh
./build/FLACHEAD                          # normal UI (SDL window on desktop)
./build/FLACHEAD --benchmark=5            # render 5 s at the 60 FPS tier, print a report
./build/FLACHEAD --input=gpio             # GPIO buttons on the Pi (falls back to SDL)
```

On first launch the app creates `~/.flachead/flachead.db`, migrates the schema,
and scans the music folder. The scan root defaults to `$HOME/Music` and can be
changed in the on-screen settings (`Settings` → Scan folder).

See `docs/PI_RUNBOOK.md` for profiling, `docs/BOOT_INTEGRATION.md` for running
as a service, and `docs/CROSS_COMPILE.md` for the armhf toolchain.

## Controls

The UI is designed for a keyboard or a handful of GPIO buttons. Up/Down moves,
Enter selects, Esc goes back, and each screen shows its hints in the footer.

| Key | Action |
|---|---|
| Up / Down | Move selection |
| Enter / Space | Select / toggle play-pause |
| Esc | Back / quit from Home |
| Left / Right | Seek −5 s / +5 s (Now Playing) |
| Tab | Cycle Now Playing focus (prev / play / next) |
| P / N | Previous / Next track |
| S / R | Toggle shuffle / repeat |
| F | Toggle favorite |
| Q | Open the play queue |

## Screens

| Screen | Factory | Purpose |
|---|---|---|
| Home | `home` | Now-playing hero + navigation for every screen |
| Now Playing | `nowplaying` | Progress, volume, shuffle/repeat, favorite, seek, queue |
| Queue | `queue` | Play queue: view, remove, clear |
| Albums | `albums` | Album grid from the library |
| Album | `album` | Album detail: tracks, play album |
| Artists | `artists` | Artist list |
| Artist | `artist` | Tracks by one artist |
| Songs | `songs` | Every song in the library |
| Search | `search` | Live title/artist search |
| Favorites | `favorites` | Favorited songs |
| Recent | `recent` | Recently added songs |
| Playlists | `playlists` | Create / delete playlists |
| Playlist | `playlist` | Playlist tracks: play, remove |
| Scan | `scan` | Rescan progress, start / cancel |
| Settings | `dapsettings` | Volume, scan folder, default repeat/shuffle, rescan, about |

## Project layout

```
src/app        Application (services, screen registration, frame loop, shutdown)
src/audio      AudioService + libmpv backend + null simulator
src/core       Renderer, logger, path utils, time
src/dap        DAP screen base + music screens (library / playback / playlist / settings)
src/database   SQLite wrapper, schema, migrations
src/events     EventBus (subsystem decoupling)
src/library    LibraryService, background scanner, album art cache
src/metadata   ffprobe / taglib extractors, JSON parser
src/models     Song / Album / Artist / Playlist models
src/playback   PlaybackController (state machine), QueueManager, PlaylistEngine
src/screens    ScreenManager + DAP Home screen
src/services   SettingsManager, BatteryManager, StorageManager
src/views      Reusable widgets (album art, progress, bottom bar, song info)
tests          7 test suites (ctest)
docs           Architecture, benchmarks, Pi runbook, boot, cross-compile
packaging      systemd unit for Raspberry Pi OS Lite
tools          pi-audit.sh one-shot resource report
```

## Data model

Everything is persisted in a single SQLite database
(`~/.flachead/flachead.db`):

- `songs` — one row per file: path, tags, duration, play count, favorite flag,
  embedded-art path
- `playlists` / `playlist_tracks` — user playlists (FK cascade)
- `settings` — typed key/value settings (volume, repeat, shuffle, scan root)
- `play_history` — played-track history
- `albums` / `artists` — present in the schema; album/artist views are computed
  from `songs` with `GROUP BY`

The schema is versioned with `PRAGMA user_version` and migrations run on open.

## Architecture

See `docs/ARCHITECTURE.md` for the full design. Short version:

- Every subsystem talks through `events::EventBus`; screens never touch the
  Database or audio backend directly.
- `Application::SetupServices` opens the DB, wires SettingsManager → playback
  (volume store, play counting) and LibraryService → scanner, restores persisted
  state, then registers the screen factories.
- The main loop ticks animators/app manager, pumps audio backend events,
  advances playback, updates screens, then renders only when something changed
  (idle path blocks in `SDL_WaitEventTimeout(50 ms)`).
- Shutdown is explicit and ordered so the EventBus outlives every subscriber
  (it is destroyed last), preventing use-after-free during teardown.
