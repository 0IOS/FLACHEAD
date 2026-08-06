# 38_NOW_PLAYING_QUEUE_MANAGER.md

# FLACHEAD UI — Now Playing Queue Manager Screen Specification

## 1. Purpose

The Now Playing Queue Manager provides advanced control over the current playback queue.

While the Queue Screen (`19_QUEUE.md`) displays the active queue, this screen focuses on **queue manipulation and playback planning**.

It allows users to:

* view upcoming tracks
* reorder playback
* remove tracks
* save queues
* manage playback flow

Primary principle:

> The user should always understand what will play next.

---

# 2. Design goals

The Queue Manager must be:

* fast
* predictable
* touch-friendly
* usable with physical controls
* independent from audio decoding

---

# 3. Dependencies

Required systems:

* Queue Manager Service
* Playback Engine
* Media Session Manager
* Library Database
* Metadata Service
* Artwork Cache
* Screen Manager
* Theme Engine

---

# 4. Architecture

Data flow:

```text id="x5m8q2"
User Action

↓

Queue Manager

↓

Playback State

↓

Media Session

↓

UI Update
```

---

The UI does not own:

* queue data
* playback state
* track loading

The Queue Manager is the single source of truth.

---

# 5. Relationship with Queue Screen

Existing:

```text id="m8q3x5"
19_QUEUE.md

=

Queue Overview
```

---

This screen:

```text id="q7m4x8"
38_NOW_PLAYING_QUEUE_MANAGER.md

=

Queue Control Center
```

---

# 6. Screen layout

Portrait layout:

```text id="v3m8x9"
+----------------------+
| < Back               |
|                      |
| Now Playing          |
| Track Name           |
| Artist               |
|                      |
| Up Next              |
| ------------------   |
| 01 Track             |
| 02 Track             |
| 03 Track             |
|                      |
| Actions              |
| Move Remove Save     |
|                      |
| Mini Player          |
+----------------------+
```

---

# 7. Current track section

Displays:

* artwork
* title
* artist
* album
* playback progress

---

Example:

```text id="c6m8x2"
Currently Playing

Starlight

Muse

Black Holes and Revelations
```

---

# 8. Queue list

Each queue item displays:

* position
* track title
* artist
* duration

Example:

```text id="w5m3x7"
01

Time

Pink Floyd

7:04
```

---

# 9. Queue actions

## Move Track

Allows changing playback order.

Flow:

```text id="p8m4x5"
Select Track

↓

Move Up/Down

↓

Queue Updated
```

---

## Remove Track

Removes from queue.

Does not:

* delete file
* remove library entry

---

## Play Next

Moves selected track immediately after current track.

---

# 10. Drag and drop

Touchscreen:

Optional future feature.

Recommended initial implementation:

Button-based movement.

Reason:

* easier on small display
* works with physical buttons
* lower complexity

---

# 11. Save Queue

Future feature.

Allows:

```text id="r4m7x2"
Current Queue

↓

Playlist
```

---

Useful for:

* listening sessions
* road trips
* custom mixes

---

# 12. Physical control support

UP/DOWN:

Navigate queue.

SELECT:

Open actions.

BACK:

Return.

PLAY:

Toggle playback.

HOME:

Launcher.

---

# 13. Queue states

## Empty queue

Display:

```text id="n6m2x8"
Queue Empty
```

---

## Single track

Display normally.

---

## Large queue

Use:

* virtualized scrolling
* lazy metadata loading

---

# 14. Theme behavior

Primary:

```text id="b8m5x3"
Album Artwork Derived Theme
```

---

Fallback:

System theme.

---

# 15. Animations

Allowed:

* queue item movement
* removal transition
* playback indicator

Duration:

100-200ms

---

Avoid:

* animated reordering effects
* expensive transitions

---

# 16. Performance requirements

Targets:

Queue update:

<50ms

Scrolling:

30-60 FPS

Large queue:

1000+ tracks supported

---

Optimization:

Use:

* indexed queue storage
* lightweight track references
* cached metadata

---

# 17. Error handling

Track unavailable:

Display:

```text id="k3m7x5"
Unavailable Track
```

---

Queue corruption:

Recovery:

```text id="x8m4q2"
Rebuild Queue
```

---

Playback failure:

Do not crash UI.

---

# 18. Acceptance criteria

Implementation is complete when:

* queue displays correctly
* tracks can be reordered
* tracks can be removed
* playback updates correctly
* physical controls work
* large queues remain responsive

---

# 19. Architectural recommendations

Required service:

```text id="z5m8x1"
Queue Manager

├── Add Track

├── Remove Track

├── Move Track

├── Clear Queue

├── Save Queue

└── Notify Changes
```

---

Required components:

```text id="a7m3x9"
QueueItem

TrackRow

ActionMenu

ProgressIndicator
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Keep Queue Manager as the only queue owner.
* Do not duplicate queue state in UI.
* Use events for updates.
* Avoid rebuilding entire lists after small changes.
* Keep playback uninterrupted.

The Queue Manager should make FLACHEAD feel like a professional dedicated audio player with precise playback control.
