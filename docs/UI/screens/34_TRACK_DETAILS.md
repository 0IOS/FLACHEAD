# 34_TRACK_DETAILS.md

# FLACHEAD UI — Track Details Screen Specification

## 1. Purpose

The Track Details Screen displays detailed information about an individual audio track.

It provides transparency about the music file and playback quality, which is important for a dedicated FLAC player.

The screen displays:

* track metadata
* audio format information
* file information
* playback details
* available actions

Primary principle:

> FLACHEAD should show the quality and identity of the music without overwhelming the user.

---

# 2. Design goals

The Track Details Screen must be:

* informative
* lightweight
* offline-first
* optimized for small portrait displays
* accessible through normal music navigation

---

# 3. Dependencies

Required systems:

* Metadata Service
* Library Database
* Audio Session API
* Playback Engine
* Artwork Cache
* Theme Engine
* Screen Manager

---

# 4. Architecture

Data flow:

```text id="a8m4x6"
Track ID

↓

Library Database

↓

Metadata Service

↓

Audio Information

↓

UI Display
```

---

The screen does not:

* parse FLAC files directly
* control playback hardware
* modify music files

---

# 5. Screen layout

Portrait layout:

```text id="q5m8x3"
+----------------------+
| < Back               |
|                      |
| Album Artwork        |
|                      |
| Track Title          |
| Artist               |
| Album                |
|                      |
| Audio Information    |
| ------------------   |
| Format: FLAC         |
| Sample: 96 kHz       |
| Bit Depth: 24-bit    |
| Bitrate: Variable    |
|                      |
| Actions              |
| Play                 |
| Add Queue            |
+----------------------+
```

---

# 6. Metadata section

Displayed information:

## Basic metadata

* title
* artist
* album
* genre
* year
* track number

---

## Optional metadata

* composer
* copyright
* comment
* embedded lyrics availability

---

# 7. Audio quality section

Important for FLAC playback.

Display:

```text id="m7x3q9"
Codec:

FLAC


Sample Rate:

96 kHz


Bit Depth:

24-bit


Channels:

Stereo
```

---

Additional:

* compression level
* file size
* duration

---

# 8. Playback information

If track is currently playing:

Show:

```text id="v8m2x5"
Currently Playing

Output:

TANCHJIM BUNNY DSP
```

---

Optional:

```text id="p4x9m6"
Output Format

Volume

ReplayGain
```

---

# 9. Artwork

Artwork priority:

```text id="r5m8x2"
Embedded Artwork

↓

Cached Artwork

↓

Album Artwork

↓

Placeholder
```

---

Artwork handling:

* decode once
* cache resized version
* avoid repeated loading

---

# 10. Actions

Available actions:

## Play

Starts playback.

Flow:

```text id="n7m3x8"
Track Selected

↓

Queue Manager

↓

Playback Engine
```

---

## Add to Queue

Adds without interrupting playback.

---

## Add to Playlist

Future feature.

---

## Open Album

Navigation:

```text id="k6m2x9"
Track Details

↓

Album View
```

---

# 11. Physical control support

UP/DOWN:

Navigate information/actions.

SELECT:

Activate action.

BACK:

Return.

PLAY:

Toggle playback.

---

# 12. Scrolling

Information may exceed screen height.

Use:

* smooth scrolling
* virtual layout

---

Avoid rendering unused sections.

---

# 13. Theme behavior

Primary theme source:

```text id="x4m8q7"
Album Artwork
```

---

Fallback:

System theme.

---

Text contrast must always remain readable.

---

# 14. Animations

Allowed:

* artwork fade-in
* section appearance

Duration:

100-200ms

---

Avoid:

* animated statistics
* constantly changing visuals

---

# 15. Performance requirements

Targets:

Opening:

<300ms

Metadata loading:

asynchronous

Memory:

minimal

---

Optimization:

Cache:

* parsed metadata
* artwork
* audio properties

---

# 16. Error handling

Missing metadata:

Display:

```text id="b7m4x2"
Unknown Information
```

---

Corrupted file:

Display:

```text id="z8m3q5"
Unable to read audio details
```

---

Missing artwork:

Use placeholder.

---

# 17. Acceptance criteria

Implementation is complete when:

* track information displays correctly
* FLAC properties are shown
* playback actions work
* artwork loads efficiently
* no audio interruption occurs

---

# 18. Architectural recommendations

Required components:

```text id="c9m5x3"
ArtworkView

MetadataList

InfoRow

ActionButton
```

---

Required services:

```text id="w6m2x8"
Metadata Service

Library Service

Playback Service

Artwork Cache
```

---

# 19. Future improvements

Possible additions:

* waveform preview
* ReplayGain information
* DSP chain information
* equalizer status
* detailed codec analysis

---

# 20. Implementation notes for AI coding agent

When implementing:

* Do not parse files on the UI thread.
* Use metadata service.
* Keep information cached.
* Reuse existing artwork components.
* Preserve album-based theming.
* Keep the screen lightweight.

The Track Details Screen should make FLACHEAD feel like a high-end digital audio player while respecting Raspberry Pi Zero W limitations.
