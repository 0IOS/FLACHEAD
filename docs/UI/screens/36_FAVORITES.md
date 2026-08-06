# 36_FAVORITES.md

# FLACHEAD UI — Favorites Screen Specification

## 1. Purpose

The Favorites Screen provides quick access to music marked as preferred by the user.

Favorites are designed as a lightweight personal collection system for frequently played tracks.

The feature provides:

* quick track access
* offline storage
* instant playback
* simple organization

Primary principle:

> Favorites should be the fastest path from opening FLACHEAD to playing preferred music.

---

# 2. Design goals

The Favorites system must be:

* instant
* simple
* offline-first
* low memory usage
* integrated with playback systems

---

# 3. Dependencies

Required systems:

* Library Database
* Favorites Manager
* Metadata Service
* Queue Manager
* Playback Engine
* Theme Engine
* Screen Manager

---

# 4. Architecture

Data flow:

```text id="p8m4x7"
User Action

↓

Favorites Manager

↓

Local Database

↓

Track References

↓

Queue Manager

↓

Playback Engine
```

---

Favorites store:

* track IDs
* timestamp added
* optional user ordering

---

Favorites do not store:

* audio files
* duplicate metadata
* artwork copies

---

# 5. Screen layout

Portrait layout:

```text id="m5x8q2"
+----------------------+
| < Back               |
|                      |
| Favorites            |
|                      |
| 01 Track Name        |
|    Artist            |
|                      |
| 02 Track Name        |
|    Artist            |
|                      |
| 03 Track Name        |
|    Artist            |
|                      |
| Mini Player          |
+----------------------+
```

---

# 6. Favorite item

Each item displays:

* track title
* artist
* album
* duration
* favorite indicator

Example:

```text id="x7m3q9"
Midnight City

M83

Hurry Up, We're Dreaming

4:03
```

---

# 7. Adding favorites

Tracks can be favorited from:

* Full Music Player
* Mini Player
* Track Details
* Album View
* Search Results

Flow:

```text id="q4m8x6"
Favorite Action

↓

Favorites Manager

↓

Database Update

↓

UI Refresh
```

---

# 8. Removing favorites

Removing a favorite:

* removes favorite status
* does not delete the track

---

Confirmation:

Not required.

Reason:

Favorite actions should feel instant.

---

# 9. Playback behavior

Play favorite:

```text id="n6m2x8"
Favorite Selected

↓

Queue Manager

↓

Load Favorites

↓

Playback
```

---

Options:

* play all
* shuffle favorites
* play selected

---

# 10. Sorting options

Supported:

## Default

Recently added.

---

## Alternative

Alphabetical:

```text id="b7m4x9"
Artist

↓

Track
```

---

## Future

Most played.

---

# 11. Physical control support

UP/DOWN:

Navigate favorites.

SELECT:

Open/play track.

BACK:

Return.

PLAY:

Toggle playback.

HOME:

Launcher.

---

# 12. Empty state

When no favorites exist:

Display:

```text id="c5m8x3"
No favorites yet

Add tracks using the heart button
```

---

# 13. Theme behavior

Favorites follows:

```text id="v9m4x2"
Music/System Theme
```

---

When opened from music screens:

May use:

```text id="z8m3q5"
Album-derived accent
```

---

# 14. Animations

Allowed:

* favorite icon transition
* list updates
* screen transition

Duration:

100-200ms

---

Avoid:

* large effects
* animated heart graphics

---

# 15. Performance requirements

Targets:

Open screen:

<200ms

List scrolling:

30-60 FPS

Database lookup:

asynchronous

---

Optimization:

Use:

* indexed track IDs
* cached metadata
* virtualized lists

---

# 16. Error handling

Missing favorite track:

Display:

```text id="r7m2x4"
Track unavailable
```

---

Corrupt database:

Display:

```text id="k8m5x1"
Favorites unavailable
```

---

Missing metadata:

Use fallback values.

---

# 17. Acceptance criteria

Implementation is complete when:

* tracks can be favorited
* favorites persist after reboot
* playback works
* missing files are handled
* UI remains responsive

---

# 18. Architectural recommendations

Required service:

```text id="t6m3x8"
Favorites Manager

├── Add Favorite

├── Remove Favorite

├── Check Status

├── Get Favorites

└── Persist Changes
```

---

Required components:

```text id="w8m4x2"
FavoriteListItem

IconButton

ScrollableList

EmptyState
```

---

# 19. Implementation notes for AI coding agent

When implementing:

* Keep favorites as metadata references.
* Do not duplicate playlist logic.
* Reuse track components.
* Keep favorite actions immediate.
* Use database transactions for persistence.

Favorites should behave like a built-in part of the music library, not a separate application.
