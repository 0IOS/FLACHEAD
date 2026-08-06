# 32_ARTIST_VIEW.md

# FLACHEAD UI — Artist View Screen Specification

## 1. Purpose

The Artist View displays all music content belonging to a specific artist.

It provides a structured offline browsing experience for:

* artist information
* albums
* tracks
* related library content

The screen is designed for a dedicated digital audio player experience.

Primary principle:

> Browsing music should feel natural and fast without requiring a network connection.

---

# 2. Design goals

The Artist View must be:

* fast to open
* optimized for offline libraries
* visually focused on music organization
* lightweight for Raspberry Pi Zero W
* compatible with touchscreen and physical controls

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

```text id="m8x4p2"
Artist ID

↓

Library Database

↓

Artist Metadata

↓

Albums + Tracks

↓

UI Components
```

---

The screen only displays data.

It does not:

* scan storage
* parse metadata
* manage playback directly

---

# 5. Screen structure

Layout:

```text id="q7m3x9"
+----------------------+
| < Back               |
|                      |
| Artist Image         |
| Artist Name          |
|                      |
| Albums               |
| ------------------   |
| Album 1              |
| Album 2              |
|                      |
| Tracks               |
| ------------------   |
| Track 1              |
| Track 2              |
|                      |
| Mini Player          |
+----------------------+
```

---

# 6. Header section

Contains:

* artist artwork (optional)
* artist name
* total albums
* total tracks

Example:

```text id="c5m8x1"
Pink Floyd

15 Albums

167 Tracks
```

---

# 7. Artwork handling

Artist artwork priority:

```text id="v8m2q5"
Artist Artwork

↓

Album Artwork

↓

Placeholder
```

---

Artwork must be:

* cached
* scaled before rendering
* loaded asynchronously

---

# 8. Album section

Displays:

* album cover
* album title
* release year
* track count

Example:

```text id="r3x7m9"
The Dark Side of the Moon

1973

10 Tracks
```

---

Interaction:

Tap album:

Open:

```text id="w6m2x8"
Album View
```

---

# 9. Track section

Displays tracks from the artist.

Information:

* track number
* title
* album
* duration

---

Example:

```text id="p9m4x6"
01

Money

The Dark Side Of The Moon

6:22
```

---

# 10. Track interaction

Tap:

Play track.

Flow:

```text id="k8x3m5"
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

* add to queue
* add to playlist
* view details

---

# 11. Physical control support

UP/DOWN:

Navigate items.

SELECT:

Open selection.

BACK:

Return previous screen.

---

HOME:

Open launcher.

---

# 12. Sorting options

Supported:

Albums:

```text id="m4x8q2"
Release date

Alphabetical
```

Tracks:

```text id="s7m3x9"
Album order

Alphabetical
```

---

Default:

Album release order.

---

# 13. Scrolling behavior

Large artists may contain:

* hundreds of tracks
* many albums

Required:

* virtualized lists
* lazy loading

---

Never render the entire artist library at once.

---

# 14. Animations

Allowed:

* artwork fade-in
* list appearance
* screen transition

Duration:

100-250ms

---

Avoid:

* animated album carousels
* heavy effects

---

# 15. Theme behavior

Artist View follows:

```text id="x5m8q7"
System Theme
```

---

Optional future:

Generate artist-based theme.

---

# 16. Performance requirements

Targets:

Screen opening:

<300ms

Scrolling:

30-60 FPS

Artwork loading:

non-blocking

---

Optimization:

Use:

* metadata cache
* artwork cache
* list virtualization

---

# 17. Error handling

Missing artist data:

Show:

```text id="n4m7x2"
Unknown Artist
```

---

Missing artwork:

Use placeholder.

---

Database error:

Show:

```text id="y8m3q5"
Library unavailable
```

---

# 18. Acceptance criteria

Implementation is complete when:

* artist information loads correctly
* albums display correctly
* tracks are playable
* navigation works
* large artists remain responsive
* playback continues during browsing

---

# 19. Architectural recommendations

Required components:

```text id="b6m2x8"
ArtistHeader

AlbumCard

TrackListItem

ArtworkView

ScrollableList
```

---

Required services:

```text id="u9m4x3"
Library Service

Metadata Service

Artwork Cache

Playback Service
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Keep this screen read-only.
* Do not duplicate library logic.
* Reuse existing list components.
* Use queue manager for playback.
* Cache artwork aggressively.
* Optimize for Pi Zero W memory limits.

The Artist View should feel like a premium offline DAP library browser, not a file explorer.
