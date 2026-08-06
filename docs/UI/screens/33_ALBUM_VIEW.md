# 33_ALBUM_VIEW.md

# FLACHEAD UI — Album View Screen Specification

## 1. Purpose

The Album View displays the complete contents and metadata of a single album.

It provides the primary album-based browsing experience for FLACHEAD.

The screen allows users to:

* view album information
* browse tracks
* play the album
* add tracks to queue
* manage playback order

Primary principle:

> An album should feel like a complete music object, not just a folder of files.

---

# 2. Design goals

The Album View must be:

* fast
* artwork-focused
* offline-first
* optimized for Raspberry Pi Zero W
* consistent with the premium DAP experience

---

# 3. Dependencies

Required systems:

* Library Database
* Metadata Service
* Artwork Cache
* Playback Engine
* Queue Manager
* Theme Engine
* Screen Manager

---

# 4. Architecture

Data flow:

```text id="g5m8x2"
Album ID

↓

Library Database

↓

Album Metadata

↓

Track List

↓

UI Components
```

---

The Album View does not:

* scan music files
* decode audio
* manage storage

---

# 5. Screen layout

Portrait layout:

```text id="q8m3x5"
+----------------------+
| < Back               |
|                      |
|  Album Artwork       |
|                      |
| Album Name           |
| Artist               |
| Year                 |
|                      |
| [ Play Album ]       |
|                      |
| Track List           |
| ------------------   |
| 01 Track             |
| 02 Track             |
| 03 Track             |
|                      |
| Mini Player          |
+----------------------+
```

---

# 6. Album header

Contains:

* artwork
* album title
* artist
* release year
* track count
* duration

Example:

```text id="v6m2x9"
Random Access Memories

Daft Punk

2013

13 Tracks

74:24
```

---

# 7. Artwork system

Artwork loading priority:

```text id="r4x8m3"
Cached Artwork

↓

Embedded FLAC Artwork

↓

Album Placeholder
```

---

Requirements:

* asynchronous loading
* resized cache
* no render blocking

---

# 8. Play Album action

Primary button.

Behavior:

```text id="p7m4x8"
Play Album

↓

Clear/Replace Queue

↓

Add Album Tracks

↓

Start Playback
```

---

Future option:

User preference:

* replace queue
* append queue

---

# 9. Track list

Each track displays:

```text id="m3x8q5"
Track Number

Title

Duration
```

---

Example:

```text id="z5m2x7"
01

Give Life Back To Music

4:34
```

---

Optional metadata:

* codec
* bitrate
* sample rate

---

# 10. Track interaction

Tap:

```text id="c8m5x2"
Play Track
```

---

Flow:

```text id="x7m3q9"
Track Selected

↓

Queue Manager

↓

Playback Engine

↓

Mini Player Update
```

---

Long press:

Options:

```text id="k4m8x6"
Add to Queue

Add to Playlist

Track Details
```

---

# 11. Physical controls

UP/DOWN:

Navigate tracks.

SELECT:

Open/play selection.

BACK:

Return.

HOME:

Launcher.

PLAY:

Toggle playback.

---

# 12. Track ordering

Default:

Album order.

Preserve:

* disc number
* track number

---

Example:

Multi-disc albums:

```text id="n8m2x5"
Disc 1

01-10


Disc 2

01-12
```

---

# 13. Queue behavior

When selecting a track:

Recommended:

```text id="w5x9m3"
Selected Track

↓

Add remaining album tracks

↓

Play selected track
```

---

Example:

User selects track 5.

Queue:

```text id="h7m4x2"
Track 5

Track 6

Track 7

...
```

---

# 14. Theme behavior

Album View uses:

```text id="b6x3m8"
Album Artwork Derived Colors
```

---

Theme engine extracts:

* primary color
* secondary color
* contrast color

---

Fallback:

System theme.

---

# 15. Animations

Allowed:

* artwork fade-in
* track list reveal
* screen transition

Duration:

100-250ms

---

Avoid:

* rotating album art
* particle effects
* continuous animation

---

# 16. Performance requirements

Targets:

Opening:

<300ms

Scrolling:

30-60 FPS

Artwork loading:

background task

---

Optimization:

Use:

* virtualized track list
* cached artwork
* lazy metadata loading

---

# 17. Error handling

Missing album artwork:

Show placeholder.

---

Incomplete metadata:

Display:

```text id="y5m8x4"
Unknown Album
```

---

Missing tracks:

Skip unavailable files.

---

Database unavailable:

Show recovery state.

---

# 18. Acceptance criteria

Implementation is complete when:

* album metadata loads
* artwork displays
* tracks play correctly
* queue behavior works
* physical controls work
* performance remains stable

---

# 19. Architectural recommendations

Components:

```text id="s8m4x2"
AlbumHeader

ArtworkView

TrackList

TrackListItem

ActionButton
```

---

Services:

```text id="q6m3x8"
Library Service

Artwork Cache

Queue Manager

Playback Engine
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Keep album logic separate from playback.
* Reuse existing components.
* Do not duplicate metadata parsing.
* Use the Queue Manager for all playback actions.
* Preserve album-derived theming.
* Keep rendering lightweight.

The Album View should provide the feeling of opening a physical album on a premium music player while remaining efficient enough for Raspberry Pi Zero W.
