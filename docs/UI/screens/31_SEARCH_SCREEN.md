# 31_SEARCH_SCREEN.md

# FLACHEAD UI — Search Screen Specification

## 1. Purpose

The Search Screen provides fast offline searching across the local music library.

It allows users to find:

* songs
* albums
* artists
* genres
* playlists

The system is designed for large offline libraries stored on SD card.

Primary principle:

> Search should feel instant because FLACHEAD already knows the local library.

---

# 2. Design goals

The Search Screen must be:

* fast
* keyboard-free where possible
* optimized for touchscreen
* usable with physical controls
* lightweight on Raspberry Pi Zero W

---

# 3. Dependencies

Required systems:

* Library Database
* Metadata Service
* Search Index
* Screen Manager
* Input Manager
* Theme Engine

---

# 4. Architecture

Search flow:

```text
User Input

↓

Search Manager

↓

Library Index

↓

Results

↓

UI Renderer
```

---

# 5. Search data sources

Searchable fields:

## Track

* title
* artist
* album
* genre

---

## Album

* album name
* artist

---

## Artist

* name

---

## Playlist

* playlist name

---

# 6. Search interface

Layout:

```text
+----------------------+
| Search               |
|----------------------|
| [ Query           ]  |
|                      |
| Results              |
|                      |
| Song                 |
| Album                |
| Artist               |
|                      |
|----------------------|
| Mini Player          |
+----------------------+
```

---

# 7. Search input

Supported methods:

## Touch

On-screen input.

---

## Physical controls

Optional character navigation:

```text
UP/DOWN

SELECT

BACK
```

---

Future:

External keyboard support.

---

# 8. Search behavior

Search begins after:

* typing
* confirmation
* configurable delay

Recommended:

200-300ms debounce.

---

# 9. Result categories

Results grouped:

```text
Songs

Albums

Artists

Playlists
```

---

Example:

```text
Songs (5)

Imagine
Home
Night Drive


Albums (2)

Discovery


Artists (1)

Daft Punk
```

---

# 10. Result interaction

Tap:

Open item.

Examples:

Song:

```text
Play track
```

Album:

```text
Open Album View
```

Artist:

```text
Open Artist View
```

---

Physical controls:

UP/DOWN:

Navigate results.

SELECT:

Open selection.

---

# 11. Search indexing

The Search Screen does not scan files.

The Library Database owns:

* indexing
* metadata storage
* lookup

---

Flow:

```text
SD Card

↓

Library Scanner

↓

Database

↓

Search Index
```

---

# 12. Empty state

No query:

```text
Start typing to search
```

---

No results:

```text
No matching music found
```

---

# 13. Animations

Allowed:

* result fade
* list update transition

Duration:

100-200ms

---

Avoid:

* animated search effects
* expensive transitions

---

# 14. Theme behavior

Search follows system theme.

System apps:

```text
Wallpaper-derived colors
```

---

When opened from music context:

May inherit:

```text
Album-derived accent
```

---

# 15. Performance requirements

Targets:

Search response:

<200ms

Result rendering:

60 FPS preferred

Memory:

Minimal temporary allocation

---

Optimization:

Use:

* indexed search
* cached metadata
* virtualized lists

---

# 16. Error handling

Database unavailable:

Show:

```text
Music library unavailable
```

---

Corrupt metadata:

Skip invalid entries.

---

Missing artwork:

Use placeholder.

---

# 17. Acceptance criteria

Implementation is complete when:

* search works offline
* results appear quickly
* physical controls work
* large libraries remain responsive
* no audio interruption occurs

---

# 18. Architectural recommendations

Required services:

```text
Search Manager

├── Query Parser

├── Index Access

├── Ranking

└── Result Provider
```

---

# 19. Implementation notes for AI coding agent

When implementing:

* Never scan the SD card during search.
* Use database indexes.
* Keep search asynchronous.
* Do not block rendering.
* Preserve playback while searching.

The Search Screen should feel like a dedicated offline music player feature, not a smartphone search page.
