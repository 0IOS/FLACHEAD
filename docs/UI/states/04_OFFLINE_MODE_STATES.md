# 04_OFFLINE_MODE_STATES.md

# FLACHEAD UI — Offline Mode States Specification

## 1. Purpose

This document defines how FLACHEAD behaves when operating without network connectivity.

FLACHEAD is designed as an offline-first dedicated music player.

The device must provide a complete experience without:

* internet access
* cloud services
* online accounts
* remote servers

Primary principle:

> Network availability should enhance FLACHEAD, never become a requirement.

---

# 2. Design goals

Offline mode must be:

* fully functional
* transparent
* reliable
* fast
* storage-focused

---

# 3. Offline architecture

Data flow:

```text id="x8m4q2"
Network State

↓

Connectivity Manager

↓

Feature Availability

↓

UI State Manager

↓

User Feedback
```

---

The UI should not constantly check network status.

Network changes should be event-driven.

---

# 4. Offline capabilities

Available offline:

```text id="m5x8q3"
✓ FLAC Playback

✓ Music Library

✓ Queue Management

✓ Playlists

✓ Favorites

✓ Lyrics (cached)

✓ Album Artwork (cached)

✓ Equalizer Profiles

✓ Settings
```

---

Unavailable:

```text id="v7m3x9"
Optional:

Updates

Online Metadata Fetch

Cloud Sync

Streaming
```

---

# 5. Offline state indicator

Optional status indicator.

Example:

```text id="q6m8x4"
Offline Mode
```

---

Should not be permanently visible.

Reason:

Offline is the normal FLACHEAD state.

---

# 6. Startup without network

Boot process:

```text id="r5m3x8"
Power On

↓

Load System

↓

Load Local Database

↓

Ready
```

---

No waiting for:

* network timeout
* server connection
* online authentication

---

# 7. Library behavior

Offline library uses:

Local database:

```text id="n8m4x6"
SQLite

+

Cached Metadata

+

Artwork Cache
```

---

Available:

* browse artists
* browse albums
* search tracks
* play music

---

# 8. Lyrics behavior

Priority:

```text id="w5m3x7"
Embedded Lyrics

↓

Local Lyrics Cache

↓

No Lyrics State
```

---

No network fetching during playback.

---

# 9. Artwork behavior

Priority:

```text id="a7m4x8"
Embedded Artwork

↓

Cached Artwork

↓

Generated Placeholder
```

---

Artwork processing must happen locally.

---

# 10. Metadata behavior

Metadata source priority:

```text id="p6m8x2"
FLAC Tags

↓

Local Database

↓

Filename Fallback
```

---

No dependency on online databases.

---

# 11. Optional network features

When network becomes available:

Possible:

* firmware updates
* metadata improvements
* backup

---

These must remain optional.

---

# 12. Network loss during usage

Example:

Device connected to WiFi.

Network disappears.

Behavior:

Continue:

* playback
* browsing
* settings

---

Show only if relevant:

```text id="u5m3x9"
Network unavailable
```

---

# 13. Offline notifications

Avoid:

"Connect to internet"

messages.

---

Preferred:

```text id="k8m4x5"
Feature unavailable offline
```

---

# 14. Theme behavior

Offline mode follows normal themes.

No special offline theme.

---

Reason:

Offline is the default operating environment.

---

# 15. Performance requirements

Offline mode should be:

* faster than online systems
* lower memory usage
* predictable

---

Advantages:

* no network threads
* no API calls
* local database access

---

# 16. Error handling

Missing local data:

Example:

No cached lyrics:

```text id="d7m3x8"
Lyrics unavailable
```

---

No artwork:

Use fallback.

---

No metadata:

Use filename information.

---

# 17. Acceptance criteria

Implementation is complete when:

* music works without internet
* library loads locally
* cached data works
* network loss does not interrupt playback
* optional features degrade gracefully

---

# 18. Architectural recommendations

Required service:

```text id="h5m8x2"
Connectivity Manager

├── Detect Network

├── Report Availability

├── Enable Optional Features

└── Handle Changes
```

---

Required components:

```text id="s6m4x9"
OfflineIndicator

FeatureUnavailableCard

FallbackArtwork

CachedDataState
```

---

# 19. Implementation notes for AI coding agent

When implementing:

* Never make network calls part of the playback path.
* Avoid online dependencies.
* Cache everything possible.
* Treat local storage as the primary source.
* Test with WiFi completely disabled.

FLACHEAD should feel complete when used in a cabin, airplane, commute, or anywhere without connectivity.
