# FLACHEAD Audio Output Backend

Document

05_AUDIO_OUTPUT_BACKEND.md

Category

Audio System

Status

Production Specification

Target Hardware

- Raspberry Pi Zero W
- ARMv6
- Raspberry Pi OS
- TANCHJIM BUNNY DSP
- 2.8" portrait TFT touchscreen

---

# 1. Purpose

This document defines the audio output layer responsible for taking PCM audio produced by the FLACHEAD audio pipeline and delivering it to the physical audio device.

The output layer MUST provide a clean boundary between:

- decoded PCM
- optional audio processing
- operating-system audio APIs
- physical output devices

The output layer MUST remain independent of:

- FLAC decoding
- queue management
- UI rendering
- lyrics
- artwork
- music-library scanning

---

# 2. Position in the Audio Architecture

The intended pipeline is:

FLAC File
    ↓
FLAC Decoder
    ↓
PCM Ring Buffer
    ↓
Optional Audio Processing
    ↓
Audio Output Backend
    ↓
ALSA / OS Audio Layer
    ↓
TANCHJIM BUNNY DSP
    ↓
Headphones

The output backend is the final FLACHEAD-controlled software stage before the operating-system/device layer.

---

# 3. Primary Responsibilities

The Audio Output Backend is responsible for:

- opening an audio output device
- negotiating supported PCM formats
- configuring sample rate
- configuring channel count
- configuring sample format
- configuring hardware/software buffers
- consuming PCM from the audio pipeline
- maintaining continuous output
- reporting underruns
- stopping output
- pausing output where supported
- draining output
- flushing output when required
- closing the device
- reporting device errors

It MUST NOT decode FLAC.

---

# 4. Backend Abstraction

FLACHEAD MUST use an abstract audio-output interface.

Conceptually:

AudioOutputBackend
    ├── AlsaAudioOutputBackend
    ├── NullAudioOutputBackend
    └── FutureAudioOutputBackend

The Playback Engine MUST communicate with the abstraction rather than directly calling ALSA.

This preserves the ability to:

- test without hardware
- replace ALSA
- add alternate devices
- benchmark output independently
- run desktop development builds

---

# 5. Existing Audio Architecture

The project already contains a libmpv-ready AudioBackend abstraction and NullBackend.

The output layer MUST NOT duplicate that abstraction unnecessarily.

Before implementing a new interface, the coding agent MUST inspect the existing audio architecture and integrate with the established boundaries.

If an existing interface already represents the appropriate output responsibility, extend it rather than creating competing abstractions.

---

# 6. Null Backend

A NullAudioOutputBackend SHOULD exist for development and automated tests.

It SHOULD:

- accept PCM
- consume PCM at a configurable simulated rate
- expose output statistics
- simulate underruns where useful
- avoid requiring ALSA
- allow decoder and playback testing without physical hardware

The Null Backend MUST NOT be treated as proof of real hardware performance.

---

# 7. ALSA Backend

The production Linux backend SHOULD use ALSA.

Conceptually:

FLACHEAD
    ↓
AlsaAudioOutputBackend
    ↓
ALSA PCM API
    ↓
USB Audio Device
    ↓
TANCHJIM BUNNY DSP

The ALSA implementation MUST remain behind the output abstraction.

---

# 8. Device Discovery

The output system SHOULD be able to identify available audio devices.

At minimum, diagnostics should be capable of reporting:

- device name
- ALSA identifier
- supported state
- currently selected device

The user-facing UI SHOULD present a simplified device name rather than raw ALSA configuration strings where possible.

---

# 9. Default Device

FLACHEAD SHOULD support a configured default audio device.

If no explicit device is configured:

The implementation MAY use the system default.

However, the selected behavior MUST be deterministic and documented.

---

# 10. TANCHJIM BUNNY DSP

The TANCHJIM BUNNY DSP is the target audio device.

The output architecture MUST treat it as an external audio endpoint.

The decoder MUST NOT contain BUNNY-specific logic.

The output backend MAY contain device-detection or device-configuration logic if required.

However, device-specific code SHOULD remain isolated.

---

# 11. USB Audio

If the BUNNY DSP is exposed as a USB audio device, FLACHEAD SHOULD communicate through the standard Linux audio stack.

The application SHOULD NOT require proprietary device APIs unless the device actually requires them for required functionality.

Preferred:

FLACHEAD
    ↓
ALSA
    ↓
USB Audio
    ↓
BUNNY DSP

---

# 12. Device Capability Discovery

Before opening the device, the backend SHOULD determine:

- supported sample rates
- supported sample formats
- supported channel counts
- supported access modes
- supported buffer configurations

The backend MUST NOT blindly assume that every FLAC source format is accepted by the hardware.

---

# 13. Source Format vs Output Format

The decoder produces source PCM.

The output device may support a subset of those formats.

Therefore:

Source Format
    ↓
Output Capability Check
    ↓
Compatible Output Format

If the source format is directly supported:

PCM can pass through unchanged.

If it is not:

A clearly defined conversion stage MUST be used.

---

# 14. No Silent Conversion

The output backend MUST NOT silently perform unexpected:

- resampling
- bit-depth reduction
- channel mixing
- volume changes
- DSP processing

If conversion is required, it MUST be explicit in the architecture.

---

# 15. Bit-Perfect Path

FLACHEAD SHOULD support a bit-perfect output path where the hardware and configuration allow it.

Preferred path:

FLAC
    ↓
Lossless Decode
    ↓
PCM
    ↓
No DSP
    ↓
No Resampling
    ↓
No Bit-depth Reduction
    ↓
ALSA
    ↓
BUNNY DSP

The output backend itself MUST NOT modify samples in bit-perfect mode.

---

# 16. Output Format Negotiation

Format negotiation SHOULD occur when starting or reconfiguring playback.

Inputs:

- source sample rate
- source bit depth
- source channels
- device capabilities
- configured output mode

Output:

Final PCM format accepted by the device.

The result MUST be stored as part of the active output session.

---

# 17. Sample Rate

The backend SHOULD prefer the source sample rate when the hardware supports it.

Example:

44.1 kHz source
    ↓
44.1 kHz output

48 kHz source
    ↓
48 kHz output

96 kHz source
    ↓
96 kHz output if supported

The backend MUST NOT resample merely for convenience when direct output is possible.

---

# 18. Sample Format

The backend SHOULD preserve source bit depth when supported.

Example:

24-bit source
    ↓
24-bit output

If the ALSA device exposes a packed or unusual representation, the backend MUST correctly map the internal PCM representation to the ALSA sample format.

The mapping MUST be tested with known PCM data.

---

# 19. Channel Count

The backend SHOULD preserve source channels where supported.

Stereo source:

2 channels

Stereo output:

2 channels

If the output device cannot support the source channel configuration, an explicit channel conversion policy is required.

The backend MUST NOT silently discard channels.

---

# 20. ALSA Access Mode

The implementation SHOULD use an efficient ALSA access mode appropriate for streaming playback.

Interleaved PCM may be preferable for a simple stereo USB device.

Non-interleaved mode MAY be used if required by the device or processing pipeline.

The choice SHOULD be based on:

- device compatibility
- CPU overhead
- implementation simplicity
- measured performance

---

# 21. Hardware Buffer

The ALSA hardware buffer provides another layer of buffering.

The architecture therefore contains:

Decoder Buffer
    ↓
Audio Processing
    ↓
FLACHEAD PCM Buffer
    ↓
ALSA Buffer
    ↓
USB Device

The two buffering layers MUST NOT be confused.

The FLACHEAD PCM buffer handles application-level scheduling.

The ALSA buffer handles device-level output timing.

---

# 22. ALSA Period

The backend SHOULD configure an appropriate ALSA period size.

Smaller periods:

- lower latency
- more wakeups
- potentially higher CPU usage

Larger periods:

- greater latency
- fewer wakeups
- potentially greater tolerance for scheduling delays

The correct value MUST be benchmarked on the Pi Zero W.

---

# 23. Buffer and Period Tuning

Initial configuration SHOULD prioritize reliable playback.

Then measure:

- CPU usage
- underruns
- latency
- scheduler behavior
- power usage

Do not optimize latency at the expense of reliable playback without a clear reason.

---

# 24. Real-Time Output Loop

Conceptually:

while playback_active:

    available = pcm_buffer.available()

    if available >= required_frames:
        pcm = pcm_buffer.read(required_frames)
        alsa_write(pcm)
    else:
        handle_buffer_shortage()

The actual implementation may use ALSA's available-frame APIs and blocking/non-blocking behavior.

The output loop MUST avoid unnecessary polling.

---

# 25. Blocking vs Non-Blocking ALSA

The backend MAY use blocking ALSA writes if this provides simpler and more reliable behavior.

Non-blocking mode MAY be used when required for integration with the application's event model.

The audio output path MUST NOT be coupled to the UI event loop merely because ALSA is configured non-blocking.

---

# 26. UI Independence

The audio output thread MUST NOT depend on:

- SDL frame timing
- renderer timing
- UI event processing
- screen transitions
- touchscreen input

A frozen or delayed UI MUST NOT stop the audio output thread.

---

# 27. Output Thread

The output backend SHOULD operate in a dedicated audio-output context.

The exact threading architecture MUST remain consistent with:

docs/UI/architecture/05_RUNTIME_THREAD_MODEL.md

The output path MUST have clearly defined ownership and synchronization.

---

# 28. Priority

Audio output is timing-sensitive.

If practical, the output thread MAY receive an appropriate scheduling priority.

However, the implementation MUST NOT blindly request aggressive real-time scheduling.

It MUST account for:

- permissions
- Raspberry Pi OS behavior
- system stability
- watchdog behavior

Normal playback reliability is more important than theoretical scheduling priority.

---

# 29. Underruns

An underrun occurs when ALSA requires PCM but FLACHEAD cannot provide it in time.

The backend MUST detect ALSA underrun conditions.

Typical ALSA recovery involves preparing the PCM device again before continuing.

The exact recovery sequence MUST follow ALSA semantics.

---

# 30. Underrun Reporting

Every hardware underrun SHOULD increment:

output_underrun_count

Diagnostics SHOULD also expose:

- timestamp
- current track
- buffer occupancy
- output format
- ALSA state

Repeated underruns MUST be treated as a performance problem rather than silently ignored.

---

# 31. Underrun Recovery

The backend SHOULD:

1. Detect the ALSA error.
2. Identify whether it is recoverable.
3. Recover the device where appropriate.
4. Resume playback.
5. Record diagnostics.

Fatal device failures MUST propagate to the Playback Engine.

---

# 32. Device Disconnect

USB audio devices may disappear.

If the BUNNY DSP is disconnected:

The backend MUST detect the failure.

The application MUST NOT crash.

The Playback Engine SHOULD transition to a controlled audio-device error state.

The UI SHOULD report that the output device is unavailable.

---

# 33. Device Reconnect

If reconnect support is implemented:

Device disconnect
    ↓
Output unavailable
    ↓
Device detection
    ↓
Device reconnect
    ↓
Reopen output
    ↓
Reconfigure format
    ↓
Resume or remain paused

The default behavior MUST be deterministic.

Automatic reconnect MUST NOT cause unexpected playback starts.

---

# 34. Device Hotplug

Hotplug detection SHOULD remain separate from the core decoder.

The decoder can continue to exist independently of the physical output state.

Possible architecture:

Decoder
    ↓
PCM Buffer
    ↓
Output unavailable

When the device returns:

PCM/output session can be recreated.

The exact buffering policy during prolonged disconnection MUST be bounded.

---

# 35. Device Loss and Buffering

If the device disappears for an extended period, FLACHEAD MUST NOT continue decoding indefinitely into memory.

The PCM buffer remains bounded.

When the buffer reaches HIGH:

Decoder pauses.

This prevents device loss from causing unlimited memory growth.

---

# 36. Pause

Pause SHOULD stop active audio consumption.

Depending on ALSA semantics, the backend may:

- pause the device
- drain
- stop
- prepare for resume

The selected behavior MUST be documented and tested.

---

# 37. Resume

Resume MUST restore output without corrupting PCM.

If the hardware requires re-preparation:

prepare device
    ↓
resume output

The backend MUST ensure the PCM format remains correct.

---

# 38. Stop

Stop should terminate output for the current track/session.

The normal sequence is:

Stop requested
    ↓
Stop/prepare output
    ↓
Flush application PCM
    ↓
Close or retain device according to policy

The Playback Engine owns the overall stop state.

---

# 39. Drain

Drain means allowing queued PCM already accepted by the output device to finish playing.

Drain SHOULD be used for:

- normal track completion
- controlled end-of-track transitions
- selected shutdown cases

Drain MUST NOT be used for seeking.

---

# 40. Seek

Seeking requires immediate invalidation of old audio.

Therefore:

Seek
    ↓
Stop/flush output
    ↓
Flush PCM buffer
    ↓
Seek decoder
    ↓
Refill
    ↓
Restart output

Old output data MUST NOT continue playing after a seek unless the behavior is explicitly designed as a crossfade.

---

# 41. Track Transition

Normal transition:

Track A
    ↓
PCM drained
    ↓
Track A complete
    ↓
Track B configured
    ↓
Track B PCM
    ↓
Output continues

For gapless playback, the output device SHOULD remain configured when both tracks use a compatible format.

---

# 42. Gapless Playback

If:

Track A format == Track B format

the backend SHOULD avoid unnecessary device reconfiguration.

If formats differ:

A controlled reconfiguration MAY be required.

The Playback Engine MUST account for any transition gap.

---

# 43. Crossfade

Crossfade MUST NOT be implemented inside the output backend.

If FLACHEAD later supports crossfade:

Track A PCM
      +
Track B PCM
      ↓
Audio Processing
      ↓
Output Backend

The output backend remains a transport layer.

---

# 44. Volume

The output backend MUST NOT automatically apply software volume unless explicitly configured.

Volume architecture MUST be defined separately.

Possible paths:

PCM
 ↓
Software Volume
 ↓
ALSA

or:

PCM
 ↓
ALSA
 ↓
Hardware Volume

The chosen implementation MUST be explicit.

---

# 45. Hardware Volume

If the BUNNY DSP exposes a controllable hardware volume endpoint, FLACHEAD MAY use it.

The application MUST detect whether hardware volume is actually available.

It MUST NOT assume that an arbitrary USB audio device exposes a usable mixer control.

---

# 46. Volume Dial

The physical FLACHEAD volume dial belongs to the input/control system.

Its output should eventually modify the audio volume layer.

The dial MUST NOT directly manipulate ALSA from GPIO code.

Preferred:

GPIO
    ↓
InputBackend
    ↓
Volume Command
    ↓
Audio Control
    ↓
Volume Implementation

This keeps hardware input independent from audio output.

---

# 47. Mixer Control

If ALSA mixer controls are used:

The mixer interaction MUST remain inside the audio-control/output subsystem.

The rest of FLACHEAD should not contain raw ALSA mixer operations.

---

# 48. DSP Interaction

The BUNNY DSP may provide its own processing.

FLACHEAD MUST distinguish:

FLACHEAD software processing

from:

BUNNY hardware/DSP processing

The application MUST NOT assume that output PCM remains bit-perfect after external DSP processing.

Bit-perfect mode should therefore describe the software path accurately.

---

# 49. Device Sample-Rate Changes

Some USB audio devices may require reopening or reconfiguring the stream when the sample rate changes.

The backend MUST handle this explicitly.

Example:

44.1 kHz
    ↓
track changes
    ↓
96 kHz
    ↓
device reconfiguration

The output backend MUST NOT send 96 kHz PCM through a stream still configured for 44.1 kHz.

---

# 50. Output Session

The backend SHOULD maintain an explicit output-session state.

Suggested states:

CLOSED

OPENING

READY

PLAYING

PAUSED

DRAINING

STOPPING

ERROR

The implementation MUST prevent invalid transitions.

---

# 51. State Examples

Valid:

CLOSED
    ↓
OPENING
    ↓
READY
    ↓
PLAYING

Valid:

PLAYING
    ↓
PAUSED
    ↓
PLAYING

Valid:

PLAYING
    ↓
DRAINING
    ↓
READY

Invalid:

CLOSED
    ↓
PLAYING

without opening/configuring the device.

---

# 52. Output Error Model

Minimum output errors:

OUTPUT_DEVICE_NOT_FOUND

OUTPUT_OPEN_FAILED

OUTPUT_FORMAT_UNSUPPORTED

OUTPUT_CONFIGURATION_FAILED

OUTPUT_WRITE_FAILED

OUTPUT_UNDERRUN

OUTPUT_DEVICE_DISCONNECTED

OUTPUT_DEVICE_RECOVERY_FAILED

OUTPUT_CLOSE_FAILED

OUTPUT_INVALID_STATE

The UI should receive a high-level representation.

Detailed backend errors belong in diagnostics/logging.

---

# 53. Logging

Normal playback MUST NOT generate per-write logs.

Do not log:

every ALSA write

every PCM period

every output callback

unless explicitly running a diagnostic build.

Debug logs MAY report:

- device opened
- format configured
- buffer configuration
- underrun
- recovery
- device disconnect
- device reconnect
- shutdown

---

# 54. Device Diagnostics

Developer diagnostics SHOULD expose:

- device name
- ALSA identifier
- sample rate
- bit depth
- channels
- sample format
- period size
- buffer size
- output state
- underrun count
- recovery count
- device errors

---

# 55. CPU Usage

The output backend SHOULD consume minimal CPU.

Avoid:

- busy polling
- unnecessary PCM copies
- repeated device reopening
- excessive logging
- unnecessary format conversions

The output backend MUST be benchmarked on the Pi Zero W.

---

# 56. Memory Usage

The output backend MUST have bounded memory usage.

It MUST NOT:

- accumulate PCM indefinitely
- allocate per audio sample
- allocate indefinitely on repeated errors
- retain closed devices
- leak ALSA resources

---

# 57. Resource Ownership

The output backend owns its device-specific resources.

Example:

AlsaAudioOutputBackend

owns:

- snd_pcm_t*
- ALSA configuration state
- output configuration
- backend-specific buffers if any

The Playback Engine owns:

- playback state
- track state
- queue state

The decoder owns:

- decoder state
- source file state

The PCM buffer owns:

- ring buffer memory
- occupancy state

Ownership MUST remain explicit.

---

# 58. Shutdown

Normal shutdown:

Playback Engine
    ↓
Stop output
    ↓
Drain if appropriate
    ↓
Close ALSA
    ↓
Release backend
    ↓
Application exits

Emergency shutdown:

Playback Engine
    ↓
Stop output immediately
    ↓
Close/release device safely
    ↓
Exit

The output backend MUST tolerate repeated shutdown requests safely.

---

# 59. SIGTERM

The application already has graceful SIGTERM handling.

The audio output backend MUST participate in graceful shutdown.

SIGTERM MUST NOT cause:

- ALSA resource leaks
- dangling output threads
- writes to destroyed buffers
- use-after-free
- application hangs

---

# 60. Window Close

A normal application/window close event MUST also shut down the audio output cleanly.

The renderer and audio backend MUST have independent shutdown responsibilities.

Closing the UI MUST NOT leave the audio thread running after the application has exited.

---

# 61. Thread Lifetime

The output thread MUST be joined or otherwise safely synchronized before its owned resources are destroyed.

Incorrect:

destroy buffer
    ↓
output thread still running

Correct:

request stop
    ↓
wake thread
    ↓
thread exits
    ↓
join
    ↓
destroy resources

---

# 62. Output and PCM Buffer Interaction

The output backend SHOULD consume PCM through the PCM Buffer abstraction.

It MUST NOT directly access decoder memory.

Correct:

Decoder
    ↓
PCM Buffer
    ↓
Output Backend

Incorrect:

Decoder
    ↓
Output Backend directly

This keeps producer and consumer responsibilities separate.

---

# 63. Format Mismatch

If output expects:

48 kHz / stereo / 16-bit

but buffer contains:

44.1 kHz / stereo / 24-bit

the backend MUST NOT reinterpret the bytes.

It MUST reject the mismatch or invoke an explicit conversion stage.

---

# 64. Conversion Policy

If format conversion becomes necessary, it MUST be represented explicitly.

Example:

Decoder
    ↓
44.1 kHz / 24-bit
    ↓
Resampler / Converter
    ↓
48 kHz / 16-bit
    ↓
Output Backend

The output backend should remain responsible only for delivering the resulting compatible PCM.

---

# 65. Testing Without Hardware

The Null Backend MUST support:

- decoder tests
- buffer tests
- Playback Engine tests
- format negotiation tests
- track transition tests
- pause/resume tests
- seek tests
- EOF tests
- underrun simulation

This allows most audio logic to be tested without the BUNNY DSP.

---

# 66. Hardware Testing

The ALSA backend MUST eventually be tested with the real BUNNY DSP.

Required checks:

- device detection
- device opening
- format negotiation
- 44.1 kHz playback
- 48 kHz playback
- 24-bit playback where supported
- sustained playback
- pause/resume
- seeking
- track transitions
- shutdown
- reconnect behavior if supported

---

# 67. Audio Test Tracks

Hardware validation SHOULD include:

- silence
- sine wave
- impulse
- low-frequency tone
- high-frequency tone
- stereo channel-identification track
- normal music
- high-resolution FLAC
- high-compression FLAC

Known test signals make output errors easier to identify.

---

# 68. Output Integrity

The backend MUST preserve PCM ordering.

No:

- dropped frames
- duplicated frames
- reordered frames

should occur during normal playback.

Any conversion stage must explicitly document its effects.

---

# 69. Long-Run Test

The output backend SHOULD be tested for extended playback.

Recommended:

At least several hours of continuous playback on the Pi Zero W.

Monitor:

- underruns
- memory growth
- CPU usage
- device stability
- temperature if useful
- track transitions
- buffer behavior

The objective is to detect issues that short tests miss.

---

# 70. Stress Test

Audio playback SHOULD be tested while the system is under realistic workload.

Examples:

- UI animations
- library browsing
- album-art loading
- lyrics display
- task overview
- launcher
- settings
- filesystem activity

The audio path should remain stable.

---

# 71. Performance Acceptance Criteria

The output backend is considered production-ready when:

- normal playback is continuous
- no persistent underruns occur
- CPU usage is acceptable on Pi Zero W
- memory usage remains bounded
- device configuration is reliable
- supported formats play correctly
- seeking works
- pause/resume works
- track transitions work
- shutdown is safe
- device errors do not crash FLACHEAD
- the UI cannot block audio output
- diagnostics expose meaningful failures

---

# 72. AI Coding Agent Rules

The coding agent MUST:

1. Inspect the existing AudioBackend abstraction before adding interfaces.
2. Reuse existing architecture where appropriate.
3. Keep ALSA behind an output abstraction.
4. Keep decoder logic out of the output backend.
5. Keep UI logic out of the output backend.
6. Keep queue logic out of the output backend.
7. Keep artwork and lyrics out of the output backend.
8. Support a Null Backend for testing.
9. Detect device capabilities.
10. Avoid silent format conversion.
11. Preserve source format whenever hardware permits.
12. Handle ALSA underruns.
13. Handle device failures safely.
14. Avoid busy waiting.
15. Avoid per-period allocations.
16. Avoid per-period logging.
17. Keep memory bounded.
18. Make shutdown thread-safe.
19. Validate with the actual TANCHJIM BUNNY DSP.
20. Measure performance on Raspberry Pi Zero W before declaring the backend optimized.

---

# 73. Final Architecture

The final intended structure is:

                         PLAYBACK ENGINE
                               │
                               ▼
                         PCM BUFFER
                               │
                               ▼
                      AUDIO PROCESSING
                         /          \
                    disabled       enabled
                       │              │
                       └──────┬───────┘
                              ▼
                    AUDIO OUTPUT BACKEND
                              │
                 ┌────────────┴────────────┐
                 │                         │
              NULL                       ALSA
                 │                         │
             TESTING                  USB AUDIO
                                           │
                                           ▼
                                  TANCHJIM BUNNY DSP
                                           │
                                           ▼
                                       HEADPHONES

The output backend is deliberately narrow.

Its job is to take correctly formatted PCM and deliver it reliably to the selected audio device.

It must not become a second Playback Engine, a decoder, a DSP processor, or a UI subsystem.

The Raspberry Pi Zero W is the primary performance constraint, so reliability, bounded memory usage, low CPU overhead, and measurable behavior take priority over unnecessary abstraction or complexity.