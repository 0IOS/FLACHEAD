# FLACHEAD Playback Engine

Document

02_PLAYBACK_ENGINE.md

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
- Offline FLAC playback

---

# 1. Purpose

This document defines the architecture and behavior of the FLACHEAD Playback Engine.

The Playback Engine is the authoritative runtime component responsible for controlling music playback.

It sits between the Media Session / Queue system and the lower-level decoder, buffering, and audio output systems.

The Playback Engine MUST provide deterministic playback behavior while remaining lightweight enough for the Raspberry Pi Zero W.

It is responsible for:

- loading tracks
- starting playback
- pausing playback
- resuming playback
- stopping playback
- seeking
- advancing tracks
- returning to previous tracks
- coordinating buffering
- coordinating the decoder
- coordinating audio output
- maintaining playback state
- handling playback errors
- preserving playback position
- performing track transitions
- coordinating gapless playback
- reporting playback position
- handling end-of-track events
- exposing timing information to dependent systems
- recovering from playback failures
- coordinating queue transitions
- handling format changes

The primary objective is:

PROVIDE RELIABLE, GAPLESS-ORIENTED, LOW-LATENCY AUDIO PLAYBACK WITHOUT ALLOWING UI, ARTWORK, LYRICS, OR OTHER NON-AUDIO SYSTEMS TO BLOCK THE AUDIO PATH.

The Playback Engine MUST NOT contain UI rendering logic.

---

# 2. Core Principle

The Playback Engine answers:

"What should FLACHEAD be playing, and how should the audio pipeline transition into that state?"

"What should happen next?"

"How should PCM flow through the audio pipeline?"

It is the authoritative runtime owner of:

AUDIO EXECUTION.

It coordinates the lower-level systems but does not unnecessarily duplicate their responsibilities.

---

# 3. Architectural Position

The Playback Engine belongs to the audio subsystem.

Its intended position is:

                         UI
                          │
                          ▼
                  PLAYBACK CONTROLLER
                          │
                          ▼
                    PLAYBACK ENGINE
                          │
          ┌───────────────┼───────────────┐
          │               │               │
          ▼               ▼               ▼
       DECODER         BUFFER          CLOCK
          │               │               │
          └───────────────┼───────────────┘
                          ▼
                    AUDIO OUTPUT
                          │
                          ▼
                    TANCHJIM BUNNY DSP

The Playback Engine is therefore the central orchestration layer for audio playback.

The intended dependency direction is:

UI → Playback Controller → Playback Engine → Decoder + Buffer + Output

NOT:

UI → Decoder.

---

# 4. Ownership

The Playback Engine owns runtime playback state.

It MUST be the authoritative source for:

- current playback operation
- current playback state
- current track identity
- current track execution
- current queue position
- decoder lifecycle
- PCM buffering
- audio-device interaction
- playback position
- seek execution
- skip requests
- end-of-track detection
- playback errors
- playback lifecycle
- seek lifecycle
- transition lifecycle
- playback-session identity
- timing state
- coordination between audio components
- high-level error handling
- playback commands
- track lifecycle
- transition decisions
- PCM pipeline lifecycle

It MUST NOT become the authoritative owner of:

- persistent library metadata
- application settings
- UI state
- UI widgets
- UI rendering
- album artwork
- album artwork rendering
- lyrics data
- lyrics rendering
- touchscreen rendering
- visual animations
- queue ordering
- playlist UI
- music library UI
- screen navigation
- application settings UI
- filesystem library scanning
- filesystem indexing
- metadata database implementation
- touchscreen input handling
- ALSA implementation details
- FLAC decoding internals
- low-level ring-buffer implementation
- queue presentation

Those responsibilities belong to other systems.

---

# 5. Separation of Responsibilities

## Playback Engine owns

- playback lifecycle
- decoder lifecycle
- output lifecycle
- playback commands
- track transitions
- seek operations
- buffering coordination
- playback timing state
- error recovery
- repeat/shuffle policy execution

## Queue Manager owns

- queue contents
- queue order
- shuffle order
- repeat policy
- next-track selection
- previous-track selection

## Playback Controller owns

- playback policy decisions
- next track selection
- repeat mode decisions
- shuffle mode decisions
- stop decisions

## Media Session owns

- public playback state
- UI-facing state snapshots
- synchronization with UI

## Library owns

- persistent track metadata
- file indexing
- database state

## Audio Device Manager owns

- audio device discovery
- device capabilities
- device connection state

---

# 6. Command Model

Playback MUST be controlled through explicit commands.

Minimum commands:

PLAY

PAUSE

RESUME

TOGGLE_PLAY_PAUSE

STOP

PLAY_TRACK

PLAY_QUEUE_ITEM

NEXT

PREVIOUS

SEEK

SET_VOLUME

SET_OUTPUT_DEVICE

CHANGE_OUTPUT

RELOAD_OUTPUT

SET_TRACK

TOGGLE_REPEAT

TOGGLE_SHUFFLE

The command interface SHOULD be asynchronous.

The UI MUST NOT directly invoke blocking playback operations.

Each command SHOULD have:

- command type
- parameters
- optional request ID
- timestamp if useful
- source if useful

Example:

SEEK
    target = 183.5 seconds

---

# 7. Command Sources

Commands may originate from:

- physical buttons
- touchscreen
- task overview
- mini player
- music library
- queue screen
- hardware controls
- UI
- system controls
- future external control interfaces

All of these MUST eventually pass through the same playback control path.

---

# 8. Single Playback Authority

There MUST be one authoritative Playback Engine.

The following MUST NOT independently control audio:

- music player screen
- mini player
- home screen
- hardware button handler
- queue UI

Instead:

UI
 ↓
Playback Command
 ↓
Playback Engine

Physical controls MUST map to the same commands as touchscreen controls.

Example:

Physical Play/Pause
    ↓
TOGGLE_PLAY_PAUSE

Touch Play/Pause
    ↓
TOGGLE_PLAY_PAUSE

No separate playback implementations are allowed.

---

# 9. Command Processing

External systems MUST NOT directly mutate Playback Engine state.

Commands SHOULD be serialized through a playback-control mechanism.

Conceptually:

UI / Hardware
    ↓
Media Session
    ↓
Playback Command Queue
    ↓
Playback Engine
    ↓
State Transition

This prevents simultaneous commands from corrupting playback state.

Example:

User rapidly presses:

NEXT
NEXT
NEXT

The Playback Engine MUST process these commands predictably.

Playback commands should be processed in a deterministic order.

Example:

user presses:

NEXT

then immediately:

STOP.

The engine must process commands in a defined order.

SEEK race example:

SEEK

then:

NEXT.

The seek must not continue modifying the newly selected track.

---

# 10. Command Properties

Some commands may require special handling.

High-priority commands:

STOP

SEEK

NEXT

PREVIOUS

Normal:

PLAY

PAUSE

VOLUME

The implementation may prioritize urgent commands when safe.

A pending operation may become obsolete.

Example:

LOAD Track A

then immediately:

LOAD Track B

Track A's load operation should be cancellable or invalidated.

Commands associated with obsolete playback sessions MUST NOT affect the current session.

Session IDs and command generations SHOULD be used where needed.

---

# 11. Playback States

The Playback Engine MUST use explicit states.

Minimum states:

IDLE

LOADING

BUFFERING

PLAYING

PAUSED

SEEKING

TRANSITIONING

DRAINING

STOPPING

ENDED

ERROR

OUTPUT_UNAVAILABLE

The implementation MAY add more states if necessary.

The state MUST always represent the actual playback condition.

The system MUST NOT report PLAYING while audio output is unavailable.

---

# 12. State Definitions

## IDLE

No track is currently loaded.

No active playback session.

No track currently being decoded.

Decoder:

Inactive

Output:

Inactive

The engine may still have:

- queue contents
- selected track
- previous playback metadata

Possible transitions:

IDLE → LOADING

IDLE → ERROR

---

## LOADING

A track is being opened and prepared.

Operations:

- open file
- validate file
- initialize decoder
- inspect format
- determine source format
- negotiate output format
- create audio resources
- prepare output pipeline

UI remains responsive.

The UI may display:

Loading...

---

## BUFFERING

Decoder is preparing sufficient PCM data before or during playback.

Output may not have started yet.

Playback MUST NOT begin until the configured minimum prebuffer threshold is reached.

BUFFERING may also occur after:

- seek
- underrun
- device recovery

Typical sequence:

LOADING
    ↓
BUFFERING
    ↓
PLAYING

---

## PLAYING

Audio is actively being sent to the output device.

This is the normal playback state.

- decoder is producing PCM as necessary
- output is consuming PCM
- playback position is advancing

The actual audio clock comes from the output pipeline.

---

## PAUSED

Playback has intentionally stopped while retaining the current position.

The current logical playback position is retained.

The decoder MAY remain initialized.

The output MAY remain configured.

The system SHOULD avoid unnecessary decoding while paused.

---

## SEEKING

A seek operation is being performed.

Old buffered PCM MUST NOT be played after the seek completes.

A seek operation is invalidating old buffered audio and rebuilding the playback pipeline.

---

## TRANSITIONING

The current track is ending and the next track is being prepared.

This state is especially important for gapless playback.

---

## DRAINING

No more source PCM is expected, but previously produced/submitted PCM still needs to finish.

The source has reached the end of the track but buffered PCM remains.

Typical transition:

PLAYING
    ↓
EOF
    ↓
DRAINING
    ↓
NEXT TRACK / IDLE

---

## STOPPING

Playback is shutting down.

Buffers are being flushed.

Decoder/output resources are being released or reset.

Typical transition:

PLAYING
    ↓
STOPPING
    ↓
IDLE

---

## STOPPED

The previous playback context has been terminated.

---

## ENDED

Track reached its natural end.

The Queue Manager determines what happens next.

---

## ERROR

An unrecoverable playback error occurred.

The error MUST be exposed to the Media Session.

The system MUST enter ERROR deliberately rather than crash.

---

## OUTPUT_UNAVAILABLE

The requested audio output cannot currently be used.

The track position SHOULD be preserved where possible.

---

# 13. State Machine

Normal playback:

IDLE
  ↓
LOADING
  ↓
BUFFERING
  ↓
PLAYING
  ↓
ENDED
  ↓
LOADING

Pause:

PLAYING
  ↓
PAUSED
  ↓
PLAYING

Seek:

PLAYING
  ↓
SEEKING
  ↓
BUFFERING
  ↓
PLAYING

Stop:

PLAYING
  ↓
STOPPING
  ↓
IDLE

Natural track end:

PLAYING
  ↓
DRAINING
  ↓
IDLE / LOADING

Error:

ANY ACTIVE STATE
  ↓
ERROR

Output loss:

PLAYING
  ↓
OUTPUT_UNAVAILABLE
  ↓
RECOVERY
  ↓
BUFFERING
  ↓
PLAYING

---

# 14. State Transition Rules

The Playback Engine MUST validate transitions.

Invalid transitions MUST NOT corrupt state.

Example:

PLAYING → PLAYING

may simply be ignored or treated as a no-op.

But:

ERROR → PLAYING

should require an explicit recovery/reload operation.

SEEK while IDLE should not corrupt playback state.

---

# 15. Track Identity

Every active track SHOULD have an internal identity.

This may include:

- track ID
- file path
- queue index
- playback session ID

Track identity should use a stable identifier rather than relying only on display title.

This prevents stale operations from affecting a different track.

The engine receives a validated music-file reference.

It should not reconstruct arbitrary filesystem paths from UI text.

---

# 16. Current Track

The engine should maintain a current-track object.

Conceptually:

CurrentTrack

    track_id
    file_path
    duration
    format
    decoder_state
    playback_generation

Exact fields may differ.

---

# 17. Playback Generations

Every new playback session SHOULD receive a unique session/generation identifier.

Example:

Session 100
    Track A

Seek:

Session 101
    Track A

Next:

Session 102
    Track B

Any asynchronous operation belonging to an older session MUST NOT inject data into the current session.

---

# 18. Why Generations Matter

Without session validation:

Track A decoder
    ↓
user presses NEXT
    ↓
Track B starts
    ↓
Track A decoder finishes late
    ↓
Track A PCM enters output

This is unacceptable.

Session IDs prevent stale asynchronous operations.

This applies to:

- artwork
- lyrics
- decoder operations
- UI updates
- end-of-track events
- output operations

---

# 19. Generation Changes

A new generation should be created when appropriate, including:

- track change
- seek
- complete playback restart
- decoder restart

PCM generated by an old generation must not enter the current playback pipeline.

Output operations associated with an old generation must not control the new playback session.

LOAD_TRACK creates a new playback generation.

---

# 20. Track Loading

When a track is requested:

1. Validate Track ID.
2. Resolve track metadata.
3. Resolve file path.
4. Verify file existence.
5. Open file.
6. Initialize decoder.
7. Read stream information.
8. Determine codec.
9. Determine sample rate.
10. Determine bit depth.
11. Determine channel count.
12. Negotiate output format.
13. Initialize buffers.
14. Begin prebuffering.
15. Enter PLAYING when ready.

Any failure MUST transition to a controlled error state.

---

# 21. Track Validation

Before playback:

- file must exist
- file must be readable
- file type must be supported
- stream metadata must be readable
- decoder must be able to initialize

If a track cannot be opened:

playback should enter a controlled error/recovery path.

---

# 22. File Access

File operations MUST NOT occur on the audio output thread.

The Playback Engine MAY request file operations through a worker or decoder layer.

File access SHOULD be sequential wherever possible.

Audio files should be opened using safe filesystem APIs.

The entire FLAC file MUST NOT be loaded into RAM.

---

# 23. Streaming

FLAC data should be streamed from storage.

Compressed input should use bounded read buffers.

Limited compressed-data prefetch may be used.

Do not cache entire FLAC files in RAM.

Do not load the entire FLAC file into RAM.

The system should tolerate SD-card latency without causing unnecessary underruns.

---

# 24. Decoder Initialization

Decoder initialization MUST remain separate from Playback Engine logic.

Decoder initialization MUST produce a canonical audio format:

- sample rate
- bit depth
- channels
- sample format
- duration if available

The Playback Engine passes this information to the Audio Output Backend.

The decoder MUST remain isolated from output-device implementation details.

The engine requests:

open(track)

The decoder handles:

- file access
- FLAC parsing
- STREAMINFO
- frame decoding

The engine receives:

- success/failure
- source format
- decoded PCM

---

# 25. FLAC Decoding

FLAC decoding must preserve the original decoded sample information accurately.

The decoder must not intentionally degrade audio quality.

The playback engine must not introduce lossy conversion unnecessarily.

The decoder may produce the format most appropriate for the audio output pipeline.

Required conversions should be explicit.

The engine must correctly handle supported FLAC sample rates.

The engine must correctly handle supported channel configurations.

The engine must preserve source precision until conversion is required by the output device/API.

---

# 26. Decoder Lifecycle

The decoder lifecycle should be:

OPEN

↓

INITIALIZE

↓

DECODE

↓

FLUSH

↓

CLOSE

The decoder should expose decoded PCM to the playback pipeline.

It should not know about:

- UI
- lyrics
- artwork
- queue navigation

The playback session owns the active decoder instance.

Decoder lifetime should normally correspond to the current playback generation.

Seek may require decoder reset or repositioning.

---

# 27. Decoder EOF

EOF means:

no more source PCM will be generated.

It does not necessarily mean:

nothing remains to play.

End-of-stream must be detected explicitly.

EOF flow:

DECODER EOF

↓

PCM BUFFER DRAINS

↓

OUTPUT DRAINS

↓

TRACK COMPLETION.

---

# 28. Output Negotiation

After decoding the track format:

Playback Engine
    ↓
Audio Output Backend
    ↓
Device Capability Query
    ↓
Format Selection
    ↓
Output Initialization

Preferred behavior:

Use source format directly.

Fallback:

Use configured resampling/conversion policy.

The Playback Engine MUST know which path was selected so Media Session can report accurate information.

---

# 29. Output Initialization

The engine requests output configuration using the negotiated format.

The output backend handles:

- ALSA
- hardware parameters
- device state
- writes
- underruns
- recovery

The Playback Engine should not contain raw ALSA calls.

The final output format should be selected according to the capabilities of the Raspberry Pi audio stack and TANCHJIM BUNNY DSP.

---

# 30. Output Abstraction

The playback engine should not contain device-specific assumptions beyond the output abstraction.

The architecture should allow another audio device to be introduced without rewriting:

- decoder
- queue
- lyrics
- artwork

Audio output configuration should be centralized.

Conceptually:

AudioOutput

    open()
    configure()
    write()
    pause()
    resume()
    drain()
    stop()
    close()

---

# 31. Hardware Output

The BUNNY DSP is the downstream audio device.

The playback engine must treat the audio output subsystem as a hardware abstraction rather than embedding device-specific behavior throughout playback logic.

---

# 32. Prebuffering

Playback begins only after sufficient PCM data is available.

The Playback Engine MUST coordinate:

Decoder

Buffer Manager

Output

Minimum target:

Enough buffered audio to prevent normal startup underruns.

The exact duration MUST be configurable and validated on Pi Zero W hardware.

Starting ALSA immediately after decoding the first tiny PCM block increases the chance of startup underruns.

The Playback Engine SHOULD wait for a suitable buffer threshold.

The threshold SHOULD be based on:

- sample rate
- buffer capacity
- output period size
- measured system behavior

The value MUST be tuned on the Pi Zero W.

---

# 33. Buffer Watermarks

The buffer SHOULD use:

LOW_WATERMARK

TARGET_WATERMARK

HIGH_WATERMARK

Example:

LOW
  ↓
Decoder wakes

TARGET
  ↓
Normal operation

HIGH
  ↓
Decoder pauses

The decoder MUST NOT continuously decode while HIGH_WATERMARK is reached.

---

# 34. Buffering

The playback engine requires bounded PCM buffering.

Buffers absorb differences between:

decoder production rate

and:

audio device consumption rate

If the decoder cannot provide PCM quickly enough:

an audio underrun may occur.

A bounded producer-consumer queue should connect decoding to output.

Decoder:

produces PCM.

Audio output:

consumes PCM.

If the PCM buffer is full:

decoder work should wait or throttle safely.

Never allow PCM buffers to grow indefinitely.

Memory usage must remain appropriate for the Pi Zero W.

Buffer sizes should balance:

- latency
- underrun resistance
- memory usage

Smaller buffers reduce latency.

Larger buffers generally provide greater tolerance to scheduling delays.

Choose a practical buffer configuration through benchmarking on the Pi Zero W.

Do not optimize purely from desktop behavior.

---

# 35. Playback Start

Playback start sequence:

PLAY command
  ↓
Track loaded
  ↓
Decoder ready
  ↓
Output ready
  ↓
Buffer prefilled
  ↓
Output starts
  ↓
State = PLAYING

The Media Session MUST only publish PLAYING once playback is actually ready.

Recommended sequence:

1. Track validated.
2. Decoder initialized.
3. Output configured.
4. PCM buffer created.
5. Decoder starts.
6. Buffer fills to startup threshold.
7. Output starts.
8. State becomes PLAYING.

---

# 36. Pause

When PAUSE is received:

1. Stop/hold output consumption.
2. Preserve playback position.
3. Preserve decoder state where possible.
4. Preserve track identity.
5. Update state to PAUSED.

Pause MUST NOT unnecessarily reopen the file.

Pause MUST NOT destroy the queue.

PAUSE is idempotent.

Pausing an already paused track should not cause an error.

The exact pause behavior depends on the output backend.

The playback engine must expose one consistent logical PAUSED state regardless of backend implementation details.

---

# 37. Pause Buffer Policy

The implementation must define whether output buffers are retained or drained/flushed.

The chosen behavior must preserve correct resume semantics.

---

# 38. Resume

When PLAY is received from PAUSED:

1. Verify output availability.
2. Verify decoder state.
3. Refill buffer if necessary.
4. Resume output.
5. Update state to PLAYING.

If the decoder/output state cannot be resumed safely, the engine MAY reinitialize the necessary components while preserving position.

If buffered PCM remains valid, playback may resume immediately.

If the output was flushed:

PAUSED
    ↓
BUFFERING
    ↓
PLAYING

RESUME on already-playing playback should be harmless.

Resume must verify:

- output device is valid
- buffer contains enough PCM
- decoder is ready

---

# 39. Stop

STOP MUST:

1. Stop output.
2. Flush PCM buffers.
3. Stop decoder.
4. Release track-specific resources.
5. Preserve queue state.
6. Persist playback position if configured.
7. Set state to IDLE.

The library MUST NOT be modified.

Stopping should release or reset:

- decoder
- converter
- PCM buffer contents
- output state

Reusable memory may remain allocated.

---

# 40. Play Track

PLAY_TRACK(track_id) MUST:

1. Resolve track.
2. Stop current track if necessary.
3. Load requested track.
4. Initialize decoder.
5. Configure output.
6. Prebuffer.
7. Begin playback.

If the requested track is already playing, the behavior SHOULD avoid unnecessary reloads.

---

# 41. Seeking

Seeking MUST be asynchronous.

Command:

SEEK(target_position)

Sequence:

PLAYING
  ↓
SEEKING
  ↓
Pause output consumption
  ↓
Flush PCM buffer
  ↓
Seek decoder
  ↓
Decode from target
  ↓
Refill buffer
  ↓
Resume output
  ↓
PLAYING

The old PCM buffer MUST be completely invalidated.

Recommended seek flow:

1. increment generation
2. cancel old asynchronous decoder work
3. flush output
4. clear PCM buffer
5. reset converter
6. seek decoder
7. decode target region
8. refill buffer
9. restart output.

Seek while paused should update the current position without necessarily resuming playback.

Seek while playing should transition through SEEKING and BUFFERING.

---

# 42. Seek Accuracy

Seek position SHOULD be reported relative to actual output playback.

The decoder's seek position and audio output position may temporarily differ during buffering.

The UI SHOULD receive the final synchronized position after the seek completes.

The resulting audible position should be as close as practical to the requested target.

FLAC seeking should use available stream seek points efficiently.

If a FLAC contains a seek table:

use it.

If no efficient seek table is available:

use the decoder's supported seeking mechanism.

Seek operations should avoid unnecessary full-file scanning.

Exact seek accuracy depends on:

- FLAC frame structure
- decoder implementation
- sample position handling

---

# 43. Seek Buffer Invalidation

All PCM decoded before the seek point must be discarded.

The decoder must not publish PCM from the previous position after a seek.

A seek is complete only when the playback engine has established a valid output timeline at the requested position.

---

# 44. Seek Generation

Seek operations may use a monotonically increasing request ID (seek generation counter).

Example:

Decoder is processing:

position 100 seconds.

User seeks to:

200 seconds.

Any PCM associated with the 100-second context becomes stale.

If request 10 is followed by request 11:

request 10 must not overwrite the state established by request 11.

---

# 45. Rapid Seeking

The Playback Engine SHOULD coalesce redundant seek commands.

Example:

User drags:

30s
35s
40s
45s
50s

The engine SHOULD avoid fully rebuilding the decoder pipeline for every intermediate position.

Preferred behavior:

Rapid seek input
    ↓
Latest target retained
    ↓
Seek once when interaction stabilizes

This prevents excessive decoder work on the Pi Zero W.

Where safe:

obsolete commands may be coalesced.

Example:

SEEK 10s

SEEK 20s

SEEK 30s

can become:

SEEK 30s

if earlier seeks have not begun execution.

---

# 46. Seek During Buffering

If SEEK arrives during BUFFERING:

The current buffer should be invalidated.

The decoder should seek to the newest target.

The system should avoid completing the old buffering operation first.

---

# 47. Seek During Stopping

A seek request during STOPPING SHOULD be ignored or queued according to command policy.

It MUST NOT resurrect a stopped session.

---

# 48. Position Clamping

Requested positions should be clamped to valid track boundaries.

Seeking beyond the track duration should resolve safely to the nearest valid endpoint.

Negative seek positions should resolve to zero.

At natural end:

the playback position should resolve consistently to the track's final valid position/end state.

---

# 49. Playback Position

The Playback Engine SHOULD track:

- decoder position
- buffered position
- output position
- current_position
- track_duration
- buffered_position
- source_position
- output_position where available

Output position is authoritative for UI-visible playback time.

The Media Session MUST NOT calculate position independently from unrelated timers.

The UI should primarily consume the authoritative playback position.

---

# 50. Position Accuracy

The engine MUST NOT report decoder position as playback position.

The decoder can be ahead because of buffering.

Playback position should represent what is actually being heard as closely as the output backend permits.

Position should come from the audio clock where available.

Where the audio API provides a reliable consumed-frame position:

use it where appropriate.

The engine must account for PCM currently buffered but not yet consumed.

The public playback position should represent:

the point in the audio stream corresponding to what the listener is currently hearing.

Wall-clock timers can drift from actual audio consumption.

---

# 51. Position Unit

Internal playback position should use:

integer milliseconds

or:

a sufficiently precise sample/frame-based representation

Where accurate timing is required:

sample/frame position is preferable internally.

The UI may receive milliseconds for display.

The UI may issue seek requests in milliseconds.

The engine converts the requested position into the decoder's supported position representation.

---

# 52. Position Monotonicity

During normal playback:

elapsed position should increase monotonically.

Position may jump during:

- seek
- track change
- restart
- previous-track selection

---

# 53. Position Updates

Position updates SHOULD be published approximately 10–30 times per second.

The output clock remains authoritative.

The UI MUST NOT require sample-level position updates.

Do not emit position events excessively.

The UI can read current playback position at its normal update rate.

---

# 54. Pause Position

When paused:

the playback position must remain stable.

---

# 55. Duration

Duration SHOULD be obtained from decoder metadata where reliable.

For known local FLAC files, duration SHOULD be available before playback begins.

The system MUST gracefully handle unknown duration.

For normal FLAC files, duration can generally be determined from:

total_samples / sample_rate

The engine SHOULD obtain duration from the decoder/metadata layer.

If duration cannot be determined:

The engine MUST support an unknown-duration state.

The UI may display:

--:--

rather than an incorrect value.

If duration is temporarily unknown:

the UI should receive an explicit unknown state rather than a fabricated value.

---

# 56. Resume Playback

If resume playback is enabled:

The Playback Engine MAY restore:

- track ID
- position
- queue position
- shuffle state
- repeat state

Resume state MUST be validated before use.

If the referenced file no longer exists, resume MUST fail safely.

---

# 57. Resume Threshold

The system SHOULD avoid saving positions extremely close to:

- track start
- track end

For example, if playback ends normally, the next playback should generally begin at the beginning rather than restoring a position near the end.

The exact threshold SHOULD be configurable.

---

# 58. Next Track

NEXT requests the Queue Manager for the next track.

The Playback Engine MUST NOT calculate queue ordering itself.

NEXT should:

1. invalidate current playback generation
2. stop/flush current PCM
3. determine next track
4. initialize new track
5. buffer
6. resume playback

Queue Manager:

Current
  ↓
Next Track

Playback Engine:

Next Track
  ↓
Prepare
  ↓
Transition
  ↓
Play

A user pressing NEXT should normally skip immediately.

---

# 59. Previous Track

PREVIOUS requests the Queue Manager according to configured previous-track behavior.

Recommended behavior:

If playback position is above the configured threshold:

Restart current track.

Otherwise:

Request previous queue item.

The threshold MUST be configurable.

Many music players interpret PREVIOUS as:

restart current track if playback is sufficiently far into the track.

Otherwise:

go to previous track.

The exact threshold must be explicitly defined.

Previous should normally mean:

the previously played track in playback history.

It should not simply mean:

previous index in the original library.

---

# 60. Natural Track End

When the decoder reaches end-of-stream:

Playback Engine:

1. Drain remaining PCM.
2. Wait for output to finish.
3. Mark track ended.
4. Ask Queue Manager for next action.
5. Prepare next track.
6. Continue playback if appropriate.

The engine MUST NOT abruptly cut the final PCM samples.

Track completion occurs only after the final audible PCM has been consumed.

Only after pending audio is safely handled should:

TRACK_ENDED

be published.

Queue advancement should occur outside the low-level audio output implementation.

---

# 61. Gapless Transition

For compatible tracks:

Current Decoder
        ↓
Current PCM
        ↓
Output
        ↓
Next PCM

The next decoder SHOULD be prepared before the current track finishes.

Where possible:

Decoder A
    ↓
PCM A

Decoder B
    ↓
PCM B

Output:

A A A A A | B B B B B

No intentional silence should be introduced.

Gapless playback may bypass some teardown operations when:

- adjacent tracks are compatible
- output format is unchanged
- the pipeline supports seamless transition

---

# 62. Gapless Boundary

The playback engine must explicitly know where one track ends and the next begins.

Track position must reset to zero for the next track even if the physical output stream remains continuous.

True sample-accurate gapless playback depends on:

- decoder behavior
- FLAC stream metadata
- encoder padding information
- output pipeline behavior

The implementation must not claim true gapless behavior unless these are correctly handled.

---

# 63. Gapless Preparation

If gapless playback is required:

next-track preparation must occur early enough to avoid transition gaps.

---

# 64. Output Reconfiguration During Transition

If Track A and Track B have identical compatible output formats:

The output device SHOULD remain initialized.

If the format changes:

Track A
  ↓
Drain
  ↓
Output Reconfigure
  ↓
Track B

The reconfiguration MUST be performed safely.

The system MUST NOT corrupt or mix incompatible PCM formats.

The engine MUST prevent old-format PCM from reaching the new-format output.

The transition must prevent old-format PCM from remaining queued.

---

# 65. Crossfade

Crossfade is not part of the basic playback engine.

If added later:

the engine will coordinate a mixer rather than directly mixing samples.

---

# 66. Queue Interaction

The Playback Engine MUST treat Queue Manager as the authoritative source of queue ordering.

It requests:

getCurrent()

getNext()

getPrevious()

advance()

restart()

The Playback Engine MUST NOT modify the queue directly except through defined queue commands.

The queue subsystem determines available tracks.

The playback engine decides when to request the next one.

---

# 67. Shuffle Interaction

Shuffle belongs to Queue Manager.

Playback Engine simply receives the selected next track.

Shuffle determines the next logical track.

It should not alter the underlying library order.

Shuffle should maintain a deterministic playback sequence where practical.

The system should maintain sufficient history to make PREVIOUS behave predictably during shuffle.

This ensures that shuffle behavior remains consistent across:

- Full Player
- Mini Player
- Queue
- Hardware controls
- external control

---

# 68. Repeat Interaction

Repeat policies are handled by Queue Manager.

Possible results:

Repeat Off:

Track → Next

Repeat Track:

Track → Same Track

Repeat Queue:

Last Track → First Track

Playback Engine executes the resulting selection.

---

# 69. Queue Ownership

The Playback Engine MUST NOT duplicate the queue.

The Queue subsystem provides:

- current item
- next item
- previous item
- selected item
- queue state

The Playback Engine controls execution.

---

# 70. Queue Modification

The queue may change while playback is active.

The current track must remain valid unless explicitly removed or invalidated.

If the current track is removed from the queue:

playback may continue until completion unless the user explicitly requests stop/remove-and-stop behavior.

An empty queue does not necessarily mean the current track must stop immediately.

The engine should distinguish:

current playback

from:

future queue availability.

---

# 71. Volume Commands

Volume commands SHOULD be routed through AudioManager.

PlaybackEngine MUST NOT directly manipulate device-specific volume APIs.

Example:

Volume Dial
  ↓
Input Backend
  ↓
Media Session
  ↓
AudioManager
  ↓
Output Backend

Volume persistence belongs to the configuration/settings system.

The Playback Engine only applies the currently active volume.

---

# 72. Mute

If mute is supported:

SET_MUTE

should be represented as a playback/audio command.

The underlying volume state should remain recoverable.

Example:

Volume = 60%

Mute

Volume remains:

60%

Output level becomes muted.

Unmute:

60%

Mute must not corrupt playback position or decoder state.

Volume changes must not alter the playback clock.

---

# 73. Volume Processing

Software volume processing should be avoided if hardware/output volume control can provide the required behavior without compromising the desired audio path.

The volume dial should ultimately control the configured volume abstraction.

If digital attenuation is required:

use sufficient precision.

Volume processing must avoid unintended clipping.

---

# 74. Output Device Changes

When the user selects a different output:

1. Pause/hold playback safely.
2. Flush or preserve buffers as appropriate.
3. Close old output.
4. Initialize new output.
5. Validate format.
6. Rebuffer.
7. Resume if configured.

The transition MUST NOT crash the application.

---

# 75. Device Loss

If the active output device disappears:

PLAYING
  ↓
OUTPUT_UNAVAILABLE

The engine SHOULD preserve:

- track ID
- playback position
- queue
- playback intent

When the device returns:

Reinitialize
  ↓
Seek/recover position
  ↓
Buffer
  ↓
Resume

Automatic resume SHOULD be configurable.

Output reports device failure.

Playback Engine transitions to a controlled state.

The application MUST NOT crash.

---

# 76. Device Reconnect

If automatic recovery is enabled:

Device reconnect
    ↓
rediscover capabilities
    ↓
reconfigure output
    ↓
refill buffer
    ↓
resume

The engine MUST use a new valid output session.

After device recovery:

the current track and logical position should be restored where practical.

---

# 77. Decoder Failure

If the decoder fails:

1. Stop decoder.
2. Flush buffers.
3. Record error.
4. Publish error state.
5. Allow user to retry.
6. Allow skipping to next track.

The application MUST remain running.

The engine MAY automatically skip a corrupt track if queue policy allows.

---

# 78. Corrupt File

A corrupt FLAC MUST NOT crash playback.

Behavior:

Decoder detects corruption
  ↓
Playback error
  ↓
Track marked problematic
  ↓
User notification
  ↓
Optional skip

The original file MUST NOT be modified.

The error should include enough information for diagnostics:

- track
- path
- decoder error
- playback session
- approximate position if known

---

# 79. Missing File

If a track's file is missing:

Playback MUST enter a recoverable error state.

The Library SHOULD eventually update its availability state.

Repeated playback attempts MUST NOT occur automatically in a tight loop.

The engine should report:

TRACK_NOT_FOUND

The queue/library should be allowed to reconcile the missing item.

---

# 80. Unsupported Audio Format

If the track's format is unsupported:

Display a user-facing message such as:

"Unsupported audio format."

Developer diagnostics SHOULD expose the actual reason.

Unsupported stream properties must be rejected cleanly.

---

# 81. Storage Removal

If the SD card/file disappears during playback:

Already-buffered PCM may continue.

When the decoder needs unavailable data:

I/O error occurs.

The engine transitions safely.

---

# 82. Error Handling

Errors SHOULD be classified.

## Recoverable

- temporary ALSA underrun
- temporary output retry
- transient buffering starvation

## Non-Recoverable

- corrupt FLAC
- missing file
- unsupported format
- unavailable device
- fatal decoder error

Recoverable error:

attempt recovery.

Non-recoverable error:

stop affected session.

The engine MUST NOT endlessly retry a permanently broken operation.

---

# 83. Error Recovery

Recoverable errors SHOULD attempt recovery once or a small bounded number of times.

The system MUST NOT endlessly retry.

Example:

Output failure
  ↓
Retry initialization
  ↓
Success → Resume

Failure
  ↓
ERROR

---

# 84. Retry Limits

Automatic retries MUST be bounded.

Example:

device recovery:

maximum N attempts

After the limit:

ERROR

The exact retry count should be defined centrally.

---

# 85. Error Categories

Playback Engine MUST support categorized errors:

FILE_NOT_FOUND

FILE_ACCESS_DENIED

DECODER_INIT_FAILED

DECODER_CORRUPT_DATA

UNSUPPORTED_CODEC

UNSUPPORTED_SAMPLE_RATE

UNSUPPORTED_BIT_DEPTH

OUTPUT_INIT_FAILED

OUTPUT_DEVICE_LOST

OUTPUT_FORMAT_FAILED

BUFFER_UNDERRUN

SEEK_FAILED

TRACK_NOT_FOUND

UNKNOWN_PLAYBACK_ERROR

---

# 86. Error Reporting

The Playback Engine should expose structured errors.

Example:

AudioError

- type
- message
- track_id
- session_id
- recoverable
- source_component

UI converts this into user-facing information.

---

# 87. Error Presentation

The UI receives a high-level playback error.

The UI should not need to understand backend-specific error codes.

Bad:

Playback Engine
    ↓
show_error_popup()

Good:

Playback Engine
    ↓
AudioError
    ↓
Application State
    ↓
UI

This preserves architecture boundaries.

Detailed technical information belongs in diagnostics/logging.

The UI may show:

"Unable to play this track."

while logs contain:

UNSUPPORTED_SAMPLE_RATE.

---

# 88. Track Failure Policy

If a track repeatedly fails:

the queue manager may mark it as failed for the current session.

Automatic skipping of failed tracks may be supported.

If enabled:

the behavior must be deterministic.

A failed track should not create an infinite:

LOAD → FAIL → LOAD

loop.

Possible policy:

attempt track

↓

failure

↓

record failure

↓

advance to next valid track.

The exact policy must be configured.

---

# 89. User Feedback

The UI should be informed when:

- track fails
- device unavailable
- output recovers
- track is skipped

---

# 90. Audio Underrun

If the Audio Output Backend reports an underrun:

1. Record event.
2. Attempt output recovery.
3. Inspect buffer state.
4. Refill buffer.
5. Resume.

The engine SHOULD expose an underrun counter.

Repeated underruns SHOULD be visible in Developer Diagnostics.

The engine should attempt controlled recovery where possible.

The playback engine should coordinate recovery without restarting the entire application.

---

# 91. CPU Pressure

The Playback Engine MUST be aware that the Pi Zero W has limited CPU resources.

When CPU pressure increases:

Priority order:

1. Maintain audio continuity.
2. Maintain decoder throughput.
3. Maintain output timing.
4. Reduce background work.
5. Reduce UI frame tier if necessary.
6. Reduce nonessential animations.

The Playback Engine MUST NOT deliberately degrade audio quality simply to maintain visual FPS.

Audio continuity is more important than low-priority background work.

---

# 92. Memory Pressure

Playback MUST use bounded resources.

Do not:

- decode entire tracks
- buffer entire albums
- allocate unlimited PCM
- retain finished decoder state indefinitely
- create per-sample heap allocations

Use:

- streaming
- fixed/reusable buffers
- bounded queues
- explicit lifecycle management

---

# 93. Threading

Recommended execution model:

UI Thread
    ↓
Media Session
    ↓
Playback Command Queue
    ↓
Playback Control Thread
    ├── Decoder Worker
    └── Output Backend

The Audio Output Thread remains latency-sensitive.

Playback control MUST NOT block the output thread.

The Playback Engine should operate independently from the UI/render thread.

The UI thread MUST NOT:

- block waiting for decoder
- block waiting for ALSA
- perform FLAC decoding
- perform long filesystem operations

The engine should aim for:

one authoritative state owner

multiple asynchronous workers

controlled message passing

rather than:

many threads directly modifying shared state.

Do not create unnecessary permanent threads.

Thread count must remain appropriate for the limited CPU.

---

# 94. Real-Time Rules

The output path MUST avoid:

- malloc/free
- filesystem I/O
- blocking mutexes
- artwork processing
- metadata parsing
- logging with blocking I/O
- network requests
- UI calls

The Playback Engine MUST keep expensive work away from the output callback/thread.

The audio thread must never perform:

- image decoding
- artwork extraction
- lyric parsing
- database queries
- filesystem scanning
- UI rendering
- expensive logging
- network operations

The real-time-sensitive portion is primarily:

audio output

The Playback Engine may perform control operations outside the strict real-time path.

The engine MUST avoid forcing the audio callback/output path to wait on:

- UI
- queue database
- artwork
- lyrics
- logging
- network
- configuration writes

---

# 95. Audio Thread

The audio consumption path must remain deterministic.

The real-time-sensitive path should perform only necessary operations such as:

- obtaining ready PCM
- writing PCM
- updating timing state
- handling lightweight synchronization

---

# 96. Decoder Worker

Decoding may run in a worker thread where appropriate.

The decoder worker may:

- read compressed audio
- decode FLAC frames
- convert samples if required
- fill PCM buffers

---

# 97. Object Lifetime

Track-specific resources MUST have explicit ownership.

Typical lifecycle:

Track Loaded
  ↓
Decoder Created
  ↓
Buffers Created
  ↓
Output Connected
  ↓
Playback
  ↓
Track Ends
  ↓
Buffers Drained
  ↓
Decoder Destroyed
  ↓
Track Released

The implementation MUST prevent:

- use-after-free
- double destruction
- stale decoder references
- stale buffer ownership

---

# 98. Track Transition Lifetime

For gapless playback, two track contexts MAY temporarily coexist.

Example:

TrackContext A = PLAYING

TrackContext B = PREPARING

After transition:

TrackContext A = RELEASED

TrackContext B = PLAYING

The implementation MUST make ownership explicit.

---

# 99. Track Context

A TrackContext SHOULD contain runtime information such as:

- Track ID
- decoder instance
- source stream
- audio format
- duration
- decoder position
- buffer state
- error state

It SHOULD NOT contain UI-specific state.

---

# 100. Playback Session

A playback session concept may encapsulate:

- track
- decoder
- converter
- buffer
- generation
- timing state

A new session is created when starting a new playback generation.

The old session must be invalidated before the new session becomes authoritative.

Track changes should make the new session authoritative in a deterministic order.

Old asynchronous work may finish later.

It must detect that its generation is obsolete.

---

# 101. Resource Ownership

Every playback resource must have a clear owner.

When the session ends:

all resources must either be transferred safely or destroyed.

The C++ implementation SHOULD use RAII for resource ownership.

Examples:

DecoderHandle

AudioOutputHandle

PCMBuffer

PlaybackSession

This reduces leak risk during:

- errors
- seek
- track changes
- shutdown

---

# 102. Destruction Safety

The Playback Engine MUST NOT destroy an audio resource while another worker still uses it.

Correct:

signal stop
    ↓
worker exits
    ↓
join
    ↓
destroy resource

Incorrect:

destroy resource
    ↓
worker continues using it

The audio core should preferably use controlled error results rather than exceptions crossing real-time boundaries.

If exceptions are used:

they MUST NOT escape into the audio output thread unexpectedly.

---

# 103. Resource Leaks

Repeated:

play

stop

play

stop

must not increase memory usage indefinitely.

Repeated track switching must not leak:

- decoder contexts
- PCM buffers
- file descriptors
- output handles

Repeated seeking must not create:

- stale decoder threads
- duplicate buffers
- unbounded memory usage

---

# 104. Track Reuse

If the same track is replayed:

the system may reuse resources where safe.

Reuse should never allow stale state to leak into a new playback generation.

---

# 105. Playback Snapshot

The Playback Engine SHOULD publish immutable snapshots.

Example conceptual state:

PlaybackSnapshot

- state
- track_id
- position
- duration
- buffered_position
- sample_rate
- bit_depth
- channels
- codec
- output_device
- replaygain_enabled
- bitperfect
- error
- volume
- repeat_mode
- shuffle_enabled
- buffer_state

The UI consumes snapshots.

The UI MUST NOT mutate them.

A snapshot should represent a consistent point-in-time state.

---

# 106. Synchronization With Media Session

Playback Engine:

Runtime truth
    ↓
Media Session
    ↓
UI

The Media Session SHOULD not issue fake playback state.

For example:

Command:

PLAY

Initial state:

LOADING

Then:

BUFFERING

Then:

PLAYING

The UI should reflect those transitions.

---

# 107. UI Synchronization

The UI MUST consume playback events/state rather than polling low-level audio internals continuously.

Polling every frame is discouraged.

Playback snapshots must be safe to read from the UI thread.

Querying playback state must not block for long periods.

---

# 108. UI Update Rate

Playback position updates do not need to occur at 60 FPS.

A reasonable UI update rate may be:

10–20 updates/sec

while audio itself continues at the device sample rate.

The UI does not need sample-level updates.

Periodic snapshots are sufficient.

---

# 109. Hardware Button Behavior

Physical controls MUST map to the same commands as touchscreen controls.

Example:

Physical Play/Pause
    ↓
TOGGLE_PLAY_PAUSE

Touch Play/Pause
    ↓
TOGGLE_PLAY_PAUSE

No separate playback implementations are allowed.

Hardware input must follow the same model.

Button input should become:

INPUT EVENT

↓

PLAYBACK COMMAND

↓

PLAYBACK ENGINE

---

# 110. Application Backgrounding

FLACHEAD applications MAY coexist while playback continues.

Task Overview, Launcher, Settings, etc. MUST NOT stop audio merely because the music UI is not currently visible.

Playback belongs to the system audio subsystem, not the current screen.

---

# 111. Screen Independence

Music playback MUST continue when navigating:

Music Player
  ↓
Lyrics
  ↓
Settings
  ↓
Launcher
  ↓
Home

Unless the user explicitly stops playback.

---

# 112. Lyrics Integration

Lyrics consume:

- track ID
- playback generation
- playback position
- playback state

The playback engine provides the authoritative position.

Lyrics should not control playback timing.

---

# 113. Artwork Integration

Artwork consumes:

track identity

It does not depend on playback timing.

Artwork loading must remain outside the playback engine's real-time path.

---

# 114. Metadata

The playback engine may consume track metadata needed for:

- duration
- sample rate
- channels
- bit depth

It should not own metadata presentation.

The playback engine may consume track metadata for display state but should not parse metadata itself if a metadata subsystem already exists.

---

# 115. Playback Speed

Initial FLACHEAD architecture assumes:

1.0× playback speed.

Variable speed playback is not required.

If implemented later:

the processing pipeline must explicitly support time stretching.

Do not fake speed control by merely changing sample rate unless that behavior is intentionally desired.

---

# 116. Event System

The engine SHOULD emit events.

Examples:

PLAYBACK_STARTED

PLAYBACK_PAUSED

PLAYBACK_RESUMED

PLAYBACK_STOPPED

PLAYBACK_SEEKED

TRACK_CHANGED

TRACK_COMPLETED / TRACK_ENDED

BUFFERING_STARTED

BUFFERING_ENDED

PLAYBACK_ERROR

OUTPUT_DEVICE_CHANGED

---

# 117. Output Events

The audio output subsystem may generate:

OUTPUT_STARTED

OUTPUT_PAUSED

OUTPUT_DRAINED

OUTPUT_UNDERRUN

OUTPUT_ERROR

DEVICE_DISCONNECTED

---

# 118. Decoder Events

The decoder subsystem may generate:

DECODER_READY

DECODER_EOF

DECODER_ERROR

---

# 119. Buffer Events

The buffer subsystem may generate:

BUFFER_LOW

BUFFER_READY

BUFFER_EMPTY

BUFFER_FULL

---

# 120. Engine Event Processing

The playback engine consumes these events and determines state transitions.

Events from obsolete generations must be ignored.

---

# 121. Event Payloads

Events SHOULD include relevant data.

Playback events should include sufficient identity information where asynchronous consumers exist.

Useful fields:

- track ID
- playback generation
- event type
- timestamp/position

Example:

TRACK_CHANGED

- previous_track_id
- current_track_id
- queue_index
- session_id

---

# 122. Event Ordering

Events MUST be emitted in a deterministic order.

Example:

When starting a new track:

TRACK_LOADING
    ↓
BUFFERING_STARTED
    ↓
BUFFERING_ENDED
    ↓
PLAYBACK_STARTED

The exact public event list may vary, but ordering must remain consistent.

---

# 123. Stale Event Protection

Consumers must be able to reject events belonging to old playback generations.

---

# 124. Logging

Playback logging MUST be lightweight.

Normal playback SHOULD NOT produce excessive logs.

Important events:

- track start
- track completion
- format negotiation
- underrun
- decoder error
- output error
- device disconnect
- device recovery

Useful log events include:

PLAYBACK_LOAD

PLAYBACK_START

PLAYBACK_PAUSE

PLAYBACK_RESUME

PLAYBACK_SEEK

PLAYBACK_STOP

TRACK_END

PLAYBACK_ERROR

AUDIO_UNDERRUN

Never perform expensive synchronous logging from the real-time-sensitive audio callback.

---

# 125. Debug Logging

Developer mode MAY include:

session ID

queue index

buffer occupancy

decoder state

output state

sample rate

format

position

underrun counters

---

# 126. Metrics

The Playback Engine SHOULD expose:

current_state

current_track

queue_index

session_id

position

duration

buffer_fill

underrun_count

decoder_state

output_state

format

Metrics collection MUST be lightweight.

The UI reading diagnostics MUST NOT block the audio thread.

---

# 127. Performance Metrics

The implementation may measure:

- decoder CPU usage
- output latency
- buffer occupancy
- underrun count
- seek latency
- track-open latency
- track-transition latency

---

# 128. CPU Priority

The audio path may require appropriate scheduling priority.

Any real-time scheduling configuration must be used carefully and safely.

The playback engine must remain compatible with the project's overall resource limits.

Sustained playback should not cause unnecessary CPU load.

---

# 129. Decoding Efficiency

FLAC decoding should use efficient library APIs and avoid redundant conversions.

If conversion is required:

perform it once in the audio pipeline.

Resampling should only occur when required.

Avoid unnecessary resampling because it can increase CPU usage and potentially alter the signal.

Channel conversion should only occur when required by the output configuration.

---

# 130. Performance

The playback engine must minimize CPU usage when idle.

No decoder thread should continuously spin while playback is inactive.

Workers should sleep/wait when there is no work.

Continuous polling loops without blocking are discouraged.

When playback is inactive:

the engine should allow the system to remain power-efficient.

Idle audio workers should sleep.

---

# 131. Shutdown

Graceful shutdown:

1. Stop accepting commands
2. Persist playback state
3. Stop playback control
4. Drain/stop output
5. Stop decoder
6. Release buffers
7. Release device
8. Destroy Playback Engine

Shutdown MUST be idempotent.

Calling shutdown twice MUST NOT crash.

Shutdown MUST be deterministic.

No asynchronous worker may publish events after the engine has destroyed the state required to process them.

Application shutdown should allow playback resources to terminate cleanly.

Do not perform unsafe complex operations directly from signal handlers.

---

# 132. Startup

Initialization:

PlaybackEngine created
  ↓
Command system initialized
  ↓
Decoder backend initialized
  ↓
AudioManager available
  ↓
Queue connection established
  ↓
Media Session connected
  ↓
State = IDLE

Audio device initialization SHOULD be independent enough that absence of a DAC does not prevent the rest of FLACHEAD from booting.

If the output device cannot initialize:

publish an audio-output error.

Do not crash the application.

---

# 133. Background Work

Artwork indexing, library scanning, and other background operations must not starve playback.

A large library scan must not interrupt active audio playback.

Playback file reads should remain predictable even while background filesystem operations occur.

---

# 134. File Handle

The active playback file should remain open during playback where practical.

The decoder owns the logical read position.

Other subsystems must not manipulate the active file descriptor.

No unrelated thread should read from the same playback file descriptor.

Track loading should be isolated from unrelated library operations.

---

# 135. Preloading

The next track may be preloaded in future implementations.

Do not preload the next track if it causes excessive RAM or CPU usage.

A future optimization may prepare:

- metadata
- file access
- decoder initialization

But actual PCM predecoding must remain memory-bounded.

---

# 136. Crash Resistance

A failure in:

- decoder
- output
- artwork
- lyrics
- library scanner

MUST NOT directly cause the whole application to crash.

The Playback Engine should isolate failures wherever practical.

---

# 137. Testing

Playback Engine MUST be tested independently from the UI.

---

# 138. Testing: Basic

Required tests:

- play
- pause
- resume
- stop
- next
- previous
- seek
- rapid seek
- track end
- repeat
- shuffle
- corrupt file
- missing file
- unsupported format
- device disconnect
- device reconnect
- output failure
- decoder failure
- graceful shutdown
- volume

Each operation should work from appropriate states.

---

# 139. Testing: State Transitions

Test:

IDLE → PLAYING

PLAYING → PAUSED

PAUSED → PLAYING

PLAYING → SEEKING → PLAYING

PLAYING → STOPPING → IDLE

PLAYING → DRAINING → NEXT

PLAYING → ERROR

ERROR → recovery

---

# 140. Testing: Gapless

Use albums containing intentional track boundaries with no silence.

Test:

Track A → Track B

Verify:

- no artificial pause
- no duplicate samples
- no lost samples
- no decoder reset when unnecessary
- no audible interruption

Tests MUST be performed on the actual Pi Zero W + TANCHJIM BUNNY DSP.

---

# 141. Testing: Seeking

Test:

- forward seek
- backward seek
- beginning
- near end
- exact end
- rapid seeking
- seek while paused
- seek immediately after play
- seek during buffering
- seek while playing
- seek during stopping

No stale audio may play after a successful seek.

---

# 142. Testing: Race Conditions

Test:

- seek while buffering
- next while buffering
- stop while seeking
- pause during track transition
- shutdown during decoder operation
- device disconnect during playback
- device reconnect during recovery
- rapid next/previous
- rapid seek
- rapid commands: PLAY, PAUSE, NEXT, PREVIOUS, SEEK, STOP sent rapidly

Verify:

no race corruption.

---

# 143. Testing: Track Format Change

Switch between different sample rates/bit depths.

Verify:

output reconfiguration is correct.

---

# 144. Testing: Buffer Underrun

Artificially delay decoding.

Verify:

underrun recovery.

---

# 145. Testing: Queue and Policy

Test:

- queue end (repeat off behavior)
- repeat track (same track restarts correctly)
- repeat queue (queue cycles correctly)
- shuffle (deterministic order for a given session seed/state)
- previous track (configured restart/history behavior)

---

# 146. Testing: UI Load

Run playback with:

- animated UI
- album art
- lyrics
- touchscreen interaction

Verify:

audio remains stable.

---

# 147. Testing: Long Playback

Play many tracks continuously.

Play continuously for several hours.

Verify:

- no memory growth
- no stale PCM
- no state corruption
- no increasing latency
- no accumulating threads
- no resource leaks
- no progressive memory leak
- no timing drift that becomes operationally significant
- no playback deadlock

---

# 148. Testing: Resource Cleanup

Repeatedly open and close tracks.

Verify:

file descriptors and memory are released.

---

# 149. Testing: Shutdown

Shutdown during active playback.

Verify:

all resources terminate safely.

---

# 150. Testing: Concurrent Systems

Load artwork while playing FLAC.

Verify:

audio remains stable.

Parse lyrics while playing.

Verify:

audio remains stable.

Stress UI navigation during playback.

Verify:

audio remains stable.

---

# 151. Testing: Hardware

Test playback from the actual target SD card.

Verify:

decoder buffering remains sufficient for sustained playback.

Testing MUST include the Raspberry Pi Zero W.

Test the actual TANCHJIM BUNNY DSP.

---

# 152. Performance Testing

Measure:

- decoder CPU
- playback CPU
- output CPU
- peak memory
- average memory
- buffer occupancy
- underrun count
- startup latency
- seek latency
- track transition latency

Testing MUST include the Raspberry Pi Zero W.

---

# 153. Acceptance Criteria

Playback Engine is considered production-ready when:

- play works reliably
- pause works reliably
- resume works reliably
- stop works reliably
- seeking works without stale PCM
- next/previous work correctly
- queue state remains consistent
- shuffle is delegated correctly
- repeat is delegated correctly
- gapless playback works
- output device loss is recoverable
- corrupt FLAC cannot crash the application
- missing files fail gracefully
- unsupported formats fail gracefully
- audio output remains independent of UI visibility
- hardware and touchscreen controls use the same command path
- playback state is synchronized with Media Session
- no unbounded memory growth occurs
- audio output thread remains free of blocking operations
- Pi Zero W can maintain reliable playback under realistic system load
- one authoritative playback state exists
- all commands pass through the engine
- track transitions are deterministic
- stale PCM is impossible
- stale commands are rejected
- decoder failures are contained
- output failures are contained
- device disconnect is handled
- queue integration is clean
- UI remains independent
- shutdown is safe
- long playback remains stable
- CPU and memory usage are acceptable on Pi Zero W
- FLAC playback works reliably
- playback state transitions are deterministic
- decoder lifecycle is controlled
- audio output is abstracted
- PCM buffering is bounded
- audio timing is authoritative
- playback position is accurate
- UI receives consistent snapshots
- lyrics can synchronize from playback time
- background tasks do not starve playback
- memory usage remains bounded
- idle CPU usage remains low
- long-duration playback is stable
- shutdown is race-safe
- playback generations prevent stale work
- queue transitions work
- EOF handling is correct
- output draining is correct
- underruns are recoverable
- device failure is controlled
- decoder failure is controlled
- unsupported tracks do not crash the application
- actual TANCHJIM BUNNY DSP playback passes testing on the Raspberry Pi Zero W

---

# 154. Implementation Constraints

The AI coding agent MUST NOT:

- implement playback inside a screen class
- create a second queue implementation
- allow UI code to access decoder internals
- access ALSA from UI code
- perform blocking file I/O in the output path
- allocate PCM buffers continuously
- create a new decoder for every UI update
- use a UI timer as the authoritative audio clock
- silently resample audio
- silently apply DSP
- silently change bit depth
- silently change channel configuration
- endlessly retry failed playback
- crash on device disconnect
- crash on corrupt files
- destroy playback state unnecessarily during navigation
- invent undocumented playback behavior

---

# 155. Recommended Interface Boundary

Conceptually:

PlaybackEngine

    play()
    pause()
    togglePlayPause()
    stop()
    playTrack(trackId)
    next()
    previous()
    seek(position)
    getSnapshot()

AudioManager

    setVolume()
    getVolume()
    getDevices()
    selectDevice()
    getDeviceCapabilities()

QueueManager

    getCurrent()
    getNext()
    getPrevious()
    advance()
    getSnapshot()

MediaSession

    publishPlaybackState()
    publishPosition()
    publishAudioFormat()
    publishError()

Decoder

    open()
    decode()
    seek()
    getFormat()
    getDuration()
    close()

AudioOutputBackend

    open()
    configure()
    write()
    pause()
    resume()
    drain()
    close()

These are conceptual responsibilities. Exact C++ APIs may differ, but the separation MUST remain.

---

# 156. AI Coding Agent Rules

The coding agent MUST:

1. Treat the playback engine as the central playback coordinator.
2. Treat the playback engine as the authoritative audio execution layer.
3. Keep UI rendering outside the playback engine.
4. Keep UI code out of the decoder.
5. Keep metadata parsing outside the playback engine.
6. Keep artwork loading outside the playback engine.
7. Keep artwork code out of the audio path.
8. Keep lyrics rendering outside the playback engine.
9. Keep lyric parsing out of the audio path.
10. Keep filesystem indexing outside the playback engine.
11. Keep database access out of the audio thread.
12. Keep filesystem scanning out of the audio thread.
13. Keep network operations out of the audio thread.
14. Use explicit playback states.
15. Use explicit playback commands.
16. Use command-based control.
17. Serialize playback commands.
18. Keep UI out of the audio core.
19. Keep queue policy separate from playback execution.
20. Keep decoder internals separate.
21. Keep ALSA implementation separate.
22. Use playback session IDs or equivalent stale-operation protection.
23. Use playback generations for asynchronous state.
24. Invalidate stale asynchronous work.
25. Never allow stale PCM into a new session.
26. Flush stale PCM after seeking.
27. Flush stale PCM on stop.
28. Flush stale PCM on ordinary track changes.
29. Protect against stale decoder operations.
30. Keep decoder lifecycle explicit.
31. Keep PCM-buffer lifecycle explicit.
32. Keep output lifecycle explicit.
33. Flush buffers correctly during seek.
34. Handle EOF separately from errors.
35. Handle end-of-stream explicitly.
36. Drain output before declaring a track complete.
37. Drain valid PCM at normal EOF.
38. Distinguish drain from flush.
39. Keep queue policy outside the low-level decoder.
40. Keep repeat policy outside the decoder.
41. Keep shuffle policy outside the decoder.
42. Keep repeat behavior deterministic.
43. Keep shuffle behavior deterministic.
44. Maintain useful previous-track history.
45. Use bounded PCM buffers.
46. Never allow unbounded audio buffering.
47. Stream FLAC files from storage.
48. Avoid loading entire FLAC files into RAM.
49. Preserve source audio quality.
50. Preserve source-track timing.
51. Avoid unnecessary resampling.
52. Avoid unnecessary channel conversion.
53. Avoid unnecessary format conversion.
54. Use the audio clock for audible playback position where available.
55. Do not treat decoder progress as audible position.
56. Support pause.
57. Support resume.
58. Support seeking.
59. Handle rapid seeks safely.
60. Handle rapid command sequences safely.
61. Handle decoder errors safely.
62. Handle output errors safely.
63. Handle device disconnection safely.
64. Isolate playback errors.
65. Never crash the entire application because one track fails.
66. Keep recovery attempts bounded.
67. Prevent failed tracks from causing infinite retry loops.
68. Cleanly release decoder resources.
69. Cleanly release audio-output resources.
70. Avoid blocking the UI thread.
71. Avoid long locks in audio-sensitive paths.
72. Never perform expensive work in a real-time-sensitive audio callback.
73. Never perform blocking I/O in the real-time-sensitive audio callback.
74. Never perform image decoding in the audio thread.
75. Never perform lyric parsing in the audio thread.
76. Never perform UI rendering in the audio thread.
77. Never allow artwork or lyrics to delay audio startup.
78. Keep output-device-specific code behind an audio-output abstraction.
79. Keep playback snapshots thread-safe.
80. Keep UI reads non-blocking.
81. Avoid busy loops.
82. Sleep when no work exists.
83. Keep background work from starving audio.
84. Avoid unnecessary preloading.
85. Keep memory bounded.
86. Reuse resources where safe.
87. Avoid unnecessary permanent threads.
88. Keep the Pi Zero W resource limitations in mind.
89. Make shutdown deterministic.
90. Make playback behavior deterministic under rapid user input.
91. Keep all playback lifecycle decisions explicit and deterministic.
92. Keep playback position based on the audio timeline.
93. Test rapid command sequences.
94. Test playback state transitions.
95. Test seek races.
96. Test queue transitions.
97. Test repeat.
98. Test shuffle.
99. Test decoder failure.
100. Test output failure.
101. Test underruns.
102. Test format transitions.
103. Test long-duration playback.
104. Test shutdown races.
105. Test the actual Raspberry Pi Zero W.
106. Test the actual TANCHJIM BUNNY DSP.
107. Do not invent undocumented playback behavior.

---

# 157. Final Architecture

The Playback Engine is the central coordinator:

                         UI
                          │
                    Hardware Input
                          │
                          ▼
                   COMMAND QUEUE
                          │
                          ▼
                 ┌──────────────────┐
                 │ PLAYBACK ENGINE  │
                 │                  │
                 │ State            │
                 │ Session          │
                 │ Track            │
                 │ Commands         │
                 │ Transitions      │
                 │ Error Handling   │
                 └───────┬──────────┘
                         │
          ┌──────────────┼───────────────┐
          │              │               │
          ▼              ▼               ▼
       QUEUE          DECODER         OUTPUT
                         │               ▲
                         ▼               │
                     PCM BUFFER ─────────┘
                                         │
                                         ▼
                                      ALSA
                                         │
                                         ▼
                                TANCHJIM BUNNY DSP

Playback state:

IDLE

→ LOADING

→ BUFFERING

→ PLAYING

→ PAUSED / SEEKING / DRAINING

→ IDLE or NEXT TRACK.

---

# 158. Final Invariants

The central invariant is:

THE PLAYBACK ENGINE IS THE SINGLE AUTHORITY FOR PLAYBACK LIFECYCLE AND TRACK TRANSITIONS.

The second invariant is:

THE PLAYBACK ENGINE OWNS AUDIO EXECUTION, BUT NOT UI OR PRESENTATION.

The third invariant is:

NO UI COMPONENT MAY DIRECTLY MANIPULATE THE DECODER, PCM BUFFER, OR AUDIO DEVICE.

The fourth invariant is:

THE AUDIO THREAD MUST PERFORM ONLY TIME-SENSITIVE AUDIO WORK.

The fifth invariant is:

EVERY ASYNCHRONOUS PLAYBACK OPERATION MUST BELONG TO A VALID PLAYBACK GENERATION.

The sixth invariant is:

STALE GENERATIONS MUST NEVER MODIFY THE CURRENT PLAYBACK SESSION.

The seventh invariant is:

SEEK, STOP, AND TRACK CHANGE MUST INVALIDATE OLD AUDIO DATA.

The eighth invariant is:

NORMAL TRACK COMPLETION MUST DRAIN VALID AUDIO BEFORE ADVANCING.

The ninth invariant is:

PLAYBACK POSITION MUST REPRESENT AUDIBLE TIME, NOT MERELY DECODED TIME.

The tenth invariant is:

QUEUE, REPEAT, SHUFFLE, AND PREVIOUS/NEXT BEHAVIOR MUST BE EXPLICIT AND DETERMINISTIC.

The eleventh invariant is:

ERROR RECOVERY MUST BE BOUNDED AND MUST NEVER CREATE INFINITE RETRY LOOPS.

The twelfth invariant is:

A FAILURE IN ARTWORK, LYRICS, UI, OR METADATA MUST NEVER STOP OTHERWISE VALID AUDIO PLAYBACK.

The thirteenth invariant is:

PCM AND FILE BUFFERS MUST REMAIN BOUNDED.

The fourteenth invariant is:

THE PLAYBACK ENGINE MUST REMAIN RELIABLE UNDER UI LOAD, STORAGE LATENCY, CPU PRESSURE, AND AUDIO DEVICE INTERRUPTIONS.

---

# 159. Final Principle

The Playback Engine should be boring.

It should not contain UI tricks.

It should not contain unnecessary abstractions.

It should not guess.

Its job is simple:

Take the requested track.

Decode it correctly.

Buffer it safely.

Send it reliably to the output device.

Maintain accurate playback state.

Recover when possible.

Never crash.

Never compromise audio quality without an explicit reason.

On the Raspberry Pi Zero W, the Playback Engine MUST prioritize uninterrupted, correct audio over visual performance.

The UI can drop from 60 FPS to 45 or 30 FPS.

The audio stream must keep playing.

---

# 160. Final Responsibility Separation

Open tracks through a controlled playback context.

Decode FLAC through a dedicated decoder layer.

Use bounded PCM buffering.

Keep audio output behind an abstraction.

Maintain an authoritative playback clock.

Handle pause, resume, seek, stop, and end-of-track explicitly.

Use playback generations to reject stale operations.

Flush stale PCM after seeking.

Keep queue/repeat/shuffle decisions outside the decoder.

Keep artwork and lyrics completely independent from the audio path.

Recover from individual track failures safely.

Release all resources deterministically.

Optimize for the Raspberry Pi Zero W rather than desktop hardware.

And always preserve the separation:

PLAYBACK CONTROLLER = PLAYBACK POLICY

PLAYBACK ENGINE = AUDIO EXECUTION

DECODER = COMPRESSED AUDIO → PCM

PCM BUFFER = REAL-TIME AUDIO QUEUING

FORMAT CONVERTER = SOURCE PCM → OUTPUT PCM WHEN REQUIRED

AUDIO OUTPUT = PCM → AUDIO DEVICE

AUDIO CLOCK = AUTHORITATIVE AUDIO TIMELINE

LYRICS = TIMED PRESENTATION DATA

ARTWORK = VISUAL PRESENTATION DATA

UI = USER PRESENTATION

TANCHJIM BUNNY DSP = EXTERNAL AUDIO DEVICE

This architecture makes the playback engine the deterministic control center of FLACHEAD while keeping decoding, buffering, conversion, output, queue management, and presentation cleanly separated.
