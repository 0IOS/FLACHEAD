# 37_RECENTLY_PLAYED.md

# FLACHEAD UI — Recently Played Screen Specification

## 1. Purpose

The Recently Played Screen provides a history of tracks previously played by the user.

It allows quick access to:

* recently listened tracks
* recently opened albums
* playback history

The system is designed to make FLACHEAD feel like a personal music device that remembers listening habits while remaining fully offline.

Primary principle:

> Recently Played should provide useful memory without becoming a complicated recommendation system.

---

# 2. Design goals

The Recently Played system must be:

* lightweight
* offline-first
* fast
* privacy-friendly
* storage efficient

---

# 3. Dependencies

Required systems:

* Playback Engine
* Media Session Manager
* Library Database
* Metadata Service
* History Manager
* Queue Manager
* Screen Manager
* Theme Engine

---

# 4. Architecture

Data flow:

```text id="q7m3x8"
Playback Event

↓

History Manager

↓

Local Database

↓

Recently Played List

↓

UI Renderer
```

---

The History Manager records:

* track ID
* playback timestamp
* play duration
* completion state

---

# 5. History recording rules

A track should be recorded when:

* playback exceeds configured threshold
* user intentionally starts playback

Recommended threshold:

```text id="a8m4x2"
30 seconds
```

---

Avoid recording:

* quick previews
* accidental selections
* failed playback attempts

---

# 6. Screen layout

Portrait layout:

```text id="p5m8x7"
+----------------------+
| < Back               |
|                      |
| Recently Played      |
|                      |
| Today                |
| ------------------   |
| Track Name           |
| Artist               |
|                      |
| Yesterday            |
| ------------------   |
| Track Name           |
| Artist               |
|                      |
| Mini Player          |
+----------------------+
```

---

# 7. History item

Displays:

* track title
* artist
* album
* playback date
* duration

Example:

```text id="x6m3q9"
After Dark

Mr.Kitty

Today 14:32
```

---

# 8. Grouping

Default grouping:

```text id="w8m2x5"
Today

Yesterday

Earlier This Week

Older
```

---

Future:

* monthly grouping
* listening statistics

---

# 9. Playback behavior

Selecting item:

```text id="n4m7x3"
Track Selected

↓

Queue Manager

↓

Play Track
```

---

Options:

* play selected
* continue history
* clear history

---

# 10. History management

User actions:

## Clear all history

Removes playback records.

Does not:

* delete music
* remove favorites
* modify playlists

---

Confirmation:

Required.

Example:

```text id="r6m8x2"
Clear listening history?
```

---

# 11. Storage management

History must remain small.

Recommended:

Maximum entries:

```text id="c9m5x8"
500-1000 tracks
```

---

Older entries should be automatically removed.

---

# 12. Physical control support

UP/DOWN:

Navigate history.

SELECT:

Play selected track.

BACK:

Return.

PLAY:

Toggle playback.

HOME:

Launcher.

---

# 13. Empty state

If no history exists:

Display:

```text id="v5m3x7"
No listening history yet
```

---

# 14. Theme behavior

Uses:

```text id="m8x4q2"
Music/System Theme
```

---

If opened from player:

Can inherit:

```text id="q3m7x9"
Album Accent Colors
```

---

# 15. Animations

Allowed:

* list loading
* item updates
* screen transitions

Duration:

100-200ms

---

Avoid:

* animated timelines
* graph visualizations by default

---

# 16. Performance requirements

Targets:

Open screen:

<200ms

History query:

<100ms

Scrolling:

30-60 FPS

---

Optimization:

Use:

* indexed database queries
* limited history size
* cached metadata

---

# 17. Error handling

Missing track:

Display:

```text id="z7m3x5"
Track unavailable
```

---

Database error:

Display:

```text id="b6m8x2"
History unavailable
```

---

Corrupt entry:

Skip safely.

---

# 18. Acceptance criteria

Implementation is complete when:

* playback history is recorded
* entries persist after reboot
* tracks can be replayed
* storage remains controlled
* UI remains responsive

---

# 19. Architectural recommendations

Required service:

```text id="u4m9x6"
History Manager

├── Record Playback

├── Query History

├── Remove Entries

├── Limit Storage

└── Persist Data
```

---

Required components:

```text id="k8m3x4"
HistoryItem

DateHeader

ScrollableList

ConfirmDialog
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Keep history separate from favorites.
* Do not build recommendation algorithms.
* Store references instead of file copies.
* Limit database growth.
* Make history recording asynchronous.

Recently Played should feel like a simple memory feature of a dedicated music player, not a streaming service clone.
