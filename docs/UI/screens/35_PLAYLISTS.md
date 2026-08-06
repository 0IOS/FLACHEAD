# 35_PLAYLISTS.md

# FLACHEAD UI — Playlists Screen Specification

## 1. Purpose

The Playlists Screen allows users to organize and access custom collections of music.

It provides an offline playlist management system designed for a dedicated music player.

Users can:

* view playlists
* create playlists
* add tracks
* remove tracks
* play collections

Primary principle:

> Playlists should be simple, fast, and completely independent from online services.

---

# 2. Design goals

The Playlists Screen must be:

* offline-first
* lightweight
* fast on Raspberry Pi Zero W
* easy to control with touchscreen and physical buttons
* integrated with the Queue Manager

---

# 3. Dependencies

Required systems:

* Library Database
* Playlist Manager
* Queue Manager
* Playback Engine
* Metadata Service
* Screen Manager
* Theme Engine

---

# 4. Architecture

Data flow:

```text id="m7x4q8"
User Action

↓

Playlist Manager

↓

Local Database

↓

Track References

↓

Queue Manager

↓

Playback
```

---

The playlist system stores:

* track references
* playlist metadata
* ordering information

It does not duplicate music files.

---

# 5. Screen layout

Portrait layout:

```text id="q5m8x9"
+----------------------+
| < Back               |
|                      |
| Playlists            |
|                      |
| + Create Playlist    |
|                      |
| Favorites            |
| Workout              |
| Road Trip            |
| Late Night           |
|                      |
| Mini Player          |
+----------------------+
```

---

# 6. Playlist list

Each playlist displays:

* name
* number of tracks
* duration
* optional artwork

Example:

```text id="v3m8x2"
Night Drive

42 Tracks

3h 12m
```

---

# 7. Creating playlists

Action:

```text id="p8m4x6"
Create Playlist
```

Flow:

```text id="x7m3q5"
New Playlist

↓

Enter Name

↓

Create Database Entry

↓

Open Playlist
```

---

# 8. Playlist naming

Requirements:

* avoid empty names
* trim unnecessary spaces
* handle duplicate names

---

Example:

Invalid:

```text id="n4m8x1"
""
```

---

Valid:

```text id="c6m2x9"
"Morning Collection"
```

---

# 9. Playlist View

Selecting a playlist opens:

```text id="w5m7x3"
Playlist Details
```

Contains:

* playlist name
* tracks
* play button
* shuffle option

---

Layout:

```text id="a8m3x6"
+----------------------+
| Night Drive          |
|                      |
| [ Play ]             |
|                      |
| 01 Track             |
| 02 Track             |
| 03 Track             |
|                      |
| Mini Player          |
+----------------------+
```

---

# 10. Adding tracks

Tracks can be added from:

* Album View
* Artist View
* Track Details
* Search Results

Flow:

```text id="r6m2x8"
Track Action

↓

Add To Playlist

↓

Select Playlist

↓

Save Reference
```

---

# 11. Removing tracks

Removing a track:

* removes reference only
* does not delete audio file

---

Confirmation required:

```text id="z9m4x5"
Remove from playlist?
```

---

# 12. Playback behavior

Play playlist:

```text id="h7m3x9"
Playlist Selected

↓

Queue Manager

↓

Load Tracks

↓

Start Playback
```

---

Options:

* play all
* shuffle
* repeat

---

# 13. Sorting

Supported:

Playlist order:

```text id="k4m8x2"
Manual
```

---

Playlist list:

```text id="m5x7q3"
Alphabetical

Recently modified
```

---

# 14. Physical controls

UP/DOWN:

Navigate playlists.

SELECT:

Open playlist.

BACK:

Return.

PLAY:

Toggle playback.

HOME:

Launcher.

---

# 15. Theme behavior

Uses:

```text id="b8m4x6"
System Theme
```

---

Optional future:

Playlist artwork generated from:

* first album artwork
* dominant colors

---

# 16. Animations

Allowed:

* list transitions
* dialog opening
* playlist creation feedback

Duration:

100-200ms

---

Avoid:

* animated playlist covers
* unnecessary effects

---

# 17. Performance requirements

Targets:

Playlist open:

<200ms

Track loading:

asynchronous

Scrolling:

30-60 FPS

---

Optimization:

Use:

* database references
* lazy loading
* cached metadata

---

# 18. Error handling

Missing tracks:

Show:

```text id="u6m3x8"
Unavailable Track
```

---

Empty playlist:

Show:

```text id="p4m8x9"
No tracks added
```

---

Database error:

Show recovery state.

---

# 19. Acceptance criteria

Implementation is complete when:

* playlists can be created
* tracks can be added
* tracks can be removed
* playlists play correctly
* data persists after reboot
* performance remains stable

---

# 20. Architectural recommendations

Required service:

```text id="x9m5q7"
Playlist Manager

├── Create

├── Delete

├── Add Track

├── Remove Track

└── Load Playlist
```

---

Required components:

```text id="d7m3x5"
PlaylistCard

TrackListItem

Dialog

ScrollableList
```

---

# 21. Implementation notes for AI coding agent

When implementing:

* Store references, not copied files.
* Keep playlist logic outside UI.
* Use Queue Manager for playback.
* Persist changes safely.
* Handle missing files gracefully.

The Playlist system should feel like a classic high-end DAP feature: simple, reliable, and completely offline.
