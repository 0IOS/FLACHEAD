# 04_AUDIO_UI_ARCHITECTURE.md

# FLACHEAD UI — Audio System Architecture Foundation

## 1. Purpose

This document defines how the FLACHEAD UI communicates with the audio system.

FLACHEAD is a music-first device. The audio pipeline is the core function of the entire operating system.

The UI must provide control and visualization without directly managing audio processing.

The architecture separates:

* audio playback
* media state
* library management
* metadata
* lyrics
* visual presentation

Primary principle:

> The UI observes and controls the music system; it does not become the music system.

---

# 2. Design goals

The Audio UI Architecture must:

* Keep playback independent from UI.
* Allow UI restarts without stopping music.
* Support offline FLAC playback.
* Support TANCHJIM BUNNY DSP.
* Handle large music libraries.
* Keep Raspberry Pi Zero W responsive.
* Support future audio hardware.

The UI should never:

* decode audio
* access audio buffers directly
* manage USB audio communication
* control DSP hardware directly

---

# 3. Audio architecture overview

The complete FLACHEAD audio stack:

```text id="h8k3q1"
                 UI Layer

                     |

              Media Session API

                     |

              Audio Service Layer

                     |

     +---------------+----------------+

     |               |                |

Playback       Queue Manager    Library System
Engine

     |

     |

Audio Backend

     |

USB Audio Interface

     |

TANCHJIM BUNNY DSP

     |

Headphones
```

---

# 4. Core audio services

FLACHEAD separates audio responsibilities into independent services.

---

# 4.1 Playback Engine

Responsible for:

* decoding FLAC
* playback state
* seeking
* buffering
* playback control

Owns:

```text id="x7m1z5"
Current Track

Playback Position

Playback State

Buffer Status
```

---

The UI only receives:

```text id="v4p9k2"
Playing

Paused

Stopped

Position Changed

Track Finished
```

---

# 4.2 Media Session Manager

The bridge between UI and playback.

Responsibilities:

* expose playback controls
* broadcast playback events
* manage current media information

Architecture:

```text id="g5w2r8"
UI

↓

Media Session Manager

↓

Playback Engine
```

---

Controls:

```text id="k6x9m0"
play()

pause()

next()

previous()

seek()

setVolume()
```

---

# 4.3 Queue Manager

Responsible for:

* upcoming tracks
* shuffle
* repeat
* playback order

Owns:

```text id="f8q2n5"
Queue

Current Index

Repeat Mode

Shuffle State
```

---

The UI displays queue data but does not manage it.

---

# 4.4 Library Database

Responsible for:

* songs
* albums
* artists
* playlists
* metadata references

Example:

```text id="w9m2k6"
Song

id

title

artist

album

path

duration

artwork_id
```

---

# 4.5 Metadata Service

Responsible for extracting:

* title
* artist
* album
* codec information
* embedded artwork
* technical details

Runs independently.

---

# 4.6 Lyrics Engine

Responsible for:

* synchronized lyrics
* timestamps
* offline lyric files

The UI receives:

```text id="m4p8x1"
Current lyric line

Next lyric line

Sync position
```

---

# 5. Media session flow

Example:

User presses play.

```text id="r7k3n9"
Touch/Button

↓

UI Component

↓

Media Session Manager

↓

Playback Engine

↓

Audio Backend

↓

DSP

```

The UI never waits for decoding.

---

# 6. Playback state model

Central state:

```text id="q5m8z0"
Playback State

track_id

state

position

duration

volume

repeat_mode

shuffle_mode

audio_device
```

---

States:

```text id="n2x7p4"
Loading

Playing

Paused

Seeking

Stopped

Error
```

---

# 7. Event communication

Audio uses events.

Example:

Track change:

```text id="b6v9c3"
Playback Engine

↓

TrackChanged Event

↓

Media Session Manager

↓

UI Components
```

---

Events:

```text id="z8k1m5"
TrackStarted

TrackPaused

TrackFinished

PositionUpdated

VolumeChanged

AudioDeviceChanged

PlaybackError
```

---

# 8. UI update strategy

The UI should not constantly ask:

Bad:

```text id="p2r6x8"
Every frame:

"What is playback position?"
```

---

Good:

```text id="s3m7k9"
Playback Engine

sends updates

when state changes
```

---

Position updates:

Recommended:

250-500ms intervals.

---

# 9. Album artwork pipeline

Artwork flow:

```text id="v6q2m8"
Music File

↓

Metadata Service

↓

Artwork Cache

↓

Theme Engine

↓

Music UI
```

---

Requirements:

* resize artwork
* cache processed images
* avoid repeated decoding

---

# 10. TANCHJIM BUNNY DSP integration

The DSP is treated as an external audio device.

Architecture:

```text id="x0n5m7"
Playback Engine

↓

Audio Backend

↓

USB Audio Driver

↓

TANCHJIM BUNNY DSP
```

---

The UI receives:

```text id="r8k4p2"
DSP Connected

DSP Disconnected

DSP Format Changed
```

---

The UI should not:

* send USB commands
* configure DSP registers
* manage USB endpoints

---

# 11. Audio device states

Example:

```text id="c7m2x5"
Audio Device State

connected

initializing

ready

error

disconnected
```

---

# 12. Volume architecture

Volume ownership:

```text id="w3p8n1"
Volume Control

        |

Media Session

        |

Audio Backend

        |

DSP
```

---

Inputs:

* rotary encoder
* touchscreen slider
* buttons

all produce:

```text id="g9v4m6"
VolumeChanged Event
```

---

# 13. Music player screen data

Full player requires:

```text id="h2k6w9"
Track Info

Artwork

Artist

Album

Position

Duration

Playback State

Queue Position

Lyrics Availability
```

---

This data comes from services.

The screen does not query files.

---

# 14. Library scanning

Scanning must be asynchronous.

Flow:

```text id="n7x3p8"
Storage

↓

Library Scanner

↓

Database

↓

UI Notification
```

---

Never block:

* boot
* playback
* navigation

---

# 15. Error handling

## Decoder failure

Playback Engine reports:

```text id="m8q4v2"
PlaybackError
```

UI shows:

* notification
* recovery options

---

## DSP disconnect

Flow:

```text id="z5w8n3"
DSP Lost

↓

Audio Service

↓

Media Session

↓

UI Notification
```

---

## Library failure

Keep:

* existing database
* current playback

---

# 16. Threading model

Recommended:

```text id="k8p3x6"
Main Thread

UI

Input


Worker Threads

Audio Decode

Database

Metadata

Artwork Processing
```

---

Audio must never depend on frame rate.

---

# 17. Performance requirements

Targets:

Playback latency:

minimal

UI response:

<100ms

Audio interruption:

none during UI changes

Memory:

stable over long sessions

---

# 18. Acceptance criteria

Implementation is complete when:

* UI communicates only through services
* playback survives UI changes
* FLAC playback is independent
* DSP integration is abstracted
* queue management is separated
* metadata is asynchronous
* artwork is cached
* errors recover gracefully
* Pi Zero W remains responsive

---

# 19. Future improvements

Possible additions:

* DSP profiles
* equalizer system
* replay gain
* gapless playback
* Bluetooth audio
* multiple DAC profiles
* advanced waveform visualization

---

# 20. Architectural recommendations

## Media Session API

Create:

```text id="y4q8n2"
Media Session

├── play()
├── pause()
├── next()
├── previous()
├── seek()
├── setVolume()
├── getState()
└── subscribeEvents()
```

---

## Audio Event Bus

Central events:

```text id="s8k2m4"
PlaybackChanged

TrackChanged

VolumeChanged

DeviceChanged

PlaybackError
```

---

## Implementation notes for AI coding agent

When implementing:

* Keep audio and UI completely separate.
* Treat playback as the highest priority service.
* Never block playback for visuals.
* Use events instead of direct dependencies.
* Keep TANCHJIM BUNNY DSP behind an abstraction layer.
* Design for future DAC/audio hardware.

The final architecture should make FLACHEAD behave like a premium digital audio player: the music engine runs independently, while the UI provides a fast and beautiful control surface.
