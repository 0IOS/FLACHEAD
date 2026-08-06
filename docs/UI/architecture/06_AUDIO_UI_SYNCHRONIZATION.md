# 06_AUDIO_UI_SYNCHRONIZATION.md

# FLACHEAD UI — Audio UI Synchronization Specification

## 1. Purpose

This document defines how FLACHEAD keeps the user interface synchronized with the audio playback system.

FLACHEAD is a music-first device. The audio engine is the highest-priority system, while the UI acts as a visual representation of playback state.

The synchronization system manages:

* track information
* playback position
* play/pause state
* queue changes
* lyrics timing
* album artwork
* theme updates

Primary principle:

> The UI follows the audio engine. The audio engine never waits for the UI.

---

# 2. Design goals

The synchronization layer must be:

* accurate
* low latency
* non-blocking
* power efficient
* playback-safe

---

# 3. Synchronization architecture

Data flow:

```text id="x7m4q9"
Playback Engine

↓

Media Session Manager

↓

Audio State Events

↓

UI Synchronization Layer

↓

Screens / Components
```

---

The UI never directly queries the decoder continuously.

---

# 4. Source of truth

The playback engine is the authoritative source.

Contains:

```text id="m5x8q3"
Current Track

Playback Position

Duration

Playing State

Volume

Output Device
```

---

UI contains only:

* cached display state
* animations
* visual representation

---

# 5. Playback state model

Core states:

```text id="v8m3x5"
Stopped

Loading

Playing

Paused

Seeking

Error
```

---

Each state produces events.

---

# 6. Track synchronization

When a new track starts:

Flow:

```text id="c6m4x9"
Audio Engine

↓

TRACK_CHANGED Event

↓

UI Update

↓

Load Artwork

↓

Update Theme

↓

Refresh Player
```

---

The audio engine continues immediately.

---

# 7. Playback position updates

Progress updates are frequent.

However:

The UI does not require every audio tick.

---

Recommended:

Audio:

High frequency timing.

UI:

Update:

```text id="r7m3x8"
5-10 times/sec
```

---

Reason:

Enough for smooth progress display while saving CPU.

---

# 8. Progress bar behavior

The progress bar displays:

```text id="n8m4x6"
Current Position

/

Track Duration
```

---

During seeking:

UI temporarily controls visual movement.

After seek:

Audio engine confirms final position.

---

# 9. Play/pause synchronization

Example:

User presses pause:

```text id="w5m3x7"
Input

↓

Playback Command

↓

Audio Engine

↓

STATE_CHANGED Event

↓

UI Update
```

---

The UI should not assume success before confirmation.

---

# 10. Queue synchronization

Queue changes generate:

```text id="a7m4x8"
QUEUE_UPDATED
```

---

Affected UI:

* queue screen
* mini player
* next track indicator

---

The queue manager remains independent from rendering.

---

# 11. Album artwork synchronization

Artwork loading is separate.

Flow:

```text id="p6m8x2"
Track Changed

↓

Show Existing Artwork

↓

Load New Artwork

↓

Update Display
```

---

Avoid blank screens during loading.

---

# 12. Theme synchronization

Music player themes are generated from artwork.

Flow:

```text id="u5m3x9"
Artwork Ready

↓

Theme Engine

↓

Generate Colors

↓

Notify UI

↓

Apply Theme
```

---

Theme generation must never block playback.

---

# 13. Lyrics synchronization

Lyrics depend on playback position.

Flow:

```text id="k8m4x5"
Playback Position

↓

Lyrics Engine

↓

Current Line

↓

Lyrics View Update
```

---

Updates should be lightweight.

---

# 14. Handling delayed updates

Sometimes:

* artwork loads slowly
* database responds late
* theme generation takes time

---

The UI should show:

```text id="d7m3x8"
Previous Data

+

Loading Indicator
```

---

Never freeze.

---

# 15. Seeking behavior

When user seeks:

```text id="h5m8x2"
Touch / Dial Input

↓

Seek Request

↓

Audio Engine

↓

New Position Event

↓

UI Confirmation
```

---

---

# 16. Volume synchronization

Volume sources:

* physical dial
* touchscreen slider
* settings

---

All update:

```text id="s6m4x9"
Media Session Manager
```

---

UI displays final confirmed value.

---

# 17. Error synchronization

Audio errors:

Example:

```text id="b8m3x5"
Decoder Failure
```

---

Flow:

```text id="x7m4q2"
Audio Engine

↓

Error Event

↓

Notification System

↓

UI Message
```

---

Playback recovery happens independently.

---

# 18. Performance requirements

Synchronization must:

* avoid constant polling
* use events
* minimize UI updates
* avoid blocking audio

---

Targets:

Playback event latency:

```text id="d8m4x6"
<100ms
```

---

# 19. Acceptance criteria

Implementation is complete when:

* UI accurately reflects playback
* audio never waits for UI
* lyrics stay synchronized
* themes update safely
* queue changes propagate correctly

---

# 20. Architectural recommendations

Required module:

```text id="m5x8q3"
Audio UI Sync Manager

├── Playback Listener

├── State Cache

├── Event Translator

├── UI Update Scheduler

└── Timing Manager
```

---

# 21. Implementation notes for AI coding agent

When implementing:

* Keep playback engine independent.
* Use events instead of polling.
* Update visuals at appropriate frequency.
* Never perform heavy UI work in the audio thread.
* Treat audio timing as more important than visual precision.

FLACHEAD should feel like a dedicated high-end digital audio player: instant controls, stable playback, and a UI that always reflects the music without interfering with it.
