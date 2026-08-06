# 05_AUDIO_SESSION_API.md

# FLACHEAD UI — Audio Session API Contract

## 1. Purpose

This document defines the communication contract between the FLACHEAD UI and the audio subsystem.

The Audio Session API is the only interface that UI components use to control playback.

The UI must never directly communicate with:

* FLAC decoder
* ALSA/audio backend
* USB audio device
* TANCHJIM BUNNY DSP
* filesystem audio streams

Primary principle:

> The UI controls the session; the audio engine controls the sound.

---

# 2. Audio architecture

Communication flow:

```text id="m7x2q8"
UI Components

       |

       v

Audio Session API

       |

       v

Media Session Manager

       |

       v

Playback Engine

       |

       v

Audio Backend

       |

       v

TANCHJIM BUNNY DSP
```

---

# 3. Audio Session responsibilities

The Audio Session manages:

* playback commands
* current track information
* playback state
* volume state
* queue communication
* audio device status

It does not:

* decode audio
* process DSP signals
* manage files

---

# 4. Audio Session interface

Recommended interface:

```cpp id="x9m3p7"
class AudioSession
{

public:

    play();

    pause();

    togglePlayback();

    stop();

    next();

    previous();

    seek();

    setVolume();

    getState();

    subscribe();

};
```

---

# 5. Playback commands

## play()

Starts playback.

Input:

```text id="p4m8x2"
track_id (optional)
```

Behavior:

```text id="k7x2m5"
Requested

↓

Loading

↓

Playing
```

---

## pause()

Pauses current playback.

The current position is preserved.

---

## togglePlayback()

Used by:

* play/pause button
* hardware play button

Behavior:

```text id="w8m3q6"
Playing

↓

Pause


Paused

↓

Playing
```

---

## stop()

Stops playback.

Position handling depends on configuration.

---

## next()

Requests next queue item.

---

## previous()

Requests previous item.

---

# 6. Seeking

## seek()

Moves playback position.

Input:

```text id="n5x8m2"
position_ms
```

---

Flow:

```text id="r3m7q9"
UI Slider

↓

Audio Session

↓

Playback Engine

↓

New Position
```

---

The UI should not calculate audio buffers.

---

# 7. Volume control

## setVolume()

Input:

```text id="v6m2x8"
volume_percent
```

Range:

```text id="q8x4m1"
0-100
```

---

Sources:

* touchscreen slider
* rotary dial
* buttons

---

All produce:

```text id="s5m9x3"
VOLUME_CHANGED
```

---

# 8. Playback state model

The UI receives:

```text id="a7m3x5"
PlaybackState
```

Containing:

```text id="h2x8m6"
track_id

title

artist

album

duration

position

state

volume
```

---

State values:

```text id="c9m4x7"
Idle

Loading

Playing

Paused

Seeking

Stopped

Error
```

---

# 9. Current track information

The Audio Session provides:

```text id="f8m2x4"
TrackInfo
```

Fields:

```text id="z7m5q1"
id

title

artist

album

genre

duration

artwork_id

codec
```

---

# 10. Queue interaction

The UI requests queue changes through:

```cpp id="x3m8p6"
AudioSession

addToQueue()

removeFromQueue()

moveQueueItem()

clearQueue()
```

---

The Queue Manager owns:

* ordering
* shuffle
* repeat

---

# 11. Repeat modes

Supported:

```text id="m6x2p8"
OFF

REPEAT_ONE

REPEAT_ALL
```

---

API:

```cpp id="q5m9x2"
setRepeatMode()
```

---

# 12. Shuffle mode

API:

```cpp id="w7m3x8"
setShuffle()
```

State:

```text id="p9x4m6"
enabled

disabled
```

---

# 13. Audio device information

The UI can request:

```cpp id="n8m2q5"
getAudioDevice()
```

Returns:

```text id="v4x7m9"
AudioDeviceInfo

name

type

status

sample_rate
```

---

# 14. TANCHJIM BUNNY DSP handling

The DSP is abstracted as an audio device.

The UI receives:

```text id="k3m8x1"
DSP_CONNECTED

DSP_READY

DSP_DISCONNECTED

DSP_ERROR
```

---

Example:

```text id="x5m7q2"
USB Connected

↓

DSP Initialized

↓

Audio Ready
```

---

# 15. Audio quality information

Optional display information:

```text id="r8m4x6"
AudioFormat

codec

sample_rate

bit_depth

channels
```

Example:

```text id="m2x9q5"
FLAC

96kHz

24-bit

Stereo
```

---

# 16. Playback event callbacks

UI subscribes:

```cpp id="b6m3x8"
audioSession.subscribe(
event,
callback
);
```

---

Events:

```text id="p7x2m9"
TRACK_CHANGED

PLAYBACK_STARTED

PLAYBACK_PAUSED

POSITION_CHANGED

VOLUME_CHANGED

DSP_STATUS_CHANGED

PLAYBACK_ERROR
```

---

# 17. Mini Player requirements

Mini Player consumes:

```text id="c8m4x2"
TrackInfo

PlaybackState

Artwork

Position
```

---

Must never:

* create another playback system
* duplicate audio state

---

# 18. Full Player requirements

Full Player consumes:

```text id="g5m8x1"
TrackInfo

PlaybackState

QueueInfo

LyricsState

AudioInfo
```

---

# 19. Hardware controls

Physical controls map:

```text id="u7m2x9"
Play Button

↓

togglePlayback()


Forward

↓

next()


Back

↓

previous()
```

---

Volume dial:

```text id="f4x8m3"
Encoder

↓

setVolume()
```

---

# 20. Error handling

Audio failures:

Example:

```text id="z8m3q5"
DSP disconnected

↓

Audio Session Error

↓

Notification Event

↓

UI Message
```

---

Recovery options:

* reconnect device
* retry playback
* fallback audio device

---

# 21. Threading rules

Audio Session calls should be:

* asynchronous
* non-blocking

Bad:

```text id="n4m8x6"
UI waits for decoder
```

---

Good:

```text id="p6x3m9"
UI sends command

↓

Audio system responds with event
```

---

# 22. Performance requirements

Audio Session:

* minimal latency
* no heavy processing
* no UI dependency

Targets:

Command response:

<50ms

State update:

<500ms

---

# 23. Acceptance criteria

Implementation is complete when:

* UI controls audio only through API
* playback survives screen changes
* DSP is abstracted
* state updates are event-driven
* queue management is separate
* errors recover gracefully

---

# 24. Architectural recommendations

Audio Session:

```text id="y3m7q8"
AudioSession

├── play()

├── pause()

├── seek()

├── setVolume()

├── getState()

├── getTrack()

├── subscribe()

└── unsubscribe()
```

---

Media Session:

```text id="q9m4x2"
Media Session Manager

├── commands

├── state

├── events

└── recovery
```

---

# 25. Implementation notes for AI coding agent

When implementing:

* Keep UI completely independent from audio backend.
* Treat TANCHJIM BUNNY DSP as replaceable hardware.
* Never block the UI thread.
* Use events for playback updates.
* Preserve playback across navigation.
* Keep audio as the highest priority system.

The final Audio Session API should make FLACHEAD behave like a dedicated digital audio player: the interface controls the experience while the audio engine remains reliable and independent.
