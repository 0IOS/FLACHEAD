# FLACHEAD Audio Architecture

Document

01_AUDIO_ARCHITECTURE.md

Category

Audio System

Status

Production Specification

Target Hardware

- Raspberry Pi Zero W
- ARMv6
- Raspberry Pi OS
- 2.8" portrait TFT touchscreen
- TANCHJIM BUNNY DSP USB audio device

---

# 1. Purpose

This document defines the complete high-level architecture of the FLACHEAD audio subsystem.

The purpose of the audio subsystem is to provide reliable, high-quality, offline-first music playback while remaining lightweight enough to operate smoothly on a Raspberry Pi Zero W.

The audio architecture MUST prioritize:

1. Audio correctness
2. Gapless playback
3. Stable real-time playback
4. Low CPU usage
5. Predictable memory usage
6. Bit-depth preservation
7. Sample-rate preservation where supported
8. Reliable USB DAC/DSP operation
9. Clean synchronization with the UI
10. Graceful handling of device and playback failures

The implementation MUST NOT sacrifice audio correctness merely to simplify the UI.

The UI MUST NOT directly control decoder, buffer, or output internals.

---

# 2. Core Design Principle

The audio system is an independent subsystem.

The UI communicates with the audio subsystem through well-defined state and command interfaces.

The audio subsystem owns:

- playback
- decoding
- buffering
- output
- seeking
- track transitions
- queue advancement
- playback state
- audio device state
- audio timing

The UI owns:

- presentation
- user interaction
- visualization
- controls
- album artwork
- lyrics presentation
- metadata presentation

The UI MUST NOT:

- decode FLAC
- write directly to ALSA
- manipulate audio buffers
- perform blocking audio operations
- access the audio device directly
- control decoder threads directly

---

# 3. High-Level Architecture

The complete audio pipeline is:

FLAC File
    ↓
Filesystem
    ↓
Library / Track Metadata
    ↓
Playback Engine
    ↓
Decoder
    ↓
PCM Pipeline
    ↓
Audio Buffer Manager
    ↓
Audio Output Backend
    ↓
ALSA / USB Audio
    ↓
TANCHJIM BUNNY DSP
    ↓
Headphones / Amplifier

Supporting services:

Media Session Manager
    ↓
Playback Engine

Queue Manager
    ↓
Playback Engine

Audio Device Manager
    ↓
Audio Output Backend

Configuration Service
    ↓
Playback / Output / Volume / Audio Policy

Theme / Metadata / Lyrics services
    ↓
UI only

---

# 4. Audio Subsystem Components

The audio subsystem MUST be divided into the following logical components.

## 4.1 AudioManager

Top-level coordinator.

Responsibilities:

- initialize audio subsystem
- start and stop playback
- expose audio state
- coordinate playback engine and output device
- manage audio device changes
- expose diagnostics
- coordinate shutdown

AudioManager MUST NOT perform decoding itself.

---

## 4.2 PlaybackEngine

Owns playback lifecycle.

Responsibilities:

- play
- pause
- stop
- seek
- next
- previous
- track transitions
- queue advancement
- resume position
- playback state

The PlaybackEngine coordinates the decoder and output pipeline.

---

## 4.3 Decoder

Responsible for converting compressed audio into PCM.

Initial required format:

- FLAC

Future formats MAY include:

- WAV
- AIFF
- ALAC
- MP3
- OGG
- Opus

The decoder MUST expose a format-independent PCM interface to the rest of the pipeline.

---

## 4.4 AudioBufferManager

Owns PCM buffering.

Responsibilities:

- decoder output buffering
- output buffering
- underrun detection
- prebuffering
- buffer recycling
- memory limits

Buffers MUST be reusable.

The normal playback path MUST NOT allocate memory continuously.

---

## 4.5 AudioOutputBackend

Provides the abstraction between FLACHEAD and the operating-system audio stack.

Initial target:

ALSA

Architecture:

AudioOutputBackend
    ├── ALSAOutputBackend
    └── FutureOutputBackend

The rest of FLACHEAD MUST NOT depend directly on ALSA APIs.

---

## 4.6 AudioDeviceManager

Responsible for discovering and managing audio devices.

Responsibilities:

- enumerate devices
- detect connection/disconnection
- identify USB DACs
- identify the TANCHJIM BUNNY DSP
- select active output
- report capabilities
- recover from device loss

---

## 4.7 MediaSessionManager

Provides the canonical playback state consumed by the UI.

It MUST expose:

- current track
- artist
- album
- artwork
- playback state
- position
- duration
- queue state
- shuffle state
- repeat state
- output device
- sample rate
- bit depth
- codec
- bitrate
- ReplayGain state
- error state

The Mini Player, Full Music Player, Lyrics View, Queue, and other UI components MUST consume this shared state rather than independently querying the PlaybackEngine.

---

# 5. Data Flow

Normal playback:

User selects track
    ↓
Media Session receives command
    ↓
Queue Manager resolves Track ID
    ↓
PlaybackEngine opens track
    ↓
Decoder initializes
    ↓
Audio format discovered
    ↓
Output backend configured
    ↓
Decoder begins filling PCM buffers
    ↓
AudioBufferManager prebuffers
    ↓
Output starts
    ↓
Playback continues
    ↓
Media Session publishes state
    ↓
UI renders state

The UI MUST NOT block waiting for any stage.

---

# 6. Track Representation

The audio subsystem MUST use a canonical Track representation.

Minimum fields:

- track ID
- absolute/normalized file path
- title
- artist
- album
- album artist
- track number
- disc number
- duration
- codec
- sample rate
- bit depth
- channel count
- bitrate
- ReplayGain track gain
- ReplayGain album gain
- ReplayGain track peak
- ReplayGain album peak
- embedded artwork reference
- embedded lyrics reference
- file modification timestamp
- file size

The Library Database owns persistent metadata.

The PlaybackEngine receives a resolved Track object or immutable Track ID.

---

# 7. PCM Contract

The decoder MUST output PCM through a defined internal format.

The internal representation MUST preserve the source's:

- channel count
- sample rate
- sample format
- bit depth

whenever the output device supports the format.

The system MUST NOT perform unnecessary:

- resampling
- bit-depth conversion
- channel mixing
- normalization
- DSP processing

---

# 8. Bit-Perfect Policy

FLACHEAD SHOULD operate in a bit-perfect path whenever technically possible.

For a compatible FLAC file and compatible output device:

FLAC
    ↓
Decode
    ↓
PCM
    ↓
No DSP
    ↓
No ReplayGain modification
    ↓
No resampling
    ↓
No format conversion
    ↓
USB Audio
    ↓
BUNNY DSP

The system MUST clearly report when the path is not bit-perfect.

Examples:

- resampling enabled
- ReplayGain applied
- DSP enabled
- format conversion required
- device does not support source format

The UI MUST NOT claim "Bit Perfect" unless the audio pipeline actually satisfies the configured bit-perfect conditions.

---

# 9. Sample Rate Handling

The output system MUST query the active audio device's supported formats.

For each track:

Source sample rate
    ↓
Device capability check
    ↓
If supported:
    Preserve sample rate

If unsupported:
    Use configured fallback policy

Fallback policies:

1. Software resampling
2. Reject playback with clear error

Default behavior SHOULD prefer software resampling over playback failure, but bit-perfect mode MAY reject unsupported formats.

The selected behavior MUST be configurable.

---

# 10. Bit Depth Handling

The system MUST preserve source bit depth when supported.

Examples:

16-bit FLAC
    ↓
16-bit PCM

24-bit FLAC
    ↓
24-bit PCM

32-bit FLAC
    ↓
32-bit PCM if supported

No conversion should occur unless required by the output device or selected DSP processing.

---

# 11. Channel Handling

Stereo must remain stereo.

The system MUST NOT automatically downmix or upmix unless required.

Supported channel configurations SHOULD be detected from the source and output device.

If a format cannot be represented by the device:

- fail gracefully
- explain the reason
- use configured fallback behavior

---

# 12. Decoder Architecture

Decoder operations MUST run outside the UI thread.

Recommended architecture:

PlaybackEngine
    ↓
Decoder Worker
    ↓
PCM Ring Buffer
    ↓
Audio Output Thread

The decoder SHOULD decode ahead of the audio output position.

The decoder MUST stop producing data when the buffer reaches its configured high-water mark.

This prevents unnecessary CPU consumption.

---

# 13. Buffer Architecture

A ring buffer SHOULD be used for normal PCM transfer.

Conceptually:

+-------------------------------+
| PCM Ring Buffer               |
|                               |
| Read → [audio data] → Write   |
|                               |
+-------------------------------+

The output thread consumes data.

The decoder thread produces data.

The two sides MUST NOT require a mutex for every audio sample/block operation.

Prefer:

- lock-free or low-lock ring buffer
- atomic read/write positions
- fixed-size reusable blocks

The exact implementation may be selected during implementation if it satisfies the requirements in this document.

---

# 14. Buffer States

The system MUST track:

EMPTY

FILLING

READY

PLAYING

LOW

UNDERRUN

DRAINING

FLUSHING

ERROR

The normal path should remain:

FILLING
    ↓
READY
    ↓
PLAYING
    ↓
FILLING

---

# 15. Prebuffering

Before playback begins, the decoder MUST fill a minimum amount of audio data.

Playback MUST NOT begin immediately after opening the file if the buffer is empty.

Initial prebuffering should be sufficient to prevent startup underruns without causing excessive startup latency.

The implementation SHOULD expose configurable:

- minimum prebuffer duration
- target buffer duration
- maximum buffer duration

Default values MUST be validated on actual Pi Zero W hardware.

---

# 16. Underrun Handling

An underrun occurs when the output device requires PCM data but the buffer is empty.

The system MUST:

1. Detect the underrun
2. Record diagnostics
3. Attempt recovery
4. Avoid crashing
5. Notify the Media Session Manager

If recovery succeeds:

Playback continues.

If recovery fails:

Playback transitions to ERROR.

The UI should report a concise error rather than exposing low-level ALSA details.

---

# 17. Overrun Prevention

The decoder MUST NOT continuously decode when the buffer is already sufficiently full.

Use high-water and low-water thresholds.

Example:

LOW WATER
    ↓
Decoder resumes

HIGH WATER
    ↓
Decoder pauses

This prevents unnecessary CPU usage.

---

# 18. Track Transition

Normal transition:

Current track nearing end
    ↓
Queue Manager resolves next Track
    ↓
Decoder prepares next track
    ↓
Next decoder/output state prepared
    ↓
Current track ends
    ↓
Next track starts

The system SHOULD support gapless playback.

---

# 19. Gapless Playback

Gapless playback is a core requirement.

For compatible files:

Track A
    ↓
Track B

MUST transition without:

- audible gap
- unnecessary device reinitialization
- unnecessary pause

The decoder/output pipeline SHOULD prepare the next track before the current track reaches the end.

If the next track requires a different hardware output configuration, the system MUST handle the transition safely.

---

# 20. Seeking

Seeking MUST be asynchronous.

User requests:

Seek → target position

PlaybackEngine:

1. pause/coordinate decoder
2. seek decoder
3. flush PCM buffer
4. decode from target
5. refill buffer
6. resume playback

Old PCM data MUST be discarded.

The UI MUST remain responsive during seeking.

---

# 21. Pause

Pause MUST preserve:

- decoder position
- queue position
- playback position
- output state where possible

Pause SHOULD avoid destroying the decoder unnecessarily.

---

# 22. Stop

Stop MUST:

- stop output
- flush audio buffers
- release decoder resources
- preserve queue according to configuration
- update Media Session state

Stopping playback MUST NOT destroy the library database or metadata caches.

---

# 23. Previous Track

Previous behavior SHOULD depend on playback position.

Example policy:

If position > configurable threshold:

Previous command

→ restart current track

If position <= threshold:

Previous command

→ previous queue item

The threshold MUST be configurable.

---

# 24. Queue Integration

The Queue Manager owns queue ordering.

PlaybackEngine requests:

Next Track

Previous Track

Current Track

The PlaybackEngine MUST NOT independently maintain a second authoritative queue.

This prevents queue desynchronization.

---

# 25. Shuffle

Shuffle MUST operate on queue ordering rather than modifying the underlying library.

The system SHOULD generate a deterministic shuffle order for the current queue.

Current track SHOULD NOT unexpectedly repeat unless required by queue size or configuration.

---

# 26. Repeat

Supported modes:

OFF

REPEAT_TRACK

REPEAT_QUEUE

Repeat state belongs to the Queue/Media Session layer.

PlaybackEngine executes the resulting command.

---

# 27. ReplayGain

ReplayGain MUST be optional.

Supported modes:

OFF

TRACK

ALBUM

ReplayGain MUST NOT be applied in Bit-Perfect Mode.

The system MUST report whether ReplayGain is active.

Gain application MUST occur in a controlled audio-processing stage.

It MUST NOT modify source FLAC files.

---

# 28. Volume

Volume control MUST be separated from digital audio decoding.

The volume dial SHOULD control the active output volume.

Preferred hierarchy:

Hardware/device volume
    ↓
Digital volume only when necessary

If hardware volume is unavailable, software volume MAY be used.

Software volume SHOULD use sufficient precision to avoid unnecessary quality degradation.

Volume changes MUST NOT cause playback interruption.

---

# 29. TANCHJIM BUNNY DSP

The TANCHJIM BUNNY DSP is the primary external USB audio device.

The system MUST identify it reliably.

Identification SHOULD use:

- USB vendor ID
- USB product ID
- ALSA device information
- stable device metadata

The known device observed during development is:

KTMicro TANCHJIM BUNNY DSP

USB ID:

31b2:1112

This MUST be treated as a device profile rather than hardcoded as the only supported audio device.

---

# 30. USB Device Detection

When the BUNNY DSP is connected:

USB device
    ↓
OS detection
    ↓
AudioDeviceManager
    ↓
Capability discovery
    ↓
Device profile
    ↓
Available output

When disconnected:

Device loss
    ↓
AudioDeviceManager
    ↓
Playback state changes
    ↓
UI notification
    ↓
Configured recovery

The system MUST NOT crash when the device disappears.

---

# 31. Device Recovery

If the active audio device disappears:

1. Stop/hold output safely
2. Preserve playback position
3. Mark output unavailable
4. Monitor for reconnection
5. Reinitialize device
6. Restore compatible format
7. Resume according to configuration

Default behavior SHOULD avoid unexpectedly losing the user's playback position.

---

# 32. Audio Device Capabilities

The AudioDeviceManager MUST expose:

- supported sample rates
- supported bit depths
- supported channel counts
- available formats
- device name
- device identifier
- current format
- current sample rate
- current bit depth
- current state

This information feeds the Audio Inspector UI.

---

# 33. Audio Inspector

The system MUST make diagnostic audio information available.

Minimum fields:

Codec

Sample Rate

Bit Depth

Channels

Bitrate

Output Device

Output Format

ReplayGain

DSP State

Resampling State

Buffer State

Bit-Perfect State

Underrun Count

The UI may expose this through the Audio Inspector screen.

---

# 34. Audio Focus

Only one playback owner SHOULD have active audio focus at a time.

Audio focus states:

NONE

FOREGROUND

BACKGROUND_AUDIO

PAUSED

SUSPENDED

ERROR

If another application requests audio focus:

The Audio Focus Manager decides whether:

- current playback pauses
- current playback continues
- volume ducks
- request is denied

Third-party applications MUST NOT bypass the Audio Focus Manager.

---

# 35. Hardware Controls

Hardware controls MUST generate high-level commands.

Examples:

Play/Pause
    ↓
MediaSession::TogglePlayPause()

Next
    ↓
MediaSession::Next()

Previous
    ↓
MediaSession::Previous()

Volume Dial
    ↓
AudioManager::AdjustVolume()

The hardware layer MUST NOT access the decoder directly.

---

# 36. Touch Controls

Touch controls use the same command interface as physical controls.

For example:

UI Play button
    ↓
Media Session command
    ↓
PlaybackEngine

Hardware Play button
    ↓
Media Session command
    ↓
PlaybackEngine

Both paths MUST produce identical behavior.

---

# 37. Thread Model

Recommended threads:

Main/UI Thread

Responsibilities:

- UI
- input
- state rendering

Playback Control Thread

Responsibilities:

- playback commands
- state transitions

Decoder Thread

Responsibilities:

- FLAC decoding
- PCM production

Audio Output Thread

Responsibilities:

- feeding PCM to output device

Library Worker

Responsibilities:

- metadata
- filesystem operations

Artwork Worker

Responsibilities:

- artwork decoding

No blocking filesystem operation MUST execute on the audio output thread.

No UI rendering MUST execute on the audio output thread.

---

# 38. Real-Time Audio Requirements

The Audio Output Thread is latency-sensitive.

It MUST:

- avoid dynamic allocation
- avoid filesystem access
- avoid logging except through lock-free/minimal diagnostics
- avoid blocking locks
- avoid expensive calculations
- avoid UI calls

The output thread should perform only the work required to provide PCM reliably.

---

# 39. Logging

Audio logging MUST be lightweight.

Normal playback SHOULD NOT produce excessive logs.

Important events:

- device connection
- device disconnection
- decoder initialization
- format negotiation
- underrun
- overrun
- seek
- track transition
- playback error
- recovery

Debug logging MAY be enabled through Developer Settings.

---

# 40. Error Model

Audio errors MUST use categorized error codes.

Examples:

AUDIO_DEVICE_NOT_FOUND

AUDIO_DEVICE_LOST

AUDIO_FORMAT_UNSUPPORTED

AUDIO_INITIALIZATION_FAILED

AUDIO_BUFFER_UNDERRUN

AUDIO_DECODER_FAILED

AUDIO_FILE_NOT_FOUND

AUDIO_FILE_CORRUPT

AUDIO_SEEK_FAILED

AUDIO_OUTPUT_FAILED

AUDIO_RESAMPLER_FAILED

Errors MUST contain enough diagnostic information for developers but expose a concise human-readable message to the UI.

---

# 41. Corrupt FLAC

If a FLAC file cannot be decoded:

Playback MUST NOT crash.

Behavior:

1. Report decoder error
2. Mark track unavailable for current playback attempt
3. Log diagnostic information
4. Offer skip
5. Continue to next queue item if configured

The library SHOULD retain the track and mark it as problematic rather than automatically deleting it.

---

# 42. Missing File

If the database references a missing file:

Playback MUST fail gracefully.

The library SHOULD mark the track as unavailable.

The system MUST NOT repeatedly attempt to open the missing file on every UI frame.

---

# 43. Unsupported Format

If the decoder cannot handle a format:

Display:

Unsupported audio format

Do not crash.

Future codec support MUST be implemented through the decoder abstraction.

---

# 44. Shutdown

Shutdown sequence:

Stop accepting playback commands
    ↓
Stop new decoder work
    ↓
Drain/stop output safely
    ↓
Persist playback position
    ↓
Release decoder
    ↓
Release audio output
    ↓
Release device
    ↓
Shutdown audio subsystem

The audio subsystem MUST support graceful SIGTERM handling.

---

# 45. Startup

Startup sequence:

Initialize AudioManager
    ↓
Initialize AudioDeviceManager
    ↓
Discover devices
    ↓
Initialize output backend
    ↓
Initialize decoder subsystem
    ↓
Initialize Media Session
    ↓
Audio subsystem READY

Startup MUST NOT block the UI unnecessarily.

If no audio device is present, FLACHEAD MUST still boot normally.

---

# 46. No-DAC Operation

FLACHEAD MUST remain usable without the BUNNY DSP connected.

Possible outputs:

- onboard/available audio device
- USB DAC
- future Bluetooth device

If no audio output is available:

UI remains functional.

Playback may enter:

OUTPUT_UNAVAILABLE

rather than crashing.

---

# 47. Performance Targets

Target platform:

Raspberry Pi Zero W

The audio subsystem SHOULD meet:

- stable uninterrupted FLAC playback
- no continuous memory growth
- no per-frame allocations
- minimal decoder overhead
- predictable CPU usage
- predictable memory usage

The audio subsystem MUST NOT consume enough CPU to compromise the UI target of stable 30–45 FPS.

---

# 48. Memory Constraints

Memory usage MUST be bounded.

Avoid:

- loading entire FLAC files into RAM
- decoding entire albums
- retaining unnecessary PCM
- unbounded artwork caches
- unbounded queue caches

Use:

- streaming file reads
- bounded PCM buffers
- reusable decoder buffers
- bounded metadata caches
- bounded artwork caches

---

# 49. CPU Optimization

Optimization priorities:

1. Prevent underruns
2. Reduce unnecessary decoding
3. Reduce memory copies
4. Reuse buffers
5. Avoid unnecessary format conversion
6. Avoid unnecessary resampling
7. Avoid unnecessary DSP
8. Avoid polling loops
9. Keep UI and audio workloads isolated

Do NOT optimize by lowering audio quality unless explicitly configured.

---

# 50. Benchmarking

The audio system MUST eventually be benchmarked on actual Raspberry Pi Zero W hardware.

Required measurements:

- decoder CPU usage
- output CPU usage
- total audio CPU usage
- peak RSS
- average RSS
- buffer occupancy
- underruns
- startup latency
- track transition latency
- seek latency
- device initialization time
- gapless transition reliability

Testing MUST include real FLAC files of varying complexity.

---

# 51. Required Test Tracks

The validation collection SHOULD include:

- 16-bit / 44.1 kHz FLAC
- 16-bit / 48 kHz FLAC
- 24-bit / 44.1 kHz FLAC
- 24-bit / 48 kHz FLAC
- 24-bit / 96 kHz FLAC
- 24-bit / 192 kHz FLAC if supported
- mono FLAC
- stereo FLAC
- long FLAC
- short FLAC
- silent FLAC
- high-compression FLAC
- low-compression FLAC
- gapless album
- corrupted FLAC
- missing file

Actual supported formats MUST be determined by the BUNNY DSP and ALSA capability discovery rather than assumed.

---

# 52. Audio Quality Policy

The default FLACHEAD audio path SHOULD be:

Offline

Lossless

No unnecessary DSP

No unnecessary resampling

No unnecessary normalization

No unnecessary format conversion

The system should preserve the source audio as closely as the hardware path allows.

---

# 53. UI Synchronization

The audio system MUST publish state updates to the Media Session Manager.

State updates include:

Playback state

Position

Duration

Track

Queue index

Buffer state

Output device

Audio format

Error state

The UI MUST NOT poll the decoder continuously.

Position updates SHOULD use a reasonable update frequency rather than publishing every audio sample.

Recommended UI position update rate:

10–30 Hz

The audio clock remains authoritative.

---

# 54. Position Accuracy

Playback position MUST be derived from the audio output clock where possible rather than merely counting decoded samples.

This prevents visible UI drift.

The system SHOULD distinguish:

Decoder Position

Buffered Position

Output Position

UI Position

The Output Position is authoritative for what the listener is actually hearing.

---

# 55. State Machine

Minimum PlaybackEngine states:

IDLE

LOADING

BUFFERING

PLAYING

PAUSED

SEEKING

STOPPING

ENDED

ERROR

Example:

IDLE
 ↓
LOADING
 ↓
BUFFERING
 ↓
PLAYING
 ↓
PAUSED
 ↓
PLAYING
 ↓
ENDED
 ↓
LOADING

Errors from any state MUST transition safely to ERROR or a recoverable state.

---

# 56. Command Model

Commands SHOULD be represented explicitly.

Examples:

PLAY

PAUSE

TOGGLE_PLAY

STOP

NEXT

PREVIOUS

SEEK

SET_VOLUME

SET_REPEAT

SET_SHUFFLE

PLAY_TRACK

ADD_TO_QUEUE

REMOVE_FROM_QUEUE

SET_OUTPUT_DEVICE

Commands SHOULD be processed sequentially by the playback-control layer to prevent race conditions.

---

# 57. State vs Command Separation

Commands describe what the user wants.

State describes what the system is currently doing.

Example:

Command:

PLAY

State:

BUFFERING

then:

PLAYING

The UI MUST render state, not assume that a command succeeded immediately.

---

# 58. Thread Safety

Shared audio state MUST use safe synchronization.

Avoid exposing mutable internal objects directly.

Prefer:

- immutable snapshots
- atomic state
- message passing
- lock-free queues where appropriate

The UI should receive consistent state snapshots.

---

# 59. Configuration

Audio configuration MUST be controlled through the Configuration Service.

Examples:

- output device preference
- bit-perfect mode
- ReplayGain
- volume behavior
- resampling policy
- buffer size
- resume playback
- gapless playback
- audio focus behavior

Audio components MUST NOT modify configuration files directly.

---

# 60. Future DSP

DSP support MAY be added later.

Potential DSP stages:

PCM
 ↓
ReplayGain
 ↓
EQ
 ↓
Limiter
 ↓
Other DSP
 ↓
Output

DSP MUST be explicitly disabled when Bit-Perfect Mode is enabled.

DSP modules MUST be optional and bypassable.

The architecture MUST not require DSP to function.

---

# 61. Future Streaming

The architecture MUST allow future network sources without redesigning the playback engine.

Future input:

Network Stream
    ↓
Source Reader
    ↓
Decoder
    ↓
PCM Pipeline

The decoder and output layers should not care whether the source is:

- local file
- network stream
- future plugin

---

# 62. Plugin Architecture

Future audio source plugins MAY provide:

- source
- decoder
- metadata
- lyrics

Plugins MUST interact through stable interfaces.

Plugins MUST NOT directly modify the core PlaybackEngine.

---

# 63. Security and Reliability

Audio files are untrusted input.

Decoders MUST handle malformed files safely.

The system MUST:

- validate file operations
- avoid unbounded allocations from metadata
- avoid trusting metadata sizes blindly
- handle malformed tags
- handle invalid sample rates
- handle invalid channel counts
- handle corrupted frames

A malformed audio file MUST NOT crash FLACHEAD.

---

# 64. Diagnostics

Developer mode SHOULD expose:

Audio Device

Device ID

Codec

Sample Rate

Bit Depth

Channels

Bitrate

Buffer Fill

Buffer Target

Decoder CPU

Audio CPU

Underruns

Output Format

Resampler

ReplayGain

DSP

Bit Perfect

Track Position

Decoder Position

Output Position

This information is essential for validating the Pi Zero W implementation.

---

# 65. Testing Strategy

Audio testing MUST occur at multiple levels.

Unit tests:

- metadata parsing
- ReplayGain calculation
- queue behavior
- state transitions
- command processing
- position calculations

Decoder tests:

- valid FLAC
- corrupt FLAC
- unusual metadata
- high sample rate
- different bit depths

Integration tests:

- decoder → buffer
- buffer → output
- device connection
- device disconnection
- seek
- pause/resume
- gapless playback

Hardware tests:

- Raspberry Pi Zero W
- actual TANCHJIM BUNNY DSP
- real headphones
- real FLAC library

---

# 66. Acceptance Criteria

The audio architecture is considered complete when:

- FLAC playback works reliably
- Playback does not block the UI
- PCM buffers are bounded
- Normal playback produces no continuous allocations
- TANCHJIM BUNNY DSP is detected reliably
- USB disconnect does not crash the system
- USB reconnect can recover playback
- Gapless playback works for compatible tracks
- Seeking works without stale audio
- Pause/resume works reliably
- Queue transitions work correctly
- ReplayGain is optional
- Bit-perfect mode is truthful
- Unsupported formats fail gracefully
- Corrupt FLAC files cannot crash the application
- Audio state is synchronized with the UI
- Audio diagnostics are available
- Pi Zero W CPU and memory usage remain within practical limits

---

# 67. Implementation Rules for the AI Coding Agent

The coding agent MUST follow these rules.

1. Do not invent a second playback system.

2. Do not put playback logic inside UI screens.

3. Do not put decoder logic inside UI code.

4. Do not access ALSA directly from UI code.

5. Do not allocate memory continuously on the audio output thread.

6. Do not perform filesystem operations on the audio output thread.

7. Do not perform blocking network operations on the audio output thread.

8. Do not silently resample audio.

9. Do not silently alter bit depth.

10. Do not claim bit-perfect playback unless the actual pipeline satisfies the requirement.

11. Do not automatically apply DSP.

12. Do not destroy the playback position when a recoverable device error occurs.

13. Do not crash because an audio device disappears.

14. Do not crash because a FLAC file is corrupt.

15. Do not duplicate authoritative queue state.

16. Do not allow UI polling to become the source of playback timing.

17. Do not optimize audio quality away for performance without explicit configuration.

18. Do not use unbounded buffers.

19. Do not introduce unnecessary background threads.

20. Do not make hardware-specific assumptions outside the AudioDeviceManager/device-profile layer.

---

# 68. Required Architecture Boundary

The final implementation SHOULD resemble:

                    FLACHEAD
                       │
        ┌──────────────┴──────────────┐
        │                             │
       UI                         Audio System
        │                             │
        │                       AudioManager
        │                             │
        │                    ┌────────┴────────┐
        │                    │                 │
        │             PlaybackEngine     DeviceManager
        │                    │                 │
        │               Queue Manager      Output Backend
        │                    │                 │
        │                Decoder             ALSA
        │                    │                 │
        │             Buffer Manager           │
        │                    │                 │
        │                    └────────┬────────┘
        │                             │
        │                       USB Audio
        │                             │
        │                    TANCHJIM BUNNY DSP
        │
        └──── Media Session State ─────┘

The exact C++ class names may change during implementation, but the architectural responsibilities MUST remain equivalent.

---

# 69. Relationship With Existing UI Architecture

The UI documentation already defines:

- UI State Machine
- Event System
- Rendering Pipeline
- Resource Lifecycle
- Runtime Thread Model
- Audio/UI Synchronization
- Theme Engine
- Configuration System
- Logging and Debugging
- Testing and Validation

This document MUST complement those systems rather than duplicate them.

The Audio system owns audio behavior.

The UI system owns visual behavior.

The Media Session boundary connects them.

---

# 70. Relationship With FLACHEAD Performance Architecture

The existing FLACHEAD performance work targets Raspberry Pi Zero W with:

- adaptive frame tiering
- 60 → 45 → 30 FPS tiers
- render-time EMA
- no unnecessary per-frame allocations
- resource caching
- bounded memory
- graceful shutdown
- cross-compilation
- ARMv6 optimization

The audio subsystem MUST be designed so that audio playback remains reliable while the UI is operating within those performance constraints.

Audio playback has higher real-time priority than visual animation.

If the system is under load:

1. Preserve audio playback.
2. Reduce visual workload.
3. Reduce animation quality.
4. Reduce background work.
5. Never intentionally sacrifice audio continuity merely to maintain 60 FPS.

The UI may fall from 60 FPS to 45 or 30 FPS.

Audio playback MUST remain continuous.

---

# 71. Final Architecture Principle

FLACHEAD is a dedicated music player.

Therefore:

Audio correctness > visual effects

Playback reliability > animation smoothness

Predictability > unnecessary abstraction

Bit preservation > convenience

Bounded resources > unlimited caching

Offline reliability > network dependency

The final system should be small, deterministic, recoverable, and audibly correct.

The audio subsystem should make the Raspberry Pi Zero W behave like a dedicated music player rather than a general-purpose computer running a music application.