# FLACHEAD AUDIO CLOCK AND SEEK

Document

07_AUDIO_CLOCK_AND_SEEK.md

Category

Audio System

Status

Production Specification

Target Hardware

- Raspberry Pi Zero W
- ARMv6
- Raspberry Pi OS
- 2.8" portrait TFT touchscreen
- TANCHJIM BUNNY DSP
- Offline FLAC playback

---

# 1. PURPOSE

This document defines the playback clock and seek architecture for FLACHEAD.

It establishes:

- the authoritative playback clock
- how playback position is calculated
- how decoder timing differs from audible timing
- how ALSA timing is incorporated
- multiple position types (decoder, buffered, ALSA, audible)
- seek as a pipeline-wide invalidation
- playback generation counter for stale event rejection
- monotonic clock requirements
- wall-clock vs audio-clock distinction
- clock drift handling
- seek latency compensation
- UI synchronization
- lyrics synchronization
- pause/resume timing
- track-transition timing
- underrun behavior
- long-playback stability

The primary objective is:

AUDIO TIMING MUST BE DRIVEN BY THE AUDIO OUTPUT PATH, NOT BY THE UI.

Seeking MUST be treated as a coordinated operation across the complete audio pipeline.

---

# 2. CORE PRINCIPLE

The system must distinguish between:

- wall-clock time
- decoder time
- buffered audio time
- ALSA queued time
- audible playback time
- UI display time

These are related but are not identical.

The authoritative user-visible playback position is the position of audio actually being played.

It MUST NOT be derived solely from:

- decoder position
- file read position
- PCM buffer write position
- UI frame timing

---

# 3. AUDIO PIPELINE TIMELINE

The system contains several related positions along the pipeline.

Conceptually:

SOURCE FRAME
    ↓
FLAC DECODER
    ↓
PCM BUFFER WRITE POSITION
    ↓
PCM BUFFER READ POSITION
    ↓
ALSA QUEUED POSITION
    ↓
AUDIO HARDWARE CONSUMPTION
    ↓
AUDIBLE PLAYBACK POSITION

These positions are normally different.

The Playback Engine SHOULD maintain the best available estimate of the position currently being rendered by the audio device.

This position drives:

- playback timer
- progress bar
- elapsed time
- remaining time
- lyrics synchronization

---

# 4. CLOCK AUTHORITY AND SOURCES

The authoritative playback timeline should be derived from the actual audio output position whenever the output backend exposes sufficient timing information.

For ALSA playback, this means using:

- hardware playback position
- delay/available information
- stream frame position
- monotonic timestamps

where appropriate.

The UI must not become the master clock.

## 4.1 Clock Source Hierarchy

Recommended priority:

1. HARDWARE/AUDIO OUTPUT POSITION
2. BACKEND PLAYBACK POSITION
3. MONOTONIC SOFTWARE CLOCK
4. DECODER POSITION FOR DIAGNOSTICS ONLY

## 4.2 Why the UI Cannot Be the Clock

A UI loop might run at:

60 FPS

or:

30 FPS.

Audio may run at:

44100 frames/s.

The UI frame interval is not an accurate representation of audio consumption.

For example:

A frame timer saying:

"one second passed"

does not prove that:

44100 audio frames

were actually rendered.

## 4.3 Decoder Position Is Not the Audible Clock

Decoder position represents the point up to which the decoder has generated PCM.

Example:

Playback position:

60.0 s

Decoder position:

64.5 s

The decoder is therefore:

4.5 s ahead.

This is normal.

The decoder's 105 s position must not be reported as current playback time when the audible position is 100 s.

Decoder position is useful for:

- diagnostics
- buffering
- seek tracking.

It is not the authoritative audible clock.

## 4.4 Buffered Position Is Not the Audible Clock

Buffered position represents audio that has been decoded but has not yet necessarily reached the listener.

It is useful for:

- buffering diagnostics
- seek planning
- underrun analysis

It MUST NOT be displayed as the main playback position.

Likewise:

PCM buffer:

104 s

ALSA queue:

102 s

Audible:

100 s

The user should see approximately:

100 s.

Not:

104 s.

The number of frames currently buffered represents future audio.

It is not the current audible position.

Buffer occupancy describes future audio.

It does not represent elapsed audio.

## 4.5 Output Position

The output layer may have PCM already submitted to ALSA but not yet physically consumed.

Therefore:

PCM read position

is not necessarily identical to:

audible position.

The application knows approximately how many frames have been submitted to ALSA. Call this:

submitted_frames.

This is not necessarily:

played_frames.

If:

submitted_frames = 5000000

and:

queued_frames = 44100

then approximately:

4955900

frames have already been consumed.

The actual implementation must account for ALSA semantics and hardware timing.

## 4.6 ALSA Delay

If ALSA exposes:

number of frames still queued before hardware consumption,

this can be used to estimate audible position.

Conceptually:

audible_position =
submitted_position - queued_delay

The exact implementation depends on ALSA's selected timestamp/position APIs.

## 4.7 Hardware Position

When available, hardware position should be preferred over purely estimated timing.

Hardware position reflects:

what the audio device has consumed.

This is the closest representation of audible playback.

## 4.8 Timestamp + Position

ALSA timing information may provide both:

frame position

and:

timestamp.

These can be used to interpolate playback position between timing updates.

## 4.9 Audio Output Position Preference

Output consumption is the preferred playback-position source.

The exact implementation depends on the audio backend.

If the backend provides:

- consumed frame count
- hardware pointer
- queued-frame count
- latency estimate

those values should be used where appropriate.

---

# 5. POSITION TYPES

The system maintains several distinct position types. Each serves a different purpose.

## 5.1 Decoder Position

The point up to which the decoder has generated PCM.

Useful for diagnostics, buffering analysis, seek tracking.

NOT the audible playback position.

## 5.2 Buffered Position

Audio that has been decoded and placed in the PCM buffer but has not necessarily reached the listener.

Useful for buffering diagnostics, seek planning, underrun analysis.

NOT the user-visible playback position.

## 5.3 ALSA Queued Position

Audio that has been submitted to ALSA but not yet physically consumed by the hardware.

NOT identical to audible position.

## 5.4 Audible Playback Position

The best available estimate of the position currently being rendered by the audio device.

This is the authoritative user-visible playback position.

Drives: playback timer, progress bar, elapsed time, remaining time, lyrics synchronization.

## 5.5 Position Range

For a track with:

TOTAL_FRAMES

the valid logical position is:

0 <= position <= TOTAL_FRAMES.

Any externally supplied position must be clamped to the valid track range.

Negative positions are invalid.

A position beyond the track duration is invalid.

---

# 6. POSITION UNITS AND FRAME-BASED TIMELINE

The most precise logical timeline is represented using audio frames.

## 6.1 Internal Position Units

Internally, playback position SHOULD be represented using audio frames or another sample-accurate unit.

Recommended internal representation:

position_frames

with:

sample_rate

used for conversion to time.

## 6.2 Frame Index

The Playback Session should maintain a logical frame position.

Conceptually:

current_frame

and:

duration_frames.

The frame index should be integer-based.

## 6.3 Time Conversion

Conceptually:

seconds = position_frames / sample_rate

The implementation should retain integer frame positions internally.

Floating-point values should be used primarily for display/calculation where appropriate.

## 6.4 Frame-Based Benefits

For a track with:

sample_rate = 44100

then:

44100 frames = 1 second

88200 frames = 2 seconds

etc.

This avoids floating-point drift during long playback.

## 6.5 Position Units for Frame-Based

For a track with sample rate:

44,100 Hz

frame position:

44,100

represents approximately:

1 second.

## 6.6 Display Time

The UI may display:

HH:MM:SS

or:

MM:SS.

The UI representation must be derived from the internal frame position.

## 6.7 64-Bit Position

Recommended:

uint64_t

for non-negative frame counts.

This gives ample range even for high sample rates and very long tracks.

Frame counters should be large enough for the maximum supported track duration.

Use an integer width that cannot overflow during long playback.

64-bit integers are recommended.

## 6.8 Negative Positions

Playback position should never be negative.

Relative seek operations may use signed deltas:

int64_t

but the resulting absolute frame position must be clamped.

## 6.9 Subsecond Precision

Internal timing should retain enough precision to support:

- lyric synchronization
- accurate seeking
- progress rendering
- pause/resume.

## 6.10 Time Precision

Do not convert:

frame → float seconds → frame

repeatedly.

This can introduce rounding drift.

Prefer:

frame-based internal operations.

If time must be represented outside the frame domain:

use an integer time unit such as microseconds or nanoseconds rather than repeatedly using low-precision floating point.

The selected representation must avoid overflow for realistic track durations.

---

# 7. TIMING MODEL

## 7.1 Monotonic Clock

All internal audio timing should use a monotonic clock.

Do not use wall-clock time such as:

system date/time

for playback timing.

Wall-clock time can change due to:

- NTP
- manual clock changes
- timezone changes
- system synchronization

A monotonic clock only moves forward during normal operation.

It is therefore appropriate for elapsed playback timing.

## 7.2 Monotonic Timestamp

Every relevant playback event should be timestamped using a monotonic source.

Examples:

- playback started
- playback paused
- seek requested
- seek completed
- output started
- underrun occurred
- track ended

## 7.3 Fallback Clock

If exact hardware playback position is unavailable:

use a monotonic software clock anchored to the moment output begins.

## 7.4 Clock Anchor

When playback starts:

record:

clock_anchor

and:

audio_anchor_frame.

## 7.5 Software Position

Conceptually:

current_frame
=
audio_anchor_frame
+
elapsed_monotonic_time × sample_rate.

## 7.6 Buffer Latency

The software estimate must account for audio already queued ahead of the audible position.

## 7.7 Output Queue

If:

10,000 frames

are already queued in the backend:

the audible position is behind the end of those submitted frames.

## 7.8 Internal PCM Buffer

PCM waiting in FLACHEAD's internal ring buffer has not yet been consumed by the output device.

It must not advance the audible playback position.

## 7.9 Decoded Frames

Decoded but unplayed frames must not advance the playback clock.

## 7.10 Interpolation

Suppose:

at time T0:

position = 100.000 s

At time T1:

position = 101.000 s

Between those measurements, the UI can interpolate using monotonic time.

This creates smooth progress without requiring constant ALSA queries.

## 7.11 Interpolation Limit

Interpolation must not continue indefinitely without correction.

Periodically obtain a new authoritative timing measurement.

Otherwise small clock differences can accumulate.

## 7.12 Clock Correction

If estimated position differs from authoritative position:

authoritative timing wins.

Example:

estimated:

200.020 s

hardware-derived:

200.000 s

The system should correct toward:

200.000 s.

## 7.13 No Cumulative Drift

The timing implementation must not repeatedly add:

delta_time

to a floating-point position forever.

That can accumulate error.

Instead, periodically derive position from:

frame count

and:

authoritative output timing.

## 7.14 Floating-Point Position

Floating-point seconds may be used for:

- UI rendering
- animation
- diagnostics

but the canonical audio timeline should remain frame-based.

## 7.15 Double Precision

If seconds must be represented numerically:

double precision should be preferred over float for long-duration timing calculations.

However, frame indices remain preferable for exact audio positions.

## 7.16 Sample-Rate Accuracy

The playback clock must use the actual active output/sample-rate relationship.

## 7.17 Format Change

If sample rate changes:

the timing calculation must change accordingly.

Each track's frame-to-time conversion uses its active sample rate.

## 7.18 VBR

Variable bitrate does not change the audio-frame timeline.

Playback position should be based on decoded audio frames rather than compressed byte position.

## 7.19 Byte Position

Compressed file byte position is not an authoritative playback position.

---

# 8. PLAYBACK START AND CLOCK INITIALIZATION

## 8.1 Playback Start Sequence

When playback begins:

1. Decoder produces PCM.
2. PCM enters the buffer.
3. Output is prepared.
4. Initial PCM is submitted.
5. Hardware starts.
6. Playback clock becomes active.

The system must distinguish:

buffer prepared

from:

audio actually started.

## 8.2 Start Timestamp

The Playback Session should record:

audio_start_monotonic_time.

This can be useful for:

- interpolation
- diagnostics
- latency measurement

## 8.3 Start Position

The start frame may not always be:

0.

Examples:

- seek
- resume
- cue playback

Therefore:

playback_start_frame

must be tracked.

## 8.4 Track Start Position

Normal track start:

frame = 0.

Resume:

frame = saved_frame.

Seek:

frame = requested_frame.

Repeat:

frame = 0.

---

# 9. PLAYBACK GENERATIONS

## 9.1 Generation Concept

Every accepted seek creates a new playback generation.

Example:

Generation 100:

playing at 40 s

Seek to 200 s:

Generation 101.

Only generation 101 may produce new audible PCM.

## 9.2 Why Generations Are Required

Decoder operations can be asynchronous.

Example:

1. User seeks to 30 s.
2. Decoder begins processing.
3. User immediately seeks to 200 s.
4. First decoder operation finishes late.

Without generation checking, 30-second PCM could enter the output path.

Generation checking prevents this.

## 9.3 Generation Invalidation

The generation must change early.

Example:

Current:

Generation 20

Seek begins.

Immediately create:

Generation 21

Any asynchronous work belonging to Generation 20 becomes stale.

## 9.4 Why Generation Comes First

Suppose:

Decoder is still processing old data.

User seeks.

If the old decoder result completes after the seek:

that PCM MUST NOT enter the new playback stream.

Generation invalidation prevents this.

## 9.5 Stale Work

Any asynchronous operation associated with an old generation is:

STALE.

Stale work must not:

- write PCM
- publish position
- change playback state
- complete a track
- trigger a transition

## 9.6 Seek Token

The implementation SHOULD use a request token/generation.

Example:

Seek request 101

If request 102 arrives:

101 becomes stale.

The decoder should not publish results from 101 into the active stream.

## 9.7 Generation + Timing Epoch

The audio system may maintain both:

generation_id

and:

timing_epoch.

They may be represented by one unified playback-generation identifier if that simplifies the implementation.

The important requirement is that stale timing data cannot cross a timeline boundary.

## 9.8 Timing Epoch

Each major timeline discontinuity should create a new timing epoch.

Examples:

- seek
- track change
- stop
- resume after full output teardown

This prevents stale timing calculations from affecting the new stream.

---

# 10. SEEK OPERATIONS

## 10.1 Seek as Pipeline-Wide Invalidation

A seek is not merely:

"tell the FLAC decoder to jump."

It is a timeline transition affecting:

- Playback Engine
- decoder
- PCM buffer
- audio processing
- ALSA output
- playback position reporting
- lyrics synchronization
- UI state

The system must therefore invalidate:

- old decoded PCM
- old buffer contents
- old decoder state
- old timing anchors
- old asynchronous work
- old playback-position events

before allowing the new timeline to become audible.

## 10.2 Seek Sources

Seek requests may originate from:

- touchscreen progress bar
- physical controls
- UI commands
- keyboard/debug commands
- automatic playback logic

All sources MUST go through the same seek-control path.

## 10.3 Seek Request Contents

A seek request contains at minimum:

- target position
- request generation
- seek type
- source of request

Possible sources:

- progress-bar interaction
- hardware button
- keyboard/input event
- application command
- resume logic

## 10.4 Seek API

Conceptually:

seek_absolute(target)

seek_relative(delta)

The exact C++ API may differ.

The implementation should centralize seek handling rather than allowing each input source to manipulate the decoder independently.

## 10.5 Absolute Seek

Absolute seek means:

move playback to:

target position.

Example:

seek_absolute(180s)

means:

play from approximately 3:00.

## 10.6 Relative Seek

A relative seek is:

new_frame =
current_frame + delta_frames.

Example:

current = 100s

seek_relative(+10s)

target = 110s.

Example:

current = 100s

seek_relative(-10s)

target = 90s.

The target must be clamped to the valid track range.

## 10.7 Seek Target Resolution

A seek request should ultimately resolve to:

target_frame

rather than merely:

target_seconds.

If the user requests:

120.5 seconds

the system converts this into a frame position using the active sample rate.

Conceptually:

target_frame =
target_seconds × sample_rate

The result must be clamped to the valid track range.

## 10.8 Seek Target Clamping

Seek targets MUST be clamped.

Minimum:

0

Maximum:

track_duration.

Therefore:

seek(-20s)

becomes:

0s.

And:

seek(duration + 20s)

becomes:

duration.

The decoder must never be asked to seek outside its valid logical range.

## 10.9 End Position

Seeking exactly to the end of a track requires an explicit policy.

Preferred behavior:

seeking to or extremely near the end should transition through the normal track-completion logic rather than producing undefined decoder behavior.

A seek to the exact end of a track requires special handling.

The system may:

- transition directly to draining/completion
- or clamp to the last playable frame

depending on the playback engine design.

It must not attempt to decode invalid frames beyond EOF.

## 10.10 Seek Threshold

The implementation MAY define a small end-of-track threshold.

For example:

if target >= duration

then:

complete current track.

The exact threshold must be defined by implementation testing.

## 10.11 Rounding Policy

The implementation must define whether target positions are:

- rounded
- floored
- ceiled.

For audio seeking:

nearest valid frame or decoder-supported seek point is preferred.

The same target conversion rules should be used regardless of seek source.

Touchscreen and physical controls should eventually resolve to the same frame coordinate.

## 10.12 Seek Precision

The system should preserve as much precision as the source format allows.

For a 44.1 kHz stream:

one frame ≈ 22.676 µs.

UI resolution may be much lower.

That is acceptable.

## 10.13 Seekable vs Non-Seekable

The Playback Engine should know whether the source is seekable.

Normal local FLAC files on the SD card should be seekable.

If a future source is non-seekable:

seek controls should be disabled or handled differently.

---

# 11. SEEK PIPELINE SEQUENCE

## 11.1 Normal Seek Sequence

A normal seek MUST follow a controlled sequence:

1. Receive seek request.
2. Validate target.
3. Create new playback generation.
4. Cancel/invalidate old decoder work.
5. Stop/pause output.
6. Flush PCM buffer.
7. Reset processing state.
8. Seek decoder.
9. Decode from new position.
10. Refill PCM buffer.
11. Reconfigure output if required.
12. Resume playback.
13. Create new timing anchor.
14. Publish new playback position.

## 11.2 Output Flush

Before new audio is played:

old queued audio must be removed from:

- application PCM buffer
- audio processing state
- ALSA output queue

A seek is not complete until stale output has been discarded.

Recommended sequence:

1. invalidate old generation
2. stop/pause output consumption
3. flush/discard application PCM
4. flush/discard ALSA queued audio
5. reset timing state
6. seek decoder
7. decode new PCM
8. restart output

The exact ALSA mechanism depends on the selected output mode.

## 11.3 Why Invalidate First

Generation invalidation must occur before flushing.

Otherwise a racing decoder could produce old PCM between:

flush

and:

generation invalidation.

## 11.4 PCM Buffer Flush

The PCM buffer MUST be cleared.

Old decoded frames are no longer relevant.

The application PCM ring buffer must become logically empty.

After the flush:

buffered_frames = 0.

## 11.5 ALSA Flush

ALSA may contain already-submitted audio.

This audio belongs to the old timeline.

It MUST NOT continue after the seek.

The output layer must therefore use the appropriate ALSA discard/prepare sequence.

The output device must also stop using old queued audio.

## 11.6 Processing State Reset

Any stateful processing stage MUST reset.

Examples:

- resampler
- channel converter
- DSP processing
- filter state

Otherwise old pre-seek state can affect post-seek samples.

## 11.7 Decoder Seek

Only after the old playback generation has been invalidated and downstream state has been flushed should the decoder seek.

The decoder's internal state must be updated accordingly.

## 11.8 Decoder Restart

The decoder should resume from the requested position.

If the decoder requires:

nearest frame

or:

nearest seek point

the resulting actual start position must be tracked.

## 11.9 Do Not Skip Flush

Never optimize seek by simply changing:

decoder position

while leaving:

old PCM

inside the output pipeline.

That causes stale audio.

---

# 12. SEEK ACCURACY AND FLAC

## 12.1 FLAC Seek Granularity

FLAC seeking may involve:

- metadata seek tables
- frame boundaries
- decoder-dependent seeking

Therefore the implementation should distinguish:

requested position

from:

actual decoder starting position.

FLAC seeking may not jump directly to an exact compressed-frame boundary corresponding to the requested sample.

The decoder may seek to a nearby point and decode forward.

## 12.2 Seek Table

FLAC files may contain a:

SEEKTABLE.

When available, it should be used to accelerate seeking.

The Playback Engine does not need to understand FLAC frame indexing.

## 12.3 Seek Without Seek Table

If no useful seek table exists:

the decoder may need to search/scan from a suitable earlier point.

This can increase seek latency.

## 12.4 Post-Seek Accuracy

After seeking, the decoder may begin at a frame before the exact target.

The decoder must discard samples before the exact requested frame when necessary.

Example:

Requested:

120.000 s

Decoder starts:

119.850 s

The output should discard:

119.850 → 120.000 s

and begin audible playback at the target.

## 12.5 Sample-Accurate Seek

Where decoder capabilities permit:

seek should be sample/frame accurate.

This is particularly important for:

- lyrics
- cue tracks
- repeated seeking
- precise progress display.

## 12.6 Decoder Seek + Discard

A common strategy is:

seek to an earlier decoder point

then:

decode forward

discarding samples until:

target_frame.

Only frames at or after the target are placed into the playback buffer.

## 12.7 Pre-Roll

Decoder pre-roll is permitted internally.

Pre-roll audio MUST NOT become audible unless it belongs at or after the requested target.

## 12.8 Seek Start

After seeking:

the first output frame should correspond to the target frame as closely as the decoder permits.

## 12.9 Post-Seek Accuracy Recording

The final output position should be recorded in frame units.

Do not rely solely on:

floating-point seconds.

## 12.10 Bit-Perfect Seek

A direct seek path should preserve source samples.

If no resampling or gain processing exists:

the post-seek PCM should be identical to the source starting at the requested frame.

---

# 13. SEEK LATENCY AND PERFORMANCE

## 13.1 Seek Latency

Seek latency is:

time from user request

to:

new audio actually becoming audible.

This includes:

- decoder seek
- decoding
- buffer refill
- ALSA restart

Seek latency should be measured from:

user seek request

to:

first audible sample from the new target.

## 13.2 Seek Latency Priority

The system should aim for:

fast response

without compromising:

- audio correctness
- synchronization
- stability

## 13.3 Seek Latency Compensation

Seek latency must not be represented as audio playback time.

For example:

if seeking takes:

500 ms,

the track position must not automatically advance by:

500 ms.

## 13.4 Seek Latency Metrics

Measure:

request timestamp

output-resume timestamp

Then:

seek_latency = output_resume - request

This should be monitored during development.

## 13.5 Seek Performance Target

The implementation should aim for a responsive seek experience on the Pi Zero W.

The exact target should be established experimentally.

Correctness takes priority over an artificially low latency target.

## 13.6 Seek Performance

The implementation should minimize:

- file scanning
- decoder startup
- unnecessary buffer filling
- repeated ALSA configuration

The implementation should avoid decoding large amounts of unnecessary audio before reaching the requested target.

## 13.7 Buffer Refill After Seek

The buffer should refill to the normal startup threshold before resuming stable playback.

The exact threshold can be lower for very fast seek response if benchmarking shows it is safe.

## 13.8 Seek Startup Policy

Recommended:

seek
    ↓
decode enough audio to avoid immediate underrun
    ↓
start output
    ↓
continue filling

---

# 14. SEEK WHILE PLAYING AND PAUSED

## 14.1 Seek While Playing

Example:

Current:

75 s

User seeks:

180 s

Flow:

75 s
  ↓
seek requested
  ↓
Generation++
  ↓
stop/discard output
  ↓
flush PCM
  ↓
reset processing
  ↓
decoder → 180 s
  ↓
decode
  ↓
refill
  ↓
output
  ↓
180 s

The user should hear the new target without old audio between them.

## 14.2 Seek While Paused

If paused:

1. invalidate generation
2. flush stale audio
3. reset processing
4. seek decoder
5. refill buffer as appropriate
6. remain paused

The seek MUST NOT accidentally resume playback.

After a paused seek:

the logical paused position becomes:

target_frame.

The output remains paused.

When the user presses play:

audio starts from:

the newly selected target.

## 14.3 Seek During Startup

If playback has not started yet:

the current startup operation should be cancelled or invalidated.

The new seek becomes authoritative.

## 14.4 Seek During Track Transition

If a seek occurs while transitioning between tracks:

the seek request must be associated with the currently active Playback Session.

The state machine must prevent:

old track completion

from overriding:

new seek state.

Track change has priority over the obsolete seek.

The seek operation must be invalidated.

## 14.5 Seek While Buffered

Even if:

application_buffer = 500 ms

the entire old buffered region must be invalidated.

Do not attempt to preserve it across a seek.

## 14.6 Seek While Decoding

If the decoder is currently decoding:

seek should invalidate that work.

Do not attempt to mutate decoder state from another thread simultaneously.

---

# 15. RAPID SEEKING AND COALESCING

## 15.1 Rapid Seeking

Users may perform:

seek forward
seek forward
seek backward
seek forward

before the decoder finishes the first request.

The system MUST handle this safely.

## 15.2 Seek Coalescing

Rapid seek requests MAY be coalesced.

Example:

Requests:

100s
105s
110s
115s

If decoding has not started for the first request:

the system can discard intermediate requests and seek directly to:

115s.

Seek requests may be coalesced when safe.

Only the newest request matters if all previous requests have not become audible.

## 15.3 Latest-Seek-Wins

Recommended policy:

When multiple pending seeks exist:

the newest valid seek target wins.

This avoids unnecessary decoder work.

## 15.4 Seek Request Queue

Do not create an unbounded queue of seek operations.

Bad:

seek 100
seek 101
seek 102
...
seek 1000

with every request being processed independently.

Correct:

retain the latest pending target when safe.

## 15.5 Touchscreen Seeking

Dragging a progress bar may generate many updates.

The UI should not force the decoder to execute hundreds of seeks per second.

## 15.6 Seek Preview

During progress-bar dragging, the UI MAY display the target position without immediately seeking.

Example:

Current:

120 s

Finger dragged to:

240 s

UI shows:

4:00

Actual audio remains at:

2:00

until the seek is committed.

If the UI supports drag-preview:

preview position is NOT playback position.

It should be represented separately.

Example:

playback_position = 120 s

seek_preview_position = 240 s

Only after commit does:

playback_position

become:

240 s.

During dragging:

maintain:

preview_frame.

Playback continues from:

current_frame.

## 15.7 Seek Commit

A committed seek should trigger exactly one authoritative seek operation.

When the user releases the seek control:

convert:

preview_frame

into:

seek_request.

Only then perform the actual seek.

This significantly reduces:

- decoder load
- buffer flushes
- ALSA resets
- CPU usage

## 15.8 Physical Seek Buttons

Physical buttons may use relative seek behavior.

Example:

short press:

+10 seconds

or:

-10 seconds.

Long press behavior may be separately defined by the input system.

For physical buttons:

short press may produce:

small relative seek.

Long press may produce:

continuous seek.

The exact interaction policy belongs to the UI/input specification.

The audio engine only receives normalized seek commands.

## 15.9 Seek Repeat

If a physical button is held:

the input system may generate repeated relative seek events.

The Playback Engine should still coalesce excessive requests where appropriate.

## 15.10 Seek Serialization

Actual decoder seek operations should be serialized.

Do not allow multiple decoder seeks to mutate one decoder instance concurrently.

---

# 16. SEEK CANCELLATION AND CONFLICT RESOLUTION

## 16.1 Seek Cancellation

A seek can be superseded by:

- another seek
- stop
- next
- previous
- application shutdown

The latest higher-priority operation wins.

## 16.2 Operation Priority

Recommended priority:

SHUTDOWN
    >
STOP
    >
TRACK CHANGE
    >
SEEK
    >
NORMAL PLAYBACK

This prevents a stale seek from overriding a stop.

## 16.3 Seek While Dealing with Decoder

The decoder should support cancellation or invalidation.

If true thread cancellation is unsafe or unavailable:

use cooperative cancellation.

The decoder periodically checks:

current_generation == my_generation

If false:

abort current operation.

## 16.4 Decoder Checkpoints

Cancellation checks should occur at reasonable boundaries:

- after file reads
- after decoder frames are produced
- before buffer writes
- before publishing decoder progress

## 16.5 Do Not Force-Kill the Decoder Thread

The implementation should avoid unsafe thread termination.

Do not arbitrarily kill a thread while it may own:

- decoder state
- file handles
- buffers
- synchronization primitives.

## 16.6 Non-Cancellable Decode

If an individual decoder operation cannot be interrupted:

allow it to finish.

Then discard its output if the generation is stale.

Do not block the entire system waiting for cancellation.

## 16.7 Asynchronous Decoder

If decoding is asynchronous:

seek handling MUST be cancellation-aware.

The decoder should periodically check whether:

current_generation == active_generation.

## 16.8 STOP During Seek

STOP invalidates the seek entirely.

No new PCM should be emitted after stop.

STOP invalidates pending seek work.

## 16.9 PAUSE During Seek

Pause should cause the eventual seek completion to preserve:

PAUSED

rather than automatically entering:

PLAYING.

## 16.10 RESUME During Seek

If resume/play is requested during a seek:

the final state should be:

PLAYING

once the new target has been prepared.

---

# 17. SEEK ERROR HANDLING

## 17.1 Seek Error

If seek fails:

the system must not pretend that the target was reached.

Report:

SEEK_FAILED

and transition according to Playback Engine policy.

The old generation must not accidentally resume.

The system should enter an explicit error or recovery state.

## 17.2 Recovery After Seek Failure

Possible policy:

SEEK FAILURE
    ↓
attempt fallback seek
    ↓
if successful → PLAY
    ↓
otherwise → ERROR

The exact number of retries must be bounded.

## 17.3 Invalid Seek

Examples:

NaN target

negative target

target beyond duration

invalid frame conversion

These must be validated before decoder interaction.

## 17.4 Float Input

If the UI provides time as floating-point:

validate it.

Do not allow:

NaN

infinity

negative overflow

to reach frame calculations.

## 17.5 Frame Conversion Validation

Conversion from seconds to frames must use a defined rounding policy.

Possible policies:

floor

nearest

ceil

The chosen policy should be consistent.

For user-facing seeking:

nearest frame is generally appropriate.

---

# 18. SEEK AND LYRICS / UI

## 18.1 Seek and Lyrics

Lyrics synchronization MUST follow the authoritative playback timeline.

On seek:

lyrics position should jump to the corresponding timestamp.

## 18.2 Lyrics Must Not Follow Decoder

If decoder is buffered 5 seconds ahead:

lyrics must NOT jump 5 seconds ahead.

Lyrics use:

audible playback position.

## 18.3 Seek and Album Art

Album artwork is independent of audio seeking.

Seeking MUST NOT reload album artwork.

## 18.4 Seek and Metadata

Seeking MUST NOT reparse track metadata.

Metadata remains associated with the active track.

## 18.5 Seek and Queue

Seeking changes position within the current queue item.

It does not modify queue order.

## 18.6 Seek and Playback State

A seek does not inherently change:

PLAYING → PAUSED

unless the user explicitly requested pause.

For a playing track:

PLAYING

remains:

PLAYING

after successful seek.

For paused:

PAUSED

remains:

PAUSED.

## 18.7 Seek State

The Playback Engine MAY expose an intermediate state:

SEEKING

Example:

PLAYING
    ↓
SEEKING
    ↓
PLAYING

or:

PAUSED
    ↓
SEEKING
    ↓
PAUSED

## 18.8 Seek Position Publishing

During the seek operation:

the UI may display:

SEEKING…

or:

the requested preview position.

But preview state must not be confused with actual audible position.

## 18.9 Position During Seek

While SEEKING:

the old position may remain visible temporarily.

The UI may instead display:

Seeking…

The important requirement is:

do not publish misleading intermediate decoder positions as audible playback.

## 18.10 Position After Seek

Once output actually starts:

publish the new authoritative position.

This becomes the new UI timeline.

Seek is considered complete only when:

- new generation is active
- decoder is positioned
- valid PCM has been produced
- output is ready
- playback has resumed or reached the intended paused state

---

# 19. TRACK TRANSITIONS

## 19.1 Track Change Timing

When switching tracks:

old timing state must be discarded.

New track:

position = 0

unless an explicit start offset exists.

## 19.2 Track End Flow

CURRENT TRACK
    ↓
DECODER EOF
    ↓
DRAINING
    ↓
ALL AUDIO CONSUMED
    ↓
TRACK COMPLETE
    ↓
QUEUE NEXT
    ↓
NEW GENERATION
    ↓
NEXT TRACK BUFFERING
    ↓
PLAYING

## 19.3 Track Completion

Track completion should be determined by actual audio consumption.

Do not end a track merely because:

decoder reached EOF.

The decoder can reach EOF while several seconds of decoded audio remain buffered.

## 19.4 EOF vs Audible End

These are different events.

Decoder EOF:

"No more PCM can be generated."

Audible end:

"All PCM belonging to the track has been consumed."

Track transition should normally occur at audible end.

## 19.5 Draining State

Recommended state:

DRAINING

Meaning:

decoder has reached EOF

but:

audio output still contains remaining track PCM.

## 19.6 Drain Completion

Transition:

DECODING
    ↓
EOF
    ↓
DRAINING
    ↓
all frames consumed
    ↓
TRACK_COMPLETE

## 19.7 End Position

At audible completion:

current_frame = duration_frames

The position should not continue beyond the track duration.

## 19.8 Completion Threshold

Track completion should be determined by actual audio drain, not merely when:

position >= duration.

## 19.9 Final Frame

The final valid frame must be allowed to reach the output.

## 19.10 Completion Race

The UI may report a position very close to the end while the backend still has a small amount of audio queued.

This is normal.

## 19.11 Automatic Track Transition

When the current track reaches audible completion:

the playback engine requests the next queue item.

## 19.12 Gap Between Tracks

The system should minimize unnecessary silence between tracks.

However:

reliability is more important than unsafe aggressive preloading.

## 19.13 Next-Track Preparation

The system MAY prepare:

- file metadata
- file handle
- decoder initialization
- seek information
- album artwork

before the current track ends.

## 19.14 Next-Track PCM Prefetch

Pre-decoding next-track PCM is optional.

If implemented, it MUST be isolated from current-track PCM.

## 19.15 Gapless Playback

For compatible tracks, FLACHEAD should aim to support gapless transitions where technically possible.

Gapless playback requires:

- accurate track-end handling
- correct decoder sample counts
- no accidental silence insertion
- correct output buffering
- precise transition timing.

## 19.16 Gapless Limitation

Some external hardware/DSP paths may introduce unavoidable latency or behavior at stream boundaries.

Therefore the final gapless result must be tested through:

TANCHJIM BUNNY DSP

and the complete output path.

## 19.17 Track Format Change

If Track A and Track B use different:

- sample rates
- channel counts
- sample formats

the output path may require reconfiguration.

## 19.18 Format Change Flow

TRACK A
    ↓
COMPLETE
    ↓
STOP/DRAIN
    ↓
RECONFIGURE OUTPUT
    ↓
TRACK B
    ↓
BUFFER
    ↓
START

## 19.19 Format-Constant Transition

If both tracks share the same output format:

the transition may be optimized to reduce reconfiguration overhead.

## 19.20 Output Reconfiguration

The output device must never interpret PCM using the wrong format.

Format state must be associated with:

current generation.

## 19.21 Sample-Rate Change

Changing:

44.1 kHz

to:

96 kHz

changes the relationship between:

frame count

and:

time.

The new track must establish a new timing anchor.

## 19.22 Channel Change

Changing:

stereo

to:

mono

or another channel layout requires correct output reconfiguration.

Never reuse old channel assumptions.

## 19.23 Next Track

NEXT should create a new playback generation.

Sequence:

CURRENT TRACK
    ↓
NEXT REQUEST
    ↓
INVALIDATE CURRENT GENERATION
    ↓
STOP/DISCARD CURRENT AUDIO
    ↓
SELECT NEXT QUEUE ITEM
    ↓
OPEN TRACK
    ↓
INITIALIZE DECODER
    ↓
BUFFER
    ↓
START

## 19.24 Previous Track

Previous-track behavior depends on playback policy.

Typical policy:

If current position is sufficiently far into the track:

PREVIOUS

may restart the current track.

If near the beginning:

PREVIOUS

selects the previous queue item.

The threshold belongs to playback policy.

## 19.25 Restart Current Track

Restarting the current track is still a timeline transition.

It must:

- invalidate generation
- flush output
- seek to frame 0
- create a new timing anchor.

A restart is effectively:

seek_absolute(0)

It should use the normal seek pipeline.

Do not implement a separate unsafe decoder-reset path.

---

# 20. TRACK IDENTITY AND STATE

## 20.1 Track Identity

Playback position must always be associated with:

track_id.

A position of:

50 seconds

without a track identity is insufficient.

## 20.2 Playback Snapshot

A useful conceptual snapshot is:

PlaybackSnapshot

    generation
    track_id
    state
    current_frame
    duration_frames
    sample_rate
    output_state

## 20.3 Atomic Transition of Snapshot

When a new track becomes active:

publish the new track identity and generation consistently.

Do not expose:

new track ID

with:

old track position.

## 20.4 Queue Selection

The queue manager determines:

which track comes next.

The audio engine should not invent queue ordering.

## 20.5 Audio Engine Responsibility

The audio engine is responsible for:

- loading the selected track
- decoding
- buffering
- output
- timing

The queue engine is responsible for:

- ordering
- repeat
- shuffle
- next/previous selection.

## 20.6 Queue Update During Playback

If the user changes the queue while a track is playing:

the current track should normally continue unless the command explicitly changes playback.

## 20.7 Next After Queue Modification

NEXT should use the queue's current state at request time.

## 20.8 Current Track Removal

If the currently playing track is removed from the queue:

the playback engine should normally continue playing it until:

- explicit stop
- explicit transition
- natural completion

unless product policy states otherwise.

## 20.9 Repeat

Repeat-current-track is a track transition:

TRACK END
    ↓
SELECT SAME TRACK
    ↓
NEW GENERATION
    ↓
SEEK 0
    ↓
PLAY

## 20.10 Repeat Queue

Repeat-queue:

Track A
→ Track B
→ Track C
→ Track A

The queue manager handles selection.

The audio engine performs each transition.

## 20.11 Shuffle

Shuffle ordering belongs to the queue manager.

The audio system should receive:

next selected track.

It should not implement shuffle logic internally.

---

# 21. TRACK TRANSITION RACE CONDITIONS

## 21.1 User-Requested Track Change

A manual NEXT/PREVIOUS command has higher priority than an automatic transition currently being prepared.

## 21.2 Automatic Transition Race

Example:

Track A is ending.

At the same time:

user presses NEXT.

Only one transition should win.

Use serialized transition handling.

## 21.3 Transition Serialization

Track transitions should pass through one logical transition controller.

It ensures:

only one transition is active at a time.

## 21.4 Transition Request

A transition request should identify:

- source
- requested target
- request generation
- reason

Possible reasons:

- NEXT
- PREVIOUS
- EOF
- REPEAT
- QUEUE_CHANGE
- ERROR_RECOVERY.

## 21.5 Transition Priority

A reasonable priority model is:

explicit user command

over:

automatic EOF transition.

The exact policy belongs to the playback state machine.

## 21.6 Stale Transition

If a transition request belongs to an old playback generation:

discard it.

## 21.7 Position Events During Transition

Position events from the old generation must stop being treated as current once the transition begins.

## 21.8 UI During Transition

The UI may show:

Loading…

or:

Changing track…

while the new track is being prepared.

It should not show stale progress as though it belongs to the new track.

## 21.9 Double NEXT

Two rapid NEXT commands should be handled deterministically.

Possible behavior:

NEXT
→ Track B

then:

NEXT
→ Track C.

The queue controller must serialize them.

## 21.10 NEXT During Buffering

If NEXT is pressed while a new track is buffering:

the buffering generation becomes stale.

The next requested track becomes authoritative.

## 21.11 Seek During Buffering

A seek during initial buffering should update the target for the same active track generation or create a new generation according to the command architecture.

Either way:

old target work must not become audible.

---

# 22. TRACK COMPLETION AND ERROR HANDLING

## 22.1 Automatic End Event

Track completion should be generated once.

Do not repeatedly report EOF while waiting for the next transition.

## 22.2 End-of-Track Debounce

A completed generation cannot produce another completion event.

Generation state prevents duplicate completion handling.

## 22.3 Error During Next Track

If the next track fails to open or decode:

the queue manager/playback engine should determine whether to:

- skip it
- stop playback
- report an error
- attempt another queue item.

This behavior must be deterministic.

## 22.4 Error Skip

If configured to skip failed tracks:

the system should invalidate the failed generation and move to the next candidate.

## 22.5 Error Loop Prevention

The system must avoid infinite loops such as:

Track A fails
→ next
→ Track B fails
→ next
→ Track A
→ repeat forever

A finite candidate evaluation policy is required.

## 22.6 Track Open Failure

If a selected track cannot be opened:

the system must not leave stale PCM from the previous track playing indefinitely while claiming the new track is active.

## 22.7 Track Load Atomicity

A new track should become:

ACTIVE

only after enough state has been initialized to safely identify and play it.

## 22.8 Buffering State

After selecting a new track:

BUFFERING

means:

the new track is known

but sufficient PCM has not yet reached the output pipeline.

## 22.9 Buffering Timeout

A production implementation may use a timeout for pathological cases such as:

- unreadable file
- decoder stall
- hardware failure.

Timeouts should trigger diagnostics rather than silently changing tracks.

## 22.10 First Audio Frame

The first audible frame of a track establishes the practical beginning of:

that track's playback timeline.

## 22.11 Timing Anchor After Transition

After output begins:

create a new timing anchor.

Do not extrapolate from:

the previous track.

## 22.12 Track Start Event

The system should publish a track-start event only when the new track has actually entered the intended playback state.

## 22.13 Prepared vs Playing

These are distinct:

PREPARED:

decoder/output ready.

PLAYING:

audio output is actively consuming the track.

## 22.14 Paused New Track

A new track may also be prepared while paused.

It should not be reported as actively playing until output starts.

---

# 23. PAUSE AND RESUME TIMING

## 23.1 Pause

Pause must freeze the logical playback position.

Correct:

PLAYING
    ↓
PAUSE
    ↓
PAUSED

The clock stops advancing because audio consumption stops.

## 23.2 Pause Timestamp

When pausing:

record the authoritative position at the moment the output is actually stopped.

Do not simply use the last UI position.

## 23.3 Pause Accuracy

A pause command may be issued while some audio remains queued.

Therefore the implementation must ensure that the reported paused position corresponds to the actual output state rather than blindly using the command timestamp.

## 23.4 Pause Clock

The playback clock must stop advancing while audio is logically paused.

## 23.5 Backend Pause

If the backend retains queued audio during pause:

the logical playback position must account for whether that queued audio can still become audible.

## 23.6 Recommended Pause

The implementation should ensure that pause produces predictable and prompt audible behavior.

## 23.7 Position After Pause

When paused:

the playback position remains fixed.

UI timer may continue rendering, but it must not advance the underlying audio timeline.

## 23.8 Resume

On resume:

1. Determine paused frame.
2. Ensure output state is valid.
3. Restart hardware playback.
4. Establish a new timing anchor.
5. Continue position from the paused frame.

## 23.9 Resume Anchor

After resume:

new timing anchor

should be established.

Do not continue using a stale pre-pause interpolation anchor indefinitely.

## 23.10 Resume Position

Resume must continue from the last valid logical frame.

## 23.11 No Time Jump

Time spent paused must not be added to playback position.

## 23.12 Pause/Resume Drift

Repeated pause/resume cycles must not gradually shift the playback position.

Test:

pause

resume

pause

resume

many times.

The frame timeline should remain stable.

## 23.13 Position After Stop

After STOP:

the active Playback Session is invalidated.

The UI should receive the appropriate stopped position according to playback policy.

## 23.14 Stop

Stop invalidates the active playback clock.

After stop:

the playback session may report position 0 or the last position according to UI policy.

The underlying active audio clock must no longer advance.

## 23.15 Resume vs Seek

Resume is not necessarily equivalent to seek.

Resume may use persisted playback state.

Seek is an explicit timeline change.

Both still require correct output synchronization.

## 23.16 Saved Resume Position

If FLACHEAD supports persistent resume:

the saved position must be associated with:

track identity.

Never apply a saved position from one track to another.

## 23.17 Resume Validation

Before restoring a saved position:

verify:

- same track
- position within duration
- file still accessible
- metadata/file identity remains valid.

## 23.18 Position Near End

If saved position is extremely close to the end:

the system may start from:

0

or:

the saved position

depending on product policy.

This must be deterministic.

## 23.19 Position After NEXT

When NEXT starts:

new track position:

0

unless the track has an explicit start offset.

## 23.20 Position After Previous

PREVIOUS behavior depends on playback policy.

Common policy:

If current position is sufficiently far into the track:

restart current track.

Otherwise:

go to previous track.

The exact threshold belongs to Playback Engine policy.

---

# 24. UNDERRUN AND XRUN HANDLING

## 24.1 Buffer Underrun

An underrun means:

the hardware needs audio

but:

no PCM is available.

This is both an audio and timing event.

## 24.2 Underrun Effect

During an underrun:

the hardware may stop consuming frames.

Therefore the audio timeline must not continue pretending that normal playback occurred.

## 24.3 No Clock Continuation Through XRUN

Do not simply continue:

position += elapsed_wall_time

through a long underrun.

That would make the reported position run ahead of audible audio.

## 24.4 XRUN Timeline

Before XRUN:

position = 100.0 s

XRUN lasts:

2 s

Audio actually stops.

After recovery:

position should continue from approximately:

100.0 s

not:

102.0 s.

## 24.5 Underrun Handling

After an underrun:

1. Detect XRUN.
2. Recover ALSA.
3. Refill PCM.
4. Re-anchor timing.
5. Resume output.
6. Report diagnostic information.

## 24.6 Audio Starvation

Decoder starvation is different from normal playback timing.

The timing layer should observe:

actual hardware consumption.

The buffering system is responsible for preventing starvation.

## 24.7 Underrun Clock

If output stops because no PCM is available:

pause/freeze the effective playback clock until output resumes.

Do not allow the software clock to report:

"30 seconds"

when only:

"29 seconds"

of audio has actually been consumed because of an underrun.

## 24.8 Underrun Recovery

When output resumes:

create or adjust the clock anchor.

## 24.9 Clock Correction After XRUN

After XRUN recovery:

create a new timing anchor based on the actual recovered output state.

---

# 25. CLOCK DRIFT AND LONG-PLAYBACK STABILITY

## 25.1 Output Clock vs System Clock

The system clock and audio device clock may differ slightly.

Example:

System clock:

44,100.0 timing units/s

Audio device:

44,099.8 or 44,100.2 effective rate.

Over long periods, this can produce drift.

## 25.2 Clock Drift

Small clock differences can accumulate over hours.

FLACHEAD should monitor timing stability during long-playback testing.

## 25.3 No Unnecessary Clock Correction

Do not continuously resample audio simply because two clocks differ by tiny amounts unless there is a demonstrated synchronization requirement.

For normal local music playback, the audio device is the primary clock.

## 25.4 Small Drift

Small clock differences may be corrected gradually at the UI layer.

## 25.5 Audio Correction

The audio stream itself should not be time-stretched merely to make the UI clock appear correct unless an explicit synchronization feature requires it.

## 25.6 No Cumulative Frame Rounding

Repeatedly converting:

frames → milliseconds → frames

can introduce drift.

Internal calculations should remain frame-based where possible.

---

# 26. OUTPUT RECONFIGURATION AND RESAMPLING

## 26.1 Output Reconfiguration

If ALSA must be reconfigured due to a format change:

the old timing state becomes invalid.

After the new device starts:

establish a new timing epoch.

## 26.2 Resampled Output

If the source is resampled:

the relationship between source frames and output frames must be explicitly defined.

## 26.3 Source Timeline

The logical musical position normally remains tied to the source track timeline.

## 26.4 Output Timeline

The output pipeline may use a different sample rate.

Therefore:

SOURCE_FRAME_POSITION

and:

OUTPUT_FRAME_POSITION

may differ.

## 26.5 Resampling Ratio

If:

source_rate != output_rate,

the conversion ratio must be known.

## 26.6 Position Mapping

Conceptually:

output_frames

=

source_frames × output_rate / source_rate.

## 26.7 Roundmapping

Frame mapping must avoid cumulative rounding errors.

## 26.8 Resampling Recommendation

Maintain an exact source-timeline position where possible and use rational/integer conversion for resampled output.

## 26.9 Seek with Resampling

A seek target should first be interpreted on the source timeline.

Then:

source position

→

resampler initialization

→

output position.

## 26.10 Resampler Latency

If the resampler introduces latency:

the playback clock must account for it when determining audible position.

## 26.11 Resampler Reset

A seek resets resampler state.

## 26.12 Resampler EOF

EOF must account for pending resampler output before final completion.

---

# 27. OUTPUT LATENCY AND DEVICE CALIBRATION

## 27.1 Output Latency

The backend may report output latency.

This latency should be incorporated into audible-position estimation.

## 27.2 Device Latency

The TANCHJIM BUNNY DSP may introduce additional processing latency.

Exact latency depends on device configuration and processing.

## 27.3 Calibration

If exact device latency is measurable:

it may be represented as a configured latency offset.

## 27.4 Configuration

Latency compensation must be explicit.

It must not be hidden in arbitrary constants scattered through the code.

## 27.5 Hardware-Specific Offset

If required:

device_latency_frames

may be stored in the audio-device configuration.

## 27.6 No Magic Numbers

Do not add unexplained values such as:

+150 ms

to playback position calculations.

Every offset must have a documented reason.

---

# 28. UI AND LYRICS SYNCHRONIZATION

## 28.1 UI Clock

The UI should consume:

PlaybackPosition

rather than maintain its own independent playback timer.

## 28.2 UI Progress Bar

The progress bar should derive:

current position

from:

PlaybackPosition.

It should not calculate:

current += 1/60

independently.

## 28.3 UI Timer

A UI timer may trigger redraws.

Example:

30 Hz.

Its purpose is:

"refresh the displayed state."

Its purpose is NOT:

"advance audio time."

## 28.4 UI Smoothing

Optional visual interpolation may be used for smooth progress animation.

However:

the interpolated display must not become the authoritative playback state.

UI smoothing should never modify the underlying audio clock.

It is purely presentational.

## 28.5 Lyrics Clock

Lyrics should use the same authoritative playback position as the music player.

No separate lyric timer should exist.

## 28.6 Lyrics Transition

If:

position = 63.2 s

lyrics engine receives:

63.2 s.

The active lyric line is selected from that position.

## 28.7 Lyrics Drift

Lyrics must not be synchronized against UI frame count.

## 28.8 Lyric Offset

If user-configurable lyric offset exists:

adjust the lyric comparison timeline.

## 28.9 Audio Position Remains Authoritative

Changing lyric offset must never alter audio playback position.

## 28.10 Album Art Animations

Visual animations may use:

monotonic UI timing.

They do not control:

audio playback.

## 28.11 Mini Player

The mini player should display the same:

PlaybackPosition

as the full player.

There must be one source of truth.

## 28.12 Full Player

The full player must not create its own playback timer.

It subscribes to:

PlaybackState

and:

PlaybackPosition.

## 28.13 Task Overview

Task overview may show:

playing

paused

track title

position

using the same published playback state.

## 28.14 Position Update Rate

The audio timeline is sample-based.

The UI does not need sample-frequency position updates.

The Playback Engine SHOULD publish position updates at a reasonable control rate.

For example:

10–30 updates per second.

The exact rate should be tuned to UI needs.

## 28.15 Audio Remains Independent

Reducing UI position-update frequency MUST NOT affect actual playback timing.

UI:

30 updates/s

Audio:

44100 frames/s

These are independent.

## 28.16 Position Smoothing

The UI may interpolate displayed progress between authoritative audio-position updates.

However, the underlying audio position remains authoritative.

## 28.17 Position Drift

If UI interpolation drifts from actual audio position:

the next authoritative update should correct it.

The UI must not continuously accumulate error.

## 28.18 Position Query

The playback engine should expose a read-only position query.

Conceptually:

getPlaybackPosition()

## 28.19 Position Result

The result should provide enough information to determine:

- current frame
- sample rate
- duration
- playback state
- generation.

## 28.20 Position Event

The Playback Engine should publish a position update containing enough information for consumers.

Conceptually:

PlaybackPosition

    track_id
    generation
    frame
    duration_frames
    sample_rate
    timestamp
    state

The exact structure may differ.

## 28.21 Position Event Validity

Consumers should ignore position events belonging to:

- old tracks
- old generations
- old timing epochs.

## 28.22 Position Update Frequency

The system should publish position updates often enough for:

- smooth progress
- lyrics
- responsive UI

without creating unnecessary IPC/event overhead.

A starting target of:

10–30 Hz

is reasonable.

Benchmark on the Pi Zero W.

## 28.23 Position Events

Continuous position changes should not necessarily be emitted as high-frequency events.

## 28.24 Position Polling

The UI may poll the playback snapshot at a controlled rate.

## 28.25 Event Flooding

Do not emit thousands of position events per second.

## 28.26 Clock Events

The playback system should generate events for:

- playback started
- playback paused
- playback resumed
- seeking
- seek completed
- underrun
- track completed
- playback stopped.

---

# 29. THREADING AND CONCURRENCY

## 29.1 Audio Engine Independence

The audio engine must not wait for UI consumers.

Bad:

audio thread
    ↓
publish position
    ↓
wait for UI

Correct:

audio thread
    ↓
publish/update timing state
    ↓
continue audio processing

## 29.2 Locking

Position reads from UI threads should avoid blocking the audio thread.

Prefer:

- atomics
- immutable snapshots
- lock-free/low-contention state
- message passing

where appropriate.

## 29.3 Atomic Snapshot

A compact playback-position snapshot can be updated by the audio subsystem and read by UI consumers.

This avoids making the UI acquire a large audio-engine mutex.

## 29.4 Audio Thread Priority

The audio/output thread should have appropriate scheduling priority where safe and supported.

The timing architecture must not introduce unnecessary blocking.

## 29.5 No UI-Dependent Audio

If the UI freezes for:

500 ms

audio must continue playing normally.

The playback clock must continue based on audio output.

## 29.6 No Audio-Dependent UI Loop

If audio continues while the UI is temporarily busy:

the UI should catch up by reading the latest playback snapshot.

It must not attempt to process every missed position update.

## 29.7 Position Snapshot

Recommended conceptual model:

PlaybackSnapshot

    generation
    state
    track_id
    current_frame
    duration_frames
    sample_rate
    timestamp
    buffered_frames
    output_status

Only fields actually required should be implemented.

## 29.8 Buffered Frames

Buffered frames can be exposed for diagnostics.

They should not replace:

current_frame.

## 29.9 Thread Safety

All transition commands must cross thread boundaries through defined synchronization mechanisms.

No component should directly mutate another thread's private state.

## 29.10 Decoder Ownership

One logical owner must control decoder state transitions.

Recommended:

decoder thread owns the decoder object.

Seek commands are sent to it through a command mechanism.

## 29.11 Command Queue

A decoder command queue may contain:

- SEEK
- STOP
- LOAD
- START
- PAUSE
- FLUSH

Commands should be serialized.

## 29.12 Command Coalescing

Multiple obsolete SEEK commands may be coalesced.

STOP and track-change commands should not be silently discarded.

## 29.13 Seek Command Order

Example:

SEEK 100
SEEK 200
SEEK 300

If none has become active:

process only:

SEEK 300.

## 29.14 Output Ownership

Only the audio output controller should directly control:

- ALSA stream state
- output flushing
- output starting/stopping.

## 29.15 Decoder/Output Boundary

The decoder should not directly manipulate ALSA state.

The output controller consumes PCM from the buffer.

This separation simplifies:

- testing
- seeking
- error handling
- timing.

## 29.16 UI Access

The UI reads position through a thread-safe interface.

## 29.17 UI Must Not Modify Clock

The UI must never directly modify:

- frame position
- clock anchor
- output latency
- decoder timing.

## 29.18 Seek Command Delegation

The UI requests a seek.

The playback engine performs the actual timeline modification.

## 29.19 Position Query Thread Safety

Position queries must not race with:

- seek
- stop
- track change
- generation replacement.

## 29.20 Atomic Snapshot Query

A playback-position query should preferably return a consistent snapshot.

## 29.21 Snapshot Content

Example conceptual snapshot:

PlaybackPosition

    generation
    track_id
    state
    frame
    duration_frames
    sample_rate
    elapsed_ms
    remaining_ms.

## 29.22 Stale Snapshot

The UI must tolerate a snapshot becoming stale immediately after it is read.

## 29.23 Generation Check

If a command response references a generation:

the caller should verify that the generation is still active before acting on it.

## 29.24 Clock State Immutability

Only the playback engine/audio subsystem should mutate clock state.

---

# 30. TRACK DURATION AND PROGRESS

## 30.1 Track Duration

Track duration SHOULD be represented in frames where possible.

Example:

duration_frames

This gives a precise basis for:

- seeking
- progress
- end-of-track detection.

Track duration should preferably be obtained from reliable source metadata or decoder information.

## 30.2 Duration Sources

Duration may come from:

- FLAC STREAMINFO
- decoder metadata
- library database
- computed sample count

The audio decoder's actual stream information should be treated as authoritative for playback.

## 30.3 Duration Validation

If the stored duration is unavailable or unreliable:

the playback engine may derive duration from:

TOTAL_FRAMES / SAMPLE_RATE.

## 30.4 Duration Consistency

If metadata says:

duration A

but actual decoded frame count implies:

duration B

the system must not blindly trust a stale external database value.

## 30.5 Zero-Length Track

If a corrupted or unusual track reports:

duration = 0

the playback system must avoid division-by-zero or invalid seek calculations.

## 30.6 Unknown Duration

If duration is not immediately available:

the system may expose:

duration_known = false.

The UI should not assume a valid progress percentage.

## 30.7 Progress Percentage

Progress:

current_frame / duration_frames

should only be calculated when:

duration_frames > 0.

## 30.8 Unknown Duration UI

If duration is unknown:

progress percentage should be considered unavailable.

The UI can display:

elapsed time

without pretending to know:

remaining time.

## 30.9 Remaining Time

Remaining time should be calculated from:

duration_frames - current_frame.

Do not calculate remaining time using:

wall-clock track start time.

## 30.10 Clamp Remaining

If numerical rounding causes:

remaining < 0

return:

0.

## 30.11 Track Completion Near End

Near the end of a track:

rounding must not cause the UI to display an impossible negative remaining time.

---

# 31. QUEUE-LEVEL TIMELINE

## 31.1 Track Timelines

Each track has its own local frame timeline.

## 31.2 Queue Timeline

The queue may additionally have a global playback timeline, but UI track position remains relative to the active track.

## 31.3 Track Offset

For queue-level timing:

track_offset

may represent the total duration of preceding tracks.

## 31.4 Global Position

Conceptually:

global_position

=

track_offset

+

track_position.

## 31.5 Global Clock

A global queue clock is optional.

The primary playback clock remains track-relative.

## 31.6 Queue Transition

The next track should begin only after the completion conditions defined by the playback engine are satisfied.

## 31.7 Gapless Clock Transition

For gapless playback:

the transition must preserve continuous audio timing.

## 31.8 Crossfade

If crossfade is ever implemented:

the clock model must explicitly define whether position represents:

- source track position
- mixed output position
- both.

## 31.9 Initial Crossfade Implementation

Crossfade should not be assumed by the core clock architecture.

## 31.10 No Automatic Crossfade

Normal FLACHEAD playback should remain gapless only where explicitly supported.

---

# 32. RESUME POSITION PERSISTENCE

## 32.1 Database

Playback position may optionally be persisted for resume functionality.

## 32.2 Resume Position

A saved resume position should represent a logical source-track position.

## 32.3 Save Frequency

Do not write resume position to storage on every audio-frame update.

## 32.4 Recommended Save

Persist periodically or at significant playback events such as:

- pause
- stop
- track change
- shutdown.

## 32.5 Write Wear

Frequent storage writes are unnecessary and should be avoided.

## 32.6 Resume After Crash

If resume state exists:

the engine should restore a sensible source position.

## 32.7 Resume Validation

The stored position must be validated against:

- track identity
- file identity/version
- duration.

## 32.8 Changed File

If the file has changed since the position was saved:

the saved position may no longer be valid.

## 32.9 Invalid Resume

Invalid resume state should be discarded safely.

---

# 33. VARIABLE SAMPLE RATE (FUTURE)

## 33.1 Variable Sample Rate

A normal FLAC track has a fixed sample rate.

If a future format supports changing sample rates inside a single logical stream:

the timing architecture must create a new format/timing epoch.

This is outside the initial FLACHEAD scope.

## 33.2 Sample-Rate-Based Timing

For fixed-rate PCM:

duration_seconds =
duration_frames / sample_rate.

The sample rate belongs to the active AudioFormat.

---

# 34. SEEK AND BUFFER GENERATION

## 34.1 Seek and Buffer Generation

Every seek should create a new logical audio generation.

Example:

Track A

Generation 40

Seek:

Generation 41

Second seek:

Generation 42

Only Generation 42 is valid after the second request.

## 34.2 Buffer Generation Tracking

The implementation SHOULD use a request token/generation.

Example:

Seek request 101

If request 102 arrives:

101 becomes stale.

The decoder should not publish results from 101 into the active stream.

---

# 35. CLICK PREVENTION AND AUDIO ARTIFACTS

## 35.1 Seek Click Prevention

Seek transitions can create audible artifacts if processing state is mishandled.

The system must ensure:

- old PCM is fully removed
- processing state is reset
- output starts on valid sample boundaries

## 35.2 Optional Fade

A tiny transition fade MAY be used if required to suppress audible clicks caused by discontinuous seeks.

However:

this is an audio-processing policy.

It MUST NOT be added automatically without testing because it changes PCM.

---

# 36. FILE-LEVEL SEEK LOGIC

## 36.1 File Seek

The decoder may need to seek the underlying file.

This should be done through the decoder abstraction.

The Playback Engine should not directly manipulate:

FILE*

or:

file descriptors

used internally by the decoder.

## 36.2 Seek Index

FLAC metadata may contain seek information.

The decoder should use available seek indexes to accelerate seeking.

---

# 37. ERROR HANDLING (CLOCK AND SEEK)

## 37.1 LOAD Command

LOAD must create a new playback generation.

## 37.2 Seek Failure

If seeking fails:

the old generation must not accidentally resume.

The system should enter an explicit error or recovery state.

Possible policy:

SEEK FAILURE
    ↓
attempt fallback seek
    ↓
if successful → PLAY
    ↓
otherwise → ERROR

The exact number of retries must be bounded.

---

# 38. DIAGNOSTICS

## 38.1 Seek Diagnostics

Debug mode SHOULD expose:

requested position

actual decoder position

actual playback position

generation

seek latency

buffer refill time

seek success/failure

## 38.2 Timing Diagnostics

Debug mode SHOULD expose:

- hardware position
- estimated audible position
- decoder frame
- buffered frames
- ALSA delay
- generation
- timing epoch
- XRUN count
- output state

## 38.3 Clock Debugging

Debug mode should expose:

- source frame
- output frame if available
- buffer frames
- backend queued frames
- estimated latency
- playback state
- generation.

## 38.4 Debug Example

TRACK:

example.flac

SOURCE:

44,100 Hz

SOURCE FRAME:

132,300

BUFFER:

17,640 frames

BACKEND QUEUE:

2,205 frames

STATE:

PLAYING

GENERATION:

27.

## 38.5 Diagnostic Use

These values allow developers to determine whether:

- decoder is ahead
- output is starving
- clock is drifting
- backend latency is excessive.

---

# 39. ACCEPTANCE TESTS

## 39.1 TEST: ABSOLUTE SEEK

Test:

0 s

10 s

middle of track

near end

exact end.

Verify correct audio position.

## 39.2 TEST: RELATIVE SEEK

Test:

+10 s

-10 s

large positive delta

large negative delta.

Verify clamping.

## 39.3 TEST: RAPID SEEKS

Issue many seek commands rapidly.

Verify:

- latest request wins
- stale generations are discarded
- no crashes
- no stale audio
- no deadlocks

## 39.4 TEST: SEEK WHILE PAUSED

Seek repeatedly while paused.

Verify:

- position updates
- playback remains paused
- no stale PCM plays
- resume starts at latest target

## 39.5 TEST: SEEK DURING PLAYBACK

Seek while audio is actively playing.

Verify:

- old audio stops
- new audio starts near target
- playback remains PLAYING

## 39.6 TEST: SEEK DURING TRACK CHANGE

Trigger:

NEXT

and:

SEEK

at nearly the same time.

Verify that the Playback Engine's operation priority rules produce one deterministic result.

## 39.7 TEST: SEEK THEN STOP

Seek.

Immediately stop.

Verify:

seek cannot restart playback after STOP.

## 39.8 TEST: SEEK THEN SHUTDOWN

Seek.

Immediately shut down.

Verify:

no asynchronous decoder task accesses destroyed resources.

## 39.9 TEST: FORMAT CHANGE + SEEK

Play:

44.1 kHz

seek.

Then switch to:

96 kHz.

Verify:

all format state is reset correctly.

## 39.10 TEST: LONG TRACK

Use a long FLAC track.

Seek repeatedly across:

beginning

middle

end.

Verify position accuracy.

## 39.11 TEST: SHORT TRACK

Use a very short track.

Seek beyond its duration.

Verify:

target clamps safely.

## 39.12 TEST: CORRUPTED FILE

Attempt seeking inside a corrupted FLAC.

Verify:

decoder failure propagates safely.

## 39.13 TEST: POSITION DRIFT

Play for an extended period.

Compare:

reported position

against:

actual expected audio timeline.

The system should not accumulate significant timing drift.

## 39.14 TEST: LYRICS

Play a track with timestamped lyrics.

Seek to several lyric boundaries.

Verify:

displayed lyric line corresponds to audible playback position.

## 39.15 TEST: BACKWARD SEEK

Test:

Track A
→ 180 s
→ seek to 30 s.

Verify:

playback begins near:

30 s.

## 39.16 TEST: NEXT

Verify:

Track A
→ NEXT
→ Track B

with:

no audible Track A PCM after Track B begins.

## 39.17 TEST: PREVIOUS

Verify configured previous-track behavior.

Test both:

near beginning

and:

mid-track.

## 39.18 TEST: EOF

Verify:

decoder EOF

does not immediately cut off buffered PCM.

## 39.19 TEST: GAPLESS

Use suitable consecutive tracks.

Verify:

no unnecessary silence is introduced where the complete output chain supports gapless playback.

## 39.20 TEST: FORMAT CHANGE (TRACK)

Test:

44.1 kHz Track
→
96 kHz Track.

Verify:

output reconfiguration is correct.

## 39.21 TEST: ERROR

Attempt to play an invalid/unreadable track.

Verify:

- error is reported
- stale audio does not become associated with the failed track
- recovery behavior is deterministic.

## 39.22 TEST: RACE CONDITIONS

Test combinations:

SEEK + NEXT

SEEK + STOP

NEXT + PREVIOUS

NEXT + NEXT

SEEK + PAUSE

SEEK + PLAY

NEXT + EOF

Verify no stale state becomes audible.

---

# 40. ACCEPTANCE CRITERIA (COMBINED)

## 40.1 Clock Architecture

The timing architecture is production-ready when:

- audio output is the primary playback clock
- monotonic time is used for timing measurements
- frame positions are the canonical timeline
- decoder position is not treated as audible position
- buffered position is not treated as audible position
- UI timing never controls playback
- position updates are independent of rendering FPS
- seek creates a new timing epoch
- pause/resume creates correct timing anchors
- track changes reset timing state
- XRUN recovery resets timing state
- EOF is distinguished from audible track completion
- long playback does not accumulate significant drift
- position snapshots are safe for UI consumers
- stale timing events cannot cross playback generations
- lyrics and progress use the same authoritative playback position

## 40.2 Seek and Position

The seek/position system is production-ready when:

- positions are frame-based internally
- playback position is separated from decoder position
- seek targets are validated
- seek targets are clamped
- every seek invalidates the old generation
- old PCM is flushed
- ALSA queued audio is flushed
- stateful processing is reset
- decoder seeks safely
- stale asynchronous work cannot publish PCM
- rapid seeks are handled
- latest-seek-wins behavior is deterministic
- seek while paused remains paused
- seek while playing remains playing
- stop overrides pending seeks
- track changes override stale seeks
- shutdown cancels all pending work
- lyrics follow audible playback position
- UI timing does not control audio timing
- seek latency is measurable
- long playback does not accumulate position drift

## 40.3 Playback Clock

The playback-clock system is production-ready when:

- internal position is frame-based
- the authoritative clock represents output/audible playback
- decoder progress is not mistaken for playback progress
- buffer occupancy is not mistaken for playback progress
- monotonic time is used for software timing
- wall-clock time is not used as the master clock
- output latency is accounted for where possible
- resampling is accounted for
- seek creates a new timing generation
- pause freezes logical playback position
- resume continues from the correct position
- underruns do not create false position advancement
- track changes reset clock state
- stop invalidates the clock
- EOF does not occur before valid audio is drained
- lyrics use the playback timeline
- UI progress uses the playback timeline
- resume positions use source-track positions
- position queries are thread-safe
- stale snapshots are tolerated
- generation mismatches are handled safely
- no cumulative frame/time rounding drift occurs
- long-duration playback remains accurate
- the clock works correctly across mixed sample rates
- resampled playback remains correctly mapped to the source timeline

## 40.4 Seek and Transition

The seek and transition architecture is production-ready when:

- every seek creates a clean timeline
- every track transition creates a clean generation
- stale decoder work cannot produce PCM
- stale PCM cannot reach ALSA
- stale position events cannot affect the UI
- ALSA output is flushed correctly
- application PCM is flushed correctly
- seek targets are frame-based
- seek targets are clamped
- FLAC seek tables are used where available
- seek latency is measurable
- rapid seeks are handled efficiently
- paused seeks remain paused
- automatic track transitions occur only after audible completion
- EOF is distinguished from audible completion
- format changes are handled safely
- next/previous commands are serialized
- queue logic remains separate from audio decoding
- race conditions are deterministic
- long playback does not accumulate stale state

---

# 41. AI CODING AGENT RULES (MERGED AND DEDUPLICATED)

The coding agent MUST:

## Clock and Timing

1. Treat the audio playback clock as a core audio-system component.
2. Treat audio output as the authoritative playback clock.
3. Use frame counts as the canonical timeline.
4. Use audio frames as the primary exact position unit.
5. Use monotonic time for timing anchors.
6. Use a monotonic clock for software timing.
7. Never use wall-clock time as the master playback timer.
8. Never use wall-clock time for audio progression.
9. Never use UI frame count as the playback clock.
10. Never use file byte position as the playback clock.
11. Never use decoder progress as the audible position.
12. Never use PCM buffer write position as the audible position.
13. Never use UI frame timing as the audio clock.
14. Never advance audio position merely because a UI timer fired.
15. Never use decoder EOF as proof that audio has finished playing.
16. Prefer backend/hardware playback position when available.
17. Distinguish decoder position from audible position.
18. Distinguish buffered position from audible position.
19. Re-anchor timing after seek.
20. Re-anchor timing after resume.
21. Re-anchor timing after track changes.
22. Re-anchor timing after XRUN recovery.
23. Prevent stale timing data from crossing playback generations.
24. Use 64-bit frame counters.
25. Avoid repeated frame → float → frame conversions.
26. Avoid repeated frames → milliseconds → frames conversions.
27. Use exact integer/rational frame calculations where practical.
28. Account for output latency where possible.
29. Account for resampler behavior where required.
30. Keep source-track position separate from output-frame position when resampling.
31. Never add unexplained timing offsets.
32. Keep hardware-specific latency compensation explicit.
33. Document every clock source and timing assumption.

## Position and Progress

34. Treat playback position as an audio-timeline concept.
35. Use frame/sample-based internal positions.
36. Clamp positions to valid track bounds.
37. Clamp remaining time to zero.
38. Freeze logical position during pause.
39. Resume from the correct frame after pause.
40. Do not count paused time as playback time.
41. Do not count seek-processing time as playback time.
42. Do not allow underruns to create false playback advancement.
43. Keep UI timing independent from audio timing.
44. Keep UI smoothing separate from authoritative position.
45. Make position data cheap and safe for UI access.
46. Never block the audio thread waiting for UI consumers.
47. Use the same authoritative PlaybackPosition for lyrics and UI progress.
48. Keep lyrics synchronized to audible playback position.
49. Keep UI progress derived from the playback clock.
50. Do not allow UI code to mutate clock state directly.
51. Expose playback position through a safe snapshot/query interface.

## Generations

52. Treat seek as a timeline discontinuity.
53. Create a new playback generation for every accepted seek.
54. Create a new generation for every track transition.
55. Invalidate old decoder work before it can write new PCM.
56. Associate clock state with playback generation.
57. Reject stale generation data.

## Seek Operations

58. Validate all seek targets.
59. Clamp seek targets to the valid track range.
60. Invalidate the old generation before performing a seek.
61. Flush the PCM buffer on seek.
62. Flush/discard stale ALSA output on seek.
63. Reset stateful processing on seek.
64. Prevent stale asynchronous decoder results from entering the new generation.
65. Keep seek logic centralized.
66. Coalesce excessive pending seek requests when appropriate.
67. Implement latest-seek-wins for rapid pending seeks.
68. Preserve PLAYING/PAUSED state across seeks.
69. Never leave old PCM in the output pipeline after a seek.
70. Never let a stale seek restart playback after STOP or SHUTDOWN.
71. Keep file-level seek logic inside the decoder abstraction.
72. Preserve source PCM whenever the direct path is active.
73. Keep the seek architecture deterministic.
74. Never sacrifice timeline correctness for transition speed.
75. Never expose decoder pre-roll as audible target audio.
76. Use FLAC seek tables where available.
77. Decode forward from a seek point when exact sample positioning requires it.
78. Do not declare track completion before valid audio has drained.

## Track Transitions

79. Never allow old-generation PCM into the current buffer.
80. Never allow old-generation position events to become authoritative.
81. Keep decoder state owned by the decoder thread/controller.
82. Keep ALSA state owned by the output controller.
83. Serialize track transitions.
84. Coalesce obsolete rapid seek requests where safe.
85. Preserve PAUSED state when seeking while paused.
86. Make STOP invalidate all pending seek/transition work.
87. Treat EOF and audible completion as separate events.
88. Do not clear remaining PCM merely because the decoder reaches EOF.
89. Keep queue selection logic separate from audio output logic.
90. Handle sample-rate/channel-format changes explicitly.

## Pause and Resume

91. Invalidate the clock on stop.
92. Do not allow a stale seek to interfere with pause/resume state.
93. Validate resume positions.
94. Avoid excessive resume-position storage writes.

## Testing

95. Test seek behavior under CPU and I/O load.
96. Test race conditions on the real Raspberry Pi Zero W.
97. Measure long-term timing drift on the real Raspberry Pi Zero W.
98. Test mixed sample rates.
99. Test resampled playback.
100. Test pause/resume.
101. Test seek.
102. Test rapid seek.
103. Test underrun.
104. Test EOF.
105. Test track changes.
106. Test lyrics synchronization.
107. Test long-duration timing accuracy.
108. Test actual output-device latency.
109. Benchmark timing behavior on the Raspberry Pi Zero W.
110. Test with the TANCHJIM BUNNY DSP.

---

# 42. FINAL AUDIO CLOCK ARCHITECTURE

The intended timing flow is:

                         TRACK
                           │
                           ▼
                    SOURCE FRAME
                           │
                           ▼
                    FLAC DECODER
                           │
                           ▼
                     PCM PIPELINE
                           │
                           ▼
                       PCM BUFFER
                           │
                           ▼
                      AUDIO BACKEND
                           │
                           ▼
                        ALSA
                           │
                           ▼
                     AUDIO HARDWARE
                           │
                           ▼
                   AUTHORITATIVE CLOCK
                           │
                           ▼
                    PLAYBACK POSITION
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
             UI          LYRICS     DIAGNOSTICS

Decoder:

"How much audio have I generated?"

Buffer:

"How much audio is waiting?"

ALSA:

"How much audio has been submitted/queued?"

Hardware:

"How much audio has actually been consumed?"

PlaybackPosition:

"Where is the listener in the track?"

That final question is what the UI and synchronization systems care about.

Clock authority hierarchy:

1. HARDWARE/AUDIO OUTPUT POSITION
2. BACKEND PLAYBACK POSITION
3. MONOTONIC SOFTWARE CLOCK
4. DECODER POSITION FOR DIAGNOSTICS ONLY

---

# 43. FINAL SEEK ARCHITECTURE

The complete seek path is:

                     USER SEEK
                         │
                         ▼
                  SEEK REQUEST
                         │
                         ▼
                  TARGET FRAME
                         │
                         ▼
                NEW GENERATION ID
                         │
              ┌──────────┴──────────┐
              ▼                     ▼
       INVALIDATE DECODER      STOP OUTPUT
              │                     │
              └──────────┬──────────┘
                         ▼
                  FLUSH PCM BUFFER
                         │
                         ▼
                   FLUSH ALSA
                         │
                         ▼
                RESET PROCESSING
                         │
                         ▼
                  SEEK FLAC DECODER
                         │
                         ▼
                  DECODE FROM TARGET
                         │
                         ▼
                    REFILL PCM
                         │
                         ▼
                   START OUTPUT
                         │
                         ▼
                  NEW TIME ANCHOR
                         │
                         ▼
                 NEW PLAYBACK CLOCK
                         │
                         ▼
                 AUDIBLE PLAYBACK POSITION
                         │
              ┌──────────┼──────────┐
              ▼          ▼          ▼
             UI        LYRICS    PROGRESS

---

# 44. FINAL TRACK TRANSITION ARCHITECTURE

The complete transition path is:

                   TRACK COMPLETE
                         │
                         ▼
                    QUEUE ENGINE
                         │
                         ▼
                  SELECT NEXT TRACK
                         │
                         ▼
                 NEW GENERATION ID
                         │
                         ▼
                 INVALIDATE OLD WORK
                         │
                         ▼
                 DISCARD OLD AUDIO
                         │
                         ▼
                   OPEN NEW FILE
                         │
                         ▼
                 INITIALIZE DECODER
                         │
                         ▼
                    BUFFER PCM
                         │
                         ▼
                   START OUTPUT
                         │
                         ▼
                  NEW TIME ANCHOR
                         │
                         ▼
                    PLAYING

---

# 45. FINAL INVARIANTS

The central clock invariant is:

THE AUDIO OUTPUT TIMELINE MUST REMAIN AUTHORITATIVE EVEN IF THE DECODER, BUFFER, UI, OR SYSTEM CLOCK TEMPORARILY DISAGREES WITH IT.

The second clock invariant is:

THE AMOUNT OF AUDIO DECODED IS NOT THE AMOUNT OF AUDIO PLAYED.

The third clock invariant is:

THE AMOUNT OF AUDIO BUFFERED IS NOT THE CURRENT PLAYBACK POSITION.

The fourth clock invariant is:

THE PLAYBACK CLOCK MUST REPRESENT THE OUTPUT TIMELINE AS CLOSELY AS THE BACKEND PERMITS.

The fifth clock invariant is:

INTERNAL POSITION SHOULD BE FRAME-BASED TO AVOID CUMULATIVE ROUNDING DRIFT.

The sixth clock invariant is:

PAUSED TIME MUST NOT ADVANCE THE AUDIO TIMELINE.

The seventh clock invariant is:

SEEK TIME MUST NOT ADVANCE THE AUDIO TIMELINE.

The eighth clock invariant is:

UNDERRUN TIME MUST NOT BE COUNTED AS SUCCESSFULLY PLAYED AUDIO.

The ninth clock invariant is:

EVERY NEW PLAYBACK GENERATION MUST HAVE VALID CLOCK STATE.

The tenth clock invariant is:

LYRICS AND UI PROGRESS MUST READ FROM THE SAME AUTHORITATIVE PLAYBACK TIMELINE.

The eleventh clock invariant is:

TRACK COMPLETION OCCURS ONLY AFTER THE VALID AUDIO PIPELINE HAS BEEN FULLY DRAINED.

The central seek invariant is:

A SEEK IS A COMPLETE AUDIO-TIMELINE TRANSITION.

It is not merely a decoder operation.

Every component that can contain old audio must be invalidated or flushed before the new timeline becomes audible.

The second seek invariant is:

OLD GENERATION PCM MUST NEVER BECOME AUDIBLE AFTER A SUCCESSFUL SEEK TO A NEW GENERATION.

That invariant is more important than minimizing seek latency by a few milliseconds.

The third seek invariant is:

EVERY AUDIBLE TIMELINE DISCONTINUITY MUST CREATE A NEW, AUTHORITATIVE TIMING ANCHOR.

The fourth seek invariant is:

QUEUE SELECTION, DECODING, BUFFERING, OUTPUT, AND TIMING MUST REMAIN SEPARATE RESPONSIBILITIES CONNECTED THROUGH EXPLICIT STATE AND COMMAND BOUNDARIES.

The fifth seek invariant is:

NO AUDIO FROM AN INVALIDATED PLAYBACK GENERATION MAY EVER REACH THE USER.

The sixth seek invariant is:

PCM reaching ALSA MUST BELONG TO THE CURRENT PLAYBACK GENERATION.

The seventh seek invariant is:

A position event MUST NOT be interpreted as current if its generation or track identity is obsolete.

The eighth seek invariant is:

A decoder MUST NOT continue producing audible output after its generation has been invalidated.

The ninth seek invariant is:

After a successful seek or track transition: no audio from the previous timeline may be audible after the new timeline begins.

---

# 46. FINAL RESPONSIBILITY SEPARATION

AUDIO OUTPUT BACKEND

=

REPORTS OR ENABLES DETERMINATION OF ACTUAL OUTPUT PROGRESS

PLAYBACK CLOCK

=

CONVERTS OUTPUT PROGRESS INTO AUTHORITATIVE TRACK POSITION

PLAYBACK ENGINE

=

OWNS CLOCK STATE AND GENERATIONS

LYRICS ENGINE

=

READS TRACK POSITION

UI

=

DISPLAYS TRACK POSITION

RESUME SYSTEM

=

PERSISTS VALID SOURCE-TRACK POSITIONS

DECODER

=

PROVIDES SOURCE FRAME INFORMATION FOR DECODE/SEEK OPERATIONS

BUFFER

=

PROVIDES FUTURE AUDIO, NOT AUTHORITATIVE POSITION

QUEUE ENGINE

=

CONTROLS ORDERING AND SELECTION

This architecture ensures that FLACHEAD's progress display, lyrics synchronization, seeking, pause/resume behavior, queue transitions, and track-completion logic all operate from a single consistent audio timeline rather than independent clocks that can gradually drift apart.
