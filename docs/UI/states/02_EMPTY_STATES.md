# 02_EMPTY_STATES.md

# FLACHEAD UI — Empty States Specification

## 1. Purpose

This document defines how FLACHEAD handles situations where expected content or resources are unavailable.

As an offline-first music player, FLACHEAD must gracefully handle:

* no music installed
* empty playlists
* missing artwork
* no queue
* empty search results
* unavailable hardware resources

Primary principle:

> An empty state should guide the user toward the next action instead of feeling like an error.

---

# 2. Design goals

Empty states must be:

* clear
* helpful
* lightweight
* consistent
* offline-friendly

---

# 3. Empty state architecture

Data flow:

```text id="q7m4x2"
Service Status

↓

State Manager

↓

Empty State Component

↓

User Action
```

---

The UI must not create empty states manually.

Every empty condition should be reported by:

* Library Service
* Queue Manager
* Storage Manager
* Search Engine
* Playlist Manager

---

# 4. Empty state structure

Every empty state contains:

```text id="m5x8q3"
1. Icon / Visual Indicator

2. Title

3. Explanation

4. Suggested Action
```

---

Example:

```text id="v8m3x5"
No Music Found

Add FLAC files to your SD card

[Scan Library]
```

---

# 5. Empty music library

Condition:

```text id="c6m4x9"
Track Count = 0
```

---

Screen:

```text id="r7m3x8"
+----------------------+
|                      |
|      ♪               |
|                      |
| No Music Yet         |
|                      |
| Add music files to   |
| your SD card         |
|                      |
| [Scan Library]       |
|                      |
+----------------------+
```

---

Actions:

Primary:

* Scan Library

Secondary:

* Open Storage Manager

---

# 6. Empty queue

Condition:

```text id="n8m4x6"
Queue Size = 0
```

---

Display:

```text id="w5m3x7"
Queue Empty

Add songs to start playback
```

---

Action:

```text id="a7m4x8"
Browse Music
```

---

# 7. Empty playlist

Condition:

Playlist exists but contains no tracks.

---

Display:

```text id="p6m8x2"
Playlist Empty

Add songs from your library
```

---

Actions:

* Add Tracks
* Browse Library

---

# 8. Search empty results

Condition:

No matching results.

---

Display:

```text id="u5m3x9"
No Results Found

Try another search
```

---

Avoid:

Showing generic errors.

---

# 9. Missing album artwork

Condition:

Track has no embedded artwork.

---

Behavior:

Generate fallback artwork.

---

Fallback:

```text id="k8m4x5"
Default Album Image

+

Generated Accent Color
```

---

Never display broken images.

---

# 10. Missing metadata

Condition:

Incomplete tags.

---

Fallback values:

Artist:

```text id="d7m3x8"
Unknown Artist
```

---

Album:

```text id="h5m8x2"
Unknown Album
```

---

Track:

```text id="s6m4x9"
Track Name
```

---

# 11. Missing storage

Condition:

SD card unavailable.

---

Display:

```text id="b8m3x5"
Music Storage Missing

Insert SD card
```

---

Actions:

* Retry
* Open Storage Manager

---

# 12. Missing DSP

Condition:

TANCHJIM BUNNY DSP unavailable.

---

Do not block playback.

---

Display notification:

```text id="x7m4q2"
DSP unavailable

Using default audio output
```

---

# 13. Empty favorites

Condition:

No favorites saved.

---

Display:

```text id="z5m3x8"
No Favorites Yet

Tap the heart icon while listening
```

---

# 14. Empty recent history

Condition:

No playback history.

---

Display:

```text id="e6m4x7"
Nothing Played Yet
```

---

Action:

Browse Library.

---

# 15. Empty state animations

Allowed:

* fade in
* small icon movement

Duration:

100-200ms

---

Avoid:

* large illustrations
* heavy animations

Reason:

Pi Zero W resources.

---

# 16. Theme behavior

Empty states follow parent screen theme.

Examples:

Music screens:

```text id="j7m3x9"
Album-derived colors
```

---

System screens:

```text id="y8m4x2"
System theme
```

---

# 17. Performance requirements

Empty states must:

* render instantly
* use cached assets
* avoid network dependency

---

Target:

```text id="f6m3x8"
<100ms display time
```

---

# 18. Error vs empty distinction

Important:

Empty:

```text id="q5m8x7"
Nothing exists yet
```

---

Error:

```text id="u8m3x6"
Something failed
```

---

Never confuse the two.

---

# 19. Acceptance criteria

Implementation is complete when:

* every empty condition has UI
* users know the next step
* no blank screens exist
* fallback content works
* offline operation remains possible

---

# 20. Architectural recommendations

Required component:

```text id="c8m4x6"
EmptyState

├── Icon

├── Title

├── Description

└── Action Button
```

---

# 21. Implementation notes for AI coding agent

When implementing:

* Never leave empty screens blank.
* Keep messages short.
* Always provide recovery actions.
* Avoid technical error messages for normal empty conditions.
* Reuse one empty-state component everywhere.

A good empty state makes FLACHEAD feel like a finished hardware product even before the library is populated.
