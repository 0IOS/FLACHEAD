# FLACHEAD PCM Buffering Architecture

Document

04_PCM_BUFFERING.md

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

This document defines the PCM buffering layer between the FLAC decoder and the audio output backend. It establishes buffer ownership, sizing, streaming semantics, startup and steady-state behavior, underrun prevention, latency budgets, memory constraints, seek and track-change invalidation, generation tracking, threading, and error handling.

The primary objective is:

MAINTAIN REliable UNINTERRUPTED PLAYBACK ON THE RASPBERRY PI ZERO W WHILE KEEPING UNNECESSARY AUDIO LATENCY AND MEMORY USAGE LOW.

The PCM buffer is the bounded real-time handoff between the FLAC decoder (producer) and audio output (consumer).

The buffering system exists to absorb timing differences between:

- SD-card/file I/O latency
- FLAC decoding throughput
- operating-system scheduling
- CPU contention
- audio output consumption
- temporary system load
- UI activity

The system MUST provide:

- continuous PCM delivery
- bounded memory usage
- protection against decoder/output timing differences
- underrun detection
- controlled backpressure
- safe seeking
- safe track transitions
- predictable pause/resume behavior
- low CPU overhead

The system MUST remain independent of:

- UI rendering
- touchscreen input
- artwork
- lyrics rendering
- music-library presentation
- launcher state
- task overview

The buffering system MUST NOT:

- load complete tracks into memory
- grow without limits
- block the UI
- depend on UI frame timing
- perform unnecessary PCM copies
- allow stale PCM to survive a seek
- decode FLAC
- perform metadata parsing
- manage the playback queue
- render UI
- control album artwork
- synchronize lyrics
- determine playback policy

---

# 2. Position in the Audio Pipeline

The intended pipeline is:

SD CARD
    ↓
FILE READ BUFFER
    ↓
FLAC DECODER
    ↓
FORMAT CONVERSION (if required)
    ↓
PCM RING BUFFER
    ↓
AUDIO OUTPUT BACKEND
    ↓
ALSA
    ↓
TANCHJIM BUNNY DSP
    ↓
HEADPHONES

The PCM buffer is therefore the boundary between:

PRODUCER
(the decoder)

and:

CONSUMER
(the audio output pipeline).

---

# 3. Core Principle

The PCM buffer exists to absorb timing differences between:

- FLAC decoding
- filesystem I/O
- CPU scheduling
- ALSA output
- hardware consumption

It is NOT intended to store entire tracks.

The decoder produces PCM. The output backend consumes PCM. The buffer absorbs short-term timing differences between those two operations.

Conceptually:

DECODER
    ↓
PCM PRODUCER
    ↓
BOUNDED PCM BUFFER
    ↓
PCM CONSUMER
    ↓
AUDIO OUTPUT

---

# 4. Ownership

The PCM Buffer Manager is an audio-system component.

It MUST NOT belong to any screen.

The Playback Engine controls:

- PLAY
- PAUSE
- STOP
- SEEK
- NEXT
- PREVIOUS

The buffer responds to lifecycle changes.

The buffer MUST NOT decide queue behavior.

The PCM Buffer Manager owns the memory used for its ring buffer.

Decoder writes through its controlled interface.

Output reads through its controlled interface.

Neither decoder nor output should directly manipulate:

- read_index
- write_index
- occupancy

unless the implementation explicitly defines them as atomic/shared state.

The buffer owns its internal storage. Callers must not directly manipulate ring positions.

---

# 5. Responsibilities

The PCM Buffer Manager is responsible for:

- storing decoded PCM temporarily
- providing PCM to the audio output
- tracking buffer occupancy
- preventing unnecessary decoder work
- detecting low-buffer conditions
- detecting high-buffer conditions
- flushing stale PCM
- handling track transitions
- supporting seek invalidation
- coordinating producer/consumer synchronization
- exposing buffer diagnostics

It is NOT responsible for:

- FLAC decoding
- playlist management
- metadata
- lyrics
- UI rendering
- audio-device configuration
- playback policy

---

# 6. Ring Buffer Design

The preferred application-level PCM buffer is a ring buffer.

A ring buffer is preferred because it provides:

- bounded memory usage
- predictable allocation behavior
- efficient sequential writes
- efficient sequential reads
- low synchronization overhead
- natural streaming behavior
- no repeated large reallocations
- natural producer/consumer semantics

The buffer should be allocated once and reused.

The implementation SHOULD use a fixed-capacity circular buffer.

Conceptually:

+------------------------------------------------+
|                                                |
|   consumed   |   available PCM   |   free      |
|                                                |
+------------------------------------------------+
                ↑                  ↑
              read               write

When the write position reaches the end:

write position → beginning

When the read position reaches the end:

read position → beginning

The physical memory remains fixed.

The ring buffer must distinguish:

- empty
- partially occupied
- full

The implementation must avoid ambiguous read/write positions.

The implementation must use explicit state/accounting so that:

read_index == write_index

cannot ambiguously represent both empty and full.

Possible approaches:

- stored count
- one reserved slot
- monotonic counters

A strong implementation option is to maintain logical producer/consumer counters rather than only wrapped indexes. For example:

write_count
read_count

Then:

available = write_count - read_count

The actual physical index can be:

counter % capacity

This simplifies occupancy reasoning.

Counters MUST use sufficiently wide integer types.

The implementation MUST prevent overflow-related corruption during long playback.

A multi-hour track must not cause frame counters to overflow.

---

# 7. Buffer Capacity and Sizing

The buffer capacity MUST be configurable.

It MUST be large enough to absorb normal decoder and filesystem latency.

It MUST NOT be unnecessarily large.

The correct value MUST be determined through Pi Zero W benchmarking.

The implementation SHOULD begin with a conservative bounded capacity and allow later tuning.

Buffer size depends on:

- sample rate
- channels
- bytes per sample
- expected decoder latency
- SD-card latency
- output scheduling
- system load

For PCM:

bytes_per_second =
    sample_rate × channels × bytes_per_sample

Required buffer memory can then be estimated from:

buffer_bytes =
    bytes_per_second × buffer_duration

Example:

44.1 kHz / 2 channels / 16-bit

PCM consumption is approximately:

44100 × 2 × 2 = 176400 bytes/second

A one-second buffer would therefore require approximately 176 KB.

The final production value MUST be benchmarked rather than blindly selected from this example.

---

# 8. Multiple Formats

The buffer MUST account for different source formats.

Examples:

- 44.1 kHz / 16-bit / stereo
- 48 kHz / 24-bit / stereo
- 96 kHz / 24-bit / stereo
- 192 kHz / 24-bit / stereo

Higher sample rates and bit depths consume more memory per second.

The implementation SHOULD size the buffer according to PCM byte rate rather than assuming one fixed format.

---

# 9. Buffer Unit (Frames)

The buffering system SHOULD reason primarily in PCM frames rather than raw bytes.

For stereo:

one frame contains:

LEFT sample + RIGHT sample

For N channels:

1 PCM frame contains N samples.

Buffer accounting SHOULD use frames where practical.

Conversion to bytes:

bytes_per_frame =
    channels × bytes_per_sample

The buffer SHOULD internally represent capacity in bytes or PCM frames.

PCM frames are often preferable because:

one frame = one sample for every channel

The implementation MUST avoid confusing:

- samples
- channels
- frames
- bytes

Buffer capacity should be represented primarily in AUDIO FRAMES.

The byte size can be derived from:

capacity_frames × channels × bytes_per_sample

for the current format.

---

# 10. Read Position and Write Position

The read position identifies the next PCM frame consumed by the output.

When frames are consumed:

read_position += frames_consumed

When the read position reaches capacity:

read_position wraps to zero.

The write position identifies where new decoded PCM should be inserted.

When frames are written:

write_position += frames_written

When the write position reaches capacity:

write_position wraps to zero.

When either position reaches the end of the buffer:

it wraps to the beginning.

---

# 11. Occupancy and Free Space

The buffer MUST track how much PCM is currently available.

Conceptually:

occupancy = written_frames - consumed_frames

with appropriate wraparound handling.

Occupancy MUST never exceed capacity.

Occupancy MUST never become negative.

Free space is:

free_frames = capacity_frames - occupancy_frames

The decoder MUST never write more PCM than free space allows.

---

# 12. Contiguous Regions and Wraparound

A ring buffer may contain:

Region A: read → end
Region B: beginning → write

A read/write operation MAY therefore require two segments.

The implementation SHOULD expose a two-segment operation where practical.

This can reduce unnecessary temporary copying.

When the write pointer reaches the end:

it wraps to the beginning.

The same applies to the read pointer.

The same applies to reads.

Verify that:

- no bytes are skipped
- no bytes are duplicated
- ordering is preserved

---

# 13. Zero-Copy Consideration

True zero-copy is desirable but not mandatory.

The implementation SHOULD prefer:

Decoder → ring buffer

and:

ring buffer → output

without intermediate temporary PCM buffers.

If the decoder library requires its own temporary memory, that is acceptable.

Do not create complicated zero-copy infrastructure unless measurements show that copies are a meaningful Pi Zero W bottleneck.

A well-optimized bounded memory copy may be preferable on the Pi Zero W.

The implementation may use segmented read/write regions to avoid unnecessary temporary buffers.

---

# 14. Producer and Consumer Model

The decoder is the producer.

The audio output is the consumer.

Conceptually:

             PRODUCER
                │
                ▼
        ┌─────────────────┐
        │    PCM RING      │
        │     BUFFER       │
        └─────────────────┘
                │
                ▼
             CONSUMER

Decoder: writes PCM

Audio output: reads PCM

Neither side should need to wait for the other under normal operating conditions.

The producer and consumer MUST be independently schedulable.

---

# 15. SPSC Design

The initial architecture SHOULD preferably use:

one PCM producer
and:
one PCM consumer

This greatly simplifies synchronization on the Pi Zero W.

A single-producer/single-consumer ring buffer MAY use lock-free indexes.

This is potentially appropriate because:

Producer: decoder worker
Consumer: audio output thread

However, lock-free code MUST only be used if its correctness is clearly established.

A simple mutex or lightweight synchronization mechanism is preferable to fragile lock-free code when performance measurements show no meaningful difference.

SPSC can minimize:

- locks
- contention
- synchronization overhead

This is especially valuable on the Pi Zero W.

Do not implement a fully general multi-producer/multi-consumer queue if FLACHEAD only needs:

one decoder producer
and:
one output consumer.

---

# 16. Thread Ownership

The decoder thread owns:

write position.

The audio thread owns:

read position.

Shared state should use appropriate atomic operations or synchronization.

Only the producer modifies write-side state.

Only the consumer modifies read-side state.

Only the minimum required state should be shared between producer and consumer.

---

# 17. Synchronization

The ring buffer MUST be thread-safe according to the actual threading architecture.

Synchronization MUST guarantee:

- no data races
- no buffer overwrite
- no stale reads
- correct occupancy
- correct wraparound

The implementation SHOULD use the simplest synchronization primitive that meets the required performance.

The exact concurrency model MUST be documented in code.

The following operations MUST be thread-safe according to their intended usage:

- write
- read
- available
- free space
- flush
- stop
- generation reset

The ring buffer must use a safe synchronization strategy.

The API must clearly document which methods may be called concurrently.

---

# 18. Buffer Synchronization Primitives

The producer/consumer implementation MAY use:

- condition variables
- semaphores
- event flags
- atomic state + efficient waiting

The choice MUST be appropriate for the actual thread model.

The output path SHOULD avoid blocking mutexes where practical.

A lock that occasionally stalls for milliseconds can cause an underrun.

Neither decoder nor output synchronization should continuously poll at high frequency.

Avoid:

while (!data_available)
    check();

Use:

condition variables
events
semaphores
or another efficient synchronization mechanism.

Condition variables MAY be used for non-real-time producer coordination.

The audio output path itself SHOULD avoid blocking indefinitely on a general-purpose mutex.

---

# 19. No Busy Waiting

The decoder MUST NOT continuously poll the buffer at maximum CPU speed while waiting for space.

The output thread or callback must not wait indefinitely for the producer.

Neither producer nor consumer should continuously poll at high frequency while waiting for buffer state changes.

Incorrect:

while(buffer_full)
    continue;

Preferred:

wait for buffer condition

Use:

condition variables
events
semaphores
or another efficient synchronization mechanism.

Idle workers should sleep/block.

---

# 20. Backpressure

The decoder must respect buffer capacity.

When full:

Decoder waits.

When low:

Decoder resumes.

This is backpressure.

The buffering layer MUST NOT solve excessive decoder production by allocating more memory indefinitely.

When the buffer reaches its high-water mark:

the producer must stop producing or wait according to the pipeline design.

If the decoder speed is much greater than output consumption:

The buffer eventually reaches HIGH.

The decoder SHOULD sleep.

This is expected.

High decoder speed is not a reason to increase buffer size indefinitely.

The decoder must never write more frames than:

available ring-buffer capacity.

If only 1000 frames are free:

decode/write no more than safely fits.

The buffer API should support:

writing fewer frames than the decoder produced

or:

decode directly into available capacity.

---

# 21. Watermarks

The buffering system SHOULD expose three thresholds:

LOW_WATERMARK
TARGET_WATERMARK
HIGH_WATERMARK

Conceptually:

0%
│
├── EMPTY
│
├── LOW
│
│   normal refill zone
│
├── TARGET
│
│   healthy operating zone
│
├── HIGH
│
│   decoder can pause
│
└── 100%
    FULL

The exact thresholds MUST be configurable or centrally defined.

---

# 22. Low Watermark

LOW_WATERMARK means the buffer needs more PCM.

When occupancy falls below this threshold:

Decoder worker SHOULD wake.

The decoder SHOULD continue producing PCM until:

TARGET_WATERMARK or HIGH_WATERMARK

depending on scheduling policy.

---

# 23. Target Watermark

TARGET_WATERMARK is the preferred operating region.

After a refill:

buffer occupancy should normally reach at least the target.

The target MUST provide sufficient safety margin for ordinary system latency.

The decoder normally aims to refill toward TARGET_WATERMARK.

It does not need to constantly fill the buffer to 100%.

---

# 24. High Watermark

When occupancy reaches HIGH_WATERMARK:

Decoder SHOULD pause.

This prevents:

- unnecessary CPU usage
- unnecessary file reads
- unnecessary decoder work

The decoder resumes when the buffer drops below an appropriate threshold.

---

# 25. Hysteresis

The buffer SHOULD use hysteresis to avoid rapidly waking and sleeping the decoder.

Example:

Wake decoder: occupancy < LOW
Stop decoder: occupancy > HIGH

Do NOT use:

wake at 50%
stop at 50%

This can cause constant state switching.

Low/high thresholds should not be identical.

This prevents rapid producer start/stop oscillation.

Example:

LOW = 25%
HIGH = 75%

The exact values must be determined through benchmarking.

---

# 26. Buffer States

The buffer SHOULD expose logical states such as:

EMPTY
LOW
READY
NORMAL
HIGH
FULL

The exact thresholds are implementation-defined.

EMPTY means:

buffer contains zero readable PCM frames.

During playback this indicates starvation risk.

LOW means:

there is PCM available, but the buffer is approaching the point where output may starve.

The decoder should receive higher priority to refill it.

READY means:

enough PCM exists to safely begin playback.

The Playback Engine uses this state during startup.

NORMAL means:

buffer occupancy is comfortably above the low watermark.

Decoder can run at normal scheduling priority.

HIGH means:

buffer is approaching its maximum capacity.

The decoder should slow down or wait.

FULL means:

no additional PCM frames can be written.

The producer must wait or yield.

---

# 27. Full Buffer

When the buffer reaches its high watermark:

The decoder SHOULD stop decoding temporarily.

It MUST NOT overwrite unread PCM.

Correct:

Decoder
    ↓
Buffer FULL
    ↓
Decoder waits

Incorrect:

Decoder
    ↓
Buffer FULL
    ↓
Overwrite old PCM

Overwriting unread PCM would cause audible corruption.

When the ring buffer is full:

The producer MUST stop writing.

It MUST NOT:

- overwrite unread PCM
- allocate another buffer
- silently drop samples

The decoder should wait or yield until space becomes available.

---

# 28. Empty Buffer

When the output consumes all available PCM:

The buffer becomes empty.

The output backend may report an underrun if it needs data immediately.

The decoder SHOULD be awakened as soon as occupancy approaches the low watermark.

The system MUST distinguish:

- temporary buffer starvation
- from actual end-of-file
- from decoder failure
- from device failure

---

# 29. Decoder Refill Loop

Conceptual behavior:

while playback_active:
    if buffer below LOW:
        decode more
    if buffer reaches HIGH:
        wait
    if seek requested:
        flush
    if track ended:
        signal EOF

The actual implementation may use condition variables, semaphores, event objects, or another efficient mechanism.

---

# 30. Output Consumption

The output backend requests PCM according to its hardware period.

Example:

Audio Output
    ↓
request N frames
    ↓
PCM Buffer
    ↓
return N frames

The buffer SHOULD support efficient contiguous reads.

If the requested amount crosses the ring-buffer boundary:

read: end of buffer, then: beginning of buffer

The implementation SHOULD minimize extra copies.

The output layer typically consumes PCM in blocks determined by its period size.

The buffer SHOULD efficiently satisfy these requests.

If output requests more frames than are currently available:

The system must have an explicit policy.

During normal playback, the preferred behavior is:

wait/refill before output starvation.

The audio output MUST NOT receive uninitialized memory.

---

# 31. Underrun Prevention

The primary purpose of buffering is to prevent:

Decoder delay → Buffer empty → Audio underrun

The buffer MUST provide enough time to tolerate normal:

- SD-card latency
- scheduler delays
- CPU contention
- decoder variability

The safety margin MUST be determined experimentally.

The primary defense is:

adequate buffering combined with continuous decoder scheduling.

---

# 32. Underrun Detection

The system MUST count underruns.

Recommended diagnostic fields:

- underrun_count
- last_underrun_time
- buffer_level_at_underrun
- decoder_state_at_underrun
- current_track
- sample_rate
- bit_depth
- channels
- generation
- track
- requested frames
- available frames

This information is useful for Pi performance tuning.

The system SHOULD record this separately from an ALSA-level underrun.

FLACHEAD may experience:

1. Application PCM starvation.
2. ALSA/device underrun.

They should be tracked separately.

Example:

application_buffer_underruns
alsa_underruns

This distinction is important for diagnosing whether the decoder or output path is responsible.

---

# 33. Underrun Recovery

If an underrun occurs:

1. Record diagnostic information.
2. Wake decoder.
3. Refill PCM.
4. Resume output when possible.

The Playback Engine SHOULD decide whether the state becomes:

BUFFERING or remains: PLAYING

depending on output-backend behavior.

A single occasional underrun during development MUST NOT automatically trigger complicated recovery logic.

First determine:

- CPU saturation
- buffer size
- decoder throughput
- SD-card latency
- output scheduling

Only then optimize.

Do not hide persistent underruns by adding unlimited buffering.

---

# 34. Repeated Underruns

Repeated underruns indicate a systemic problem.

Possible causes:

- decoder too slow
- resampler too expensive
- filesystem latency
- CPU contention
- insufficient buffer
- scheduler delays
- output backend issue
- USB audio issue

The system MUST NOT simply hide repeated underruns by endlessly increasing the buffer.

Increasing the buffer may hide short-term stalls.

It cannot fix a producer that is consistently slower than real time.

A recoverable underrun must not crash FLACHEAD.

---

# 35. Underrun Policy

A short underrun must not automatically restart the track.

An underrun must not alter logical track position except where required by actual output consumption.

The system must distinguish:

- temporary underrun (short OS scheduling delay)
- from persistent inability to decode fast enough

Temporary recovery may succeed without changing track state.

Persistent failure should produce a controlled playback error.

---

# 36. Overrun Detection

An overrun occurs when the producer attempts to write beyond available capacity.

Overruns MUST never silently overwrite unread PCM.

The system SHOULD:

1. detect the condition
2. record diagnostic information
3. stop or throttle the producer
4. preserve existing unread PCM

An overrun should be considered an implementation bug or synchronization failure.

---

# 37. Startup Buffering

At playback start, FLACHEAD SHOULD prefill the PCM buffer before beginning sustained output.

Playback should not normally begin with:

zero buffered frames

The output should first accumulate enough PCM to survive small decoder delays.

Suggested conceptual behavior:

Decoder
    ↓
Fill to START threshold
    ↓
Start ALSA
    ↓
Continue refill while playing

The exact threshold MUST be tuned experimentally.

---

# 38. Startup Latency

The system MUST balance:

buffer startup duration against underrun resistance.

A 2-second startup buffer may improve robustness but increases:

- play-button-to-audio latency
- seek-to-audio latency

The target should be the smallest reliable value.

The startup threshold must not be unnecessarily large.

Large prebuffers increase:

- startup delay
- seek response delay
- memory use.

The initial implementation should choose the smallest prebuffer that reliably prevents startup underruns on the Pi Zero W.

This must be determined empirically.

---

# 39. Startup Buffer Range

A practical starting point for benchmarking is:

startup: approximately 150-300 ms

The exact values must be benchmarked.

---

# 40. Steady-State Buffering

After playback starts:

the buffer should remain above the low watermark under normal conditions.

The decoder should refill the buffer in reasonably sized blocks.

Avoid:

- single-frame writes
- excessively tiny writes
- huge allocations.

A healthy playback session should generally show:

startup fill → healthy occupancy → small fluctuations → stable playback.

---

# 41. Steady-State Buffer Range

steady-state: approximately 300-1000 ms

These are starting ranges, not immutable requirements.

---

# 42. Steady-State Condition

For stable playback:

average PCM production rate > average PCM consumption rate

over the long term.

---

# 43. Temporary Delay

If:

decoder temporarily slows down

but:

existing PCM buffer contains sufficient audio

playback should continue normally.

The buffer provides temporary tolerance.

---

# 44. Buffering Does Not Fix Permanent Performance Problems

A larger buffer can absorb short delays.

It cannot compensate indefinitely for:

- decoder slower than real-time
- resampler too expensive
- CPU starvation
- filesystem failures
- device failures

If average production rate is below average consumption rate, the buffer will eventually empty.

Therefore:

buffering protects against temporary jitter.

It does not replace sufficient processing performance.

This distinction is important when diagnosing performance.

---

# 45. Latency Budgets

Total audio latency is influenced by:

- decoder startup
- FLACHEAD buffer
- processing buffer
- ALSA period
- ALSA buffer
- USB transport
- DSP latency

FLACHEAD MUST NOT assume the ring-buffer duration equals total audible latency.

Conceptually:

total_latency ≈
    application_buffer_latency
  + ALSA_queue_latency
  + hardware_latency
  + DSP_latency

Exact values must be measured.

---

# 46. Seek Latency

Seek latency should include:

- decoder seek operation
- buffer refill
- ALSA restart
- device scheduling

Large buffers can increase seek latency.

Therefore buffer sizing affects both reliability and responsiveness.

---

# 47. Pause Latency

If a large amount of audio is queued:

pause may require careful ALSA handling to stop promptly.

The implementation should use proper device controls rather than waiting for queued audio to naturally finish.

---

# 48. Track Change Latency

When switching tracks:

all old audio must be discarded.

A huge application buffer provides no benefit here.

---

# 49. Latency Priority

FLACHEAD is a music player.

The system should prioritize:

reliable playback over extremely low latency.

However, latency should remain responsive enough for:

- pause
- seek
- track changes.

Users notice latency when:

- pressing pause
- pressing play
- seeking
- changing tracks

Therefore total queued audio should not be unnecessarily large.

---

# 50. Pi Zero W Memory Constraints

The Raspberry Pi Zero W has significantly less available system resources than a desktop machine.

Therefore FLACHEAD MUST avoid unnecessarily large PCM buffers.

The current service configuration uses a memory limit of:

128 MB

Memory is also required by:

- UI
- renderer
- textures
- fonts
- operating system
- other FLACHEAD components
- album artwork
- lyrics
- UI state
- decoder
- filesystem cache

The audio buffer MUST have a bounded and justified memory footprint.

---

# 51. Memory Budget

The PCM buffering layer MUST have a documented memory budget.

The budget MUST include:

- ring buffer
- decoder buffers
- file reader buffers
- output backend buffers
- transition buffers

It MUST remain within the system's configured memory constraints.

The audio subsystem MUST leave sufficient memory for:

- UI
- renderer
- fonts
- textures
- operating system
- other FLACHEAD components

---

# 52. Memory Calculation

Approximate PCM memory:

buffer_frames × channels × bytes_per_sample

For example:

22050 frames × 2 channels × 3 bytes ≈ 132300 bytes

for packed 24-bit stereo.

Actual memory depends on the internal representation.

---

# 53. Memory Monitoring

The Pi audit tooling SHOULD monitor:

- PCM buffer size
- decoder allocations
- peak RSS
- output buffers
- texture memory

The buffer itself SHOULD expose its configured capacity.

Diagnostics should expose approximate memory usage of each major audio component.

---

# 54. Why Not Use Huge Buffers

Huge buffers cause:

- higher memory usage
- slower startup
- slower seeking
- longer recovery times
- more stale audio after commands
- more complicated state transitions
- more memory pressure

FLACHEAD should use enough buffering, not maximum buffering.

---

# 55. Allocation Policy

The ring buffer SHOULD be allocated:

At initialization or when the audio format is established.

It SHOULD NOT repeatedly allocate memory during playback.

Preferred:

Initialize buffer → Reuse for entire playback session

If the required byte capacity changes because the format changes, resizing MUST occur outside the real-time output path.

Normal playback MUST avoid repeated heap allocations.

Allocate the ring buffer once per appropriate playback/output configuration.

Per-period heap allocation is prohibited in the real-time output path.

---

# 56. Buffer Resizing

Buffer resizing SHOULD NOT happen during active output consumption.

Preferred:

Stop/hold output → Drain or invalidate → Resize → Refill → Resume

Avoid dynamic resizing while audio is actively consuming PCM.

Dynamic buffer sizing MAY be implemented later.

The initial implementation SHOULD prefer a predictable fixed-capacity buffer.

Dynamic resizing adds complexity and synchronization risks.

---

# 57. Buffer Reuse

When starting another track with the same compatible PCM architecture:

The existing ring buffer MAY be reused.

Before reuse:

- old PCM MUST be flushed
- producer state MUST be reset
- consumer state MUST be reset
- EOF state MUST be reset
- error state MUST be reset

If PCM representation remains identical:

the same allocated buffer MAY be reused after flushing.

If representation changes:

reconfiguration is required.

---

# 58. Buffer Reconfiguration

The buffer may need reconfiguration when:

- channel count changes
- sample format changes
- bytes per sample changes

The safe sequence is:

stop/coordinate → flush → destroy/reconfigure buffer → start new generation

---

# 59. Format Ownership

The buffer MUST know which PCM format it contains.

At minimum:

- sample rate
- channels
- bytes per sample
- sample format
- track generation

This prevents accidental consumption of PCM under the wrong format.

---

# 60. PCM Format Change

If the decoder reports a new PCM format:

Current buffer → Invalidate → Configure new format → Begin filling

The buffer MUST NOT silently reinterpret existing bytes using a new format.

If the PCM representation changes:

the buffer must be flushed.

Depending on the implementation, it may need to be recreated.

---

# 61. Format Consistency

Every sample inside one active buffer generation MUST use the same PCM format.

A buffer MUST NOT contain:

old 44.1 kHz PCM followed by: new 96 kHz PCM

without an explicit flush/reconfiguration.

The same active PCM buffer must not contain incompatible representations without explicit format metadata and controlled boundaries.

---

# 62. High-Quality Requirement

The buffer MUST NOT reduce audio quality.

Buffering is a transport mechanism.

It MUST NOT:

- quantize samples
- resample
- normalize samples
- alter channel data
- clip samples

The buffer itself MUST NOT alter PCM sample values.

---

# 63. "Only Final Output PCM" Rule

The ring buffer must contain:

FINAL OUTPUT PCM

that is ready to be consumed by the audio backend.

It must not contain:

- compressed FLAC data
- arbitrary decoder state
- UI state
- lyrics data
- metadata

Only PCM that has passed through the conversion pipeline should enter the output ring buffer.

---

# 64. Volume Processing

Volume MUST NOT be applied inside the PCM buffer.

Software volume (if used) is applied before PCM enters the ring buffer as part of the audio processing stage.

Hardware volume is handled by the output device (e.g., TANCHJIM BUNNY DSP) after the audio backend.

This keeps the buffer purely a transport/storage layer for final output PCM.

---

# 65. ReplayGain

ReplayGain MUST NOT be applied by the ring buffer.

If enabled:

Decoder → ReplayGain Processor → PCM Buffer → Output

If disabled:

Decoder → PCM Buffer → Output

---

# 66. DSP

DSP processing MUST remain outside the buffer.

The BUNNY DSP may process the final audio stream externally.

The buffer should not contain device-specific DSP logic.

---

# 67. Audio Processing Placement

If ReplayGain, volume processing, or DSP processing exists:

Format Conversion → ReplayGain/Volume/DSP Processing → PCM Buffer → Output

The ring buffer contains FINAL OUTPUT PCM. All format conversion, resampling, channel conversion, and sample-domain processing (ReplayGain, volume, EQ, DSP) MUST occur before PCM enters the ring buffer. The ring buffer MUST NOT contain source PCM or intermediate PCM formats that require further conversion before output.

---

# 68. Bit-Perfect Mode

When bit-perfect output is enabled:

FLAC Decoder → PCM Buffer → Minimal Output Path → Audio Device

No hidden processing should occur.

---

# 69. Seek Flush

A seek invalidates all PCM currently stored in the buffer.

Correct sequence:

Seek Requested → Stop output consumption → Flush ring buffer → Seek decoder → Decode new PCM → Fill buffer → Resume output

Old PCM MUST NOT survive the seek.

Recommended detailed sequence:

1. invalidate current playback generation
2. stop/flush output
3. clear PCM buffer
4. reposition decoder
5. begin decoding from new position
6. refill buffer
7. restart output.

After seeking:

do not immediately resume if insufficient PCM has been prepared.

---

# 70. Flush Operation

The ring buffer MUST support an explicit:

flush()

operation.

flush() MUST:

- discard all unread PCM
- reset occupancy
- establish a clean read/write relationship
- preserve allocated memory
- remain safe under synchronization

flush() MUST NOT free and recreate the buffer during normal seeking.

A normal seek should usually: clear/reset rather than destroy/reallocate the entire buffer.

This avoids unnecessary allocations.

---

# 71. Why Flush Is Required

Without a flush:

old audio may remain queued.

Example:

User seeks: 30s → 120s

Buffer still contains: 30s-35s

Those samples MUST NOT play after the seek.

---

# 72. Seek Race Safety

A seek request may arrive while:

- decoder is writing
- output is consuming
- ALSA is playing previously submitted frames

Therefore seek MUST coordinate all three layers.

It is not sufficient to only move the decoder file position.

A decoder may be writing while a flush occurs.

The implementation must ensure that:

old producer writes cannot repopulate the buffer after flush.

Generation checks or producer synchronization are required.

---

# 73. Fast Seek

For a seek:

The system SHOULD avoid decoding the entire file from the beginning.

The FLAC decoder should use available seek points/indexes where supported.

After seeking:

decoder starts near target then decodes forward until target position.

---

# 74. Seek Coalescing

Repeated seek commands must not cause repeated unnecessary full pipeline initialization.

If several seek requests arrive before decoding resumes:

the engine may coalesce them.

Only the newest target matters.

---

# 75. Skip Forward and Skip Back

Skip forward should be implemented using the same seek/flush rules.

Do not leave previously buffered PCM playing after the target position is selected.

Skip back may:

- seek to previous position
- restart current track
- move to previous track

depending on Playback Engine policy.

The buffering layer only handles the resulting seek/flush operation.

---

# 76. Track Transition

At a normal track transition:

Current PCM → Drain output → Track A finished → Flush/replace buffer → Track B PCM → Continue

For gapless playback, the buffer MAY contain PCM belonging to both track contexts, but ownership MUST be explicit.

When changing tracks:

the PCM buffer MUST be flushed.

Track A PCM MUST NEVER continue into Track B unless explicit gapless architecture intentionally transfers compatible data.

---

# 77. Track Identity and Generation Tracking

PCM data SHOULD be associated with a track generation or context identifier.

Example:

Track A: generation = 41
Track B: generation = 42

After a seek or track switch:

generation changes.

Any stale PCM associated with an old generation MUST be discarded.

This provides an additional safeguard against stale audio.

---

# 78. Generation Counter

The Playback Engine SHOULD increment a generation counter when:

- track changes
- seek occurs
- decoder is recreated
- playback session is reset

PCM produced under an old generation MUST NOT be consumed as current PCM.

---

# 79. Stale PCM Protection

This is mandatory.

Example failure:

Track A decoder → PCM buffer

User presses NEXT.

Track B starts.

Old Track A decoder finishes a pending decode.

Track A PCM MUST NOT enter Track B's buffer.

Session/generation validation MUST prevent this.

After:

- seek
- track change
- stop
- device reset

no PCM belonging to the previous session may reach the output.

This is a hard correctness requirement.

---

# 80. Generation Check

Before committing a decoded block:

the producer should verify that its generation remains current.

If the generation is stale:

discard the block.

A stale producer must not write into the current buffer.

A converter operating under an obsolete generation must stop producing output.

If producer generation (G10) does not equal active generation (G11):

the write MUST be rejected or discarded.

---

# 81. Normal Next Track

For normal next-track playback:

Track A → drain → output completion → flush old session → Track B buffer → play

---

# 82. Gapless Mode

True gapless playback may require more advanced buffering.

For compatible tracks:

Track A PCM → Track B PCM → continuous output

This MUST only occur when:

- formats are compatible
- session transition is controlled
- sample boundaries are correct
- no stale data can enter the stream

If gapless playback is implemented:

the transition mechanism must explicitly authorize the next generation.

The system must never accidentally append the next track to the current track because of stale buffer state.

---

# 83. Prebuffering Next Track

Future optimization MAY decode a small amount of the next track before Track A ends.

However, this MUST NOT be implemented as an uncontrolled second PCM buffer.

If prebuffering is added, it MUST have:

- bounded memory
- explicit ownership
- defined cancellation
- defined behavior during seek
- defined behavior during skip

A future optimization may use:

CURRENT TRACK BUFFER and NEXT TRACK PREPARATION BUFFER.

These must remain logically separate.

---

# 84. Full Track Preloading Is Prohibited

Full-track PCM preloading is prohibited for normal operation.

A long FLAC track could consume excessive RAM when decoded to PCM.

A 44.1 kHz stereo 16-bit stream requires approximately:

176,400 bytes per second.

One hour would therefore require roughly:

635 MB

before additional overhead.

Therefore:

FULL TRACK PCM PRELOADING IS NOT ACCEPTABLE.

FLACHEAD MUST STREAM AUDIO.

Memory consumption should depend primarily on:

buffer duration rather than track duration.

---

# 85. Compressed Prefetch

A future optimization MAY prefetch compressed FLAC data.

If implemented:

FLAC file → Compressed read buffer → Decoder → PCM ring buffer → Output

The compressed buffer and PCM buffer MUST remain separate.

Neither:

compressed data nor decoded PCM

may accumulate without a hard limit.

Increasing the compressed FLAC input buffer does not necessarily solve PCM underruns.

Increasing PCM buffering does not necessarily solve poor filesystem throughput.

---

# 86. Pause Behavior

On pause:

The buffer MAY retain decoded PCM.

The implementation SHOULD avoid unnecessarily flushing the buffer.

When playback resumes, existing PCM can be consumed.

However, if the output backend requires a flush/restart, the implementation MUST handle this explicitly.

During pause:

The decoder SHOULD normally stop producing additional PCM once a sensible watermark is reached.

The buffer MUST NOT continuously fill while the user leaves playback paused.

Otherwise memory or unnecessary CPU usage could increase.

The user should not press pause and then hear a large amount of queued audio continue unexpectedly.

---

# 87. Resume

On resume:

If enough PCM remains: resume immediately.

If the buffer has been intentionally flushed: refill before restarting output.

The behavior MUST be deterministic.

The buffer should not automatically be flushed on every pause/resume unless required by the backend.

---

# 88. Stop Behavior

On stop:

The buffer MUST be flushed.

The decoder MUST stop producing new PCM.

The output MUST stop consuming PCM.

All track-specific PCM becomes invalid.

STOP is different from PAUSE.

STOP should:

- invalidate generation
- stop output
- discard PCM
- reset playback state according to Playback Engine policy.

---

# 89. End-of-Stream

The buffer needs to distinguish:

BUFFER EMPTY from: DECODER EOF

These are not equivalent.

Example:

Decoder: EOF
Buffer: 20 ms remaining
Output: still playing

The Playback Engine MUST wait until the final PCM is consumed before considering the track finished.

---

# 90. EOF Marker

The buffering layer SHOULD support an end-of-stream indicator associated with the current generation.

Conceptually:

PCM data + EOF = true

Once:

EOF = true and occupancy = 0

the track is fully drained.

---

# 91. EOF Sequence

At decoder EOF:

the producer must signal: SOURCE_EOF.

EOF does not immediately mean playback is finished.

The buffer may still contain PCM.

Correct sequence:

SOURCE EOF → decoder stops producing → buffer continues draining → output drains → track completion.

Only when:

SOURCE_EOF + BUFFER_EMPTY + OUTPUT_DRAINED

should the playback engine declare normal completion.

The output backend MUST NOT report an underrun merely because the track ended normally.

---

# 92. Drain vs Flush

FLUSH: discard.

DRAIN: consume.

They must never be treated as the same operation.

NORMAL EOF uses: DRAIN.
SEEK uses: FLUSH.
STOP normally uses: FLUSH.
TRACK CHANGE uses: FLUSH.
FORMAT CHANGE uses: FLUSH.

---

# 93. Output Device Reconfiguration

When output format or device changes:

The buffer SHOULD be flushed unless PCM is guaranteed to remain compatible.

Example:

44.1 kHz / 16-bit → Output change → 48 kHz / 24-bit

Existing PCM MUST NOT be sent to the new format without explicit conversion.

---

# 94. Buffer Hierarchy

Conceptually:

SD CARD
   ↓
FILE READ BUFFER
   ↓
FLAC DECODER
   ↓
PCM RING BUFFER
   ↓
ALSA BUFFER
   ↓
DSP/HARDWARE

Each layer has a different responsibility.

FLACHEAD's PCM buffer is distinct from the audio backend's own internal buffer.

ALSA itself may buffer audio.

Therefore the application does not control all queued audio through its own ring buffer.

Compressed FLAC input buffering and decoded PCM buffering are separate resources.

---

# 95. Buffer Diagnostics API

The buffer SHOULD expose a lightweight statistics structure.

Example:

PCMBufferStats

- capacity_frames
- occupied_frames
- free_frames
- occupancy_percent
- buffer_duration_ms
- low_watermark
- target_watermark
- high_watermark
- underrun_count
- overrun_count
- generation
- sample_rate
- channels
- bytes_per_sample
- producer_wait_count
- consumer_wait_count
- refill_count

The API MUST NOT expose raw internal pointers unnecessarily.

---

# 96. Memory Monitoring

The buffer SHOULD expose its configured capacity.

Developer diagnostics SHOULD expose:

- buffer_capacity_frames
- buffer_occupancy_frames
- buffer_fill_percent
- buffer_duration_ms
- low_watermark
- high_watermark
- underrun_count
- frames_produced
- frames_consumed
- frames_submitted
- frames_played where available
- application_underruns
- ALSA_underruns
- decoder_stalls
- buffer_flushes
- seek_count
- minimum occupancy
- maximum occupancy
- average occupancy

Do not log every read, write, buffer index during normal operation.

This would waste CPU and generate excessive output.

Aggregate metrics instead.

A low-frequency diagnostic sampler may periodically record these.

The existing FLACHEAD benchmark system should eventually be able to report audio-related statistics.

The benchmark output MUST remain lightweight.

---

# 97. UI Independence

The PCM buffer MUST operate independently of:

- screen refresh rate
- UI frame tier
- animations
- touchscreen state
- launcher state
- task overview
- lyrics rendering

A UI slowdown MUST NOT directly cause PCM buffering to stop.

---

# 98. Playback Continuity

The buffering layer exists specifically to isolate audio playback from non-audio workload.

For example:

Home screen: 30 FPS
Launcher: 45 FPS
Task Overview: 30 FPS

Music playback: must remain continuous.

The audio subsystem must not depend on rendering at 60 FPS.

UI activity must not be allowed to consume so much CPU that the audio pipeline repeatedly starves.

The architecture should preserve audio stability first.

If necessary:

UI rendering quality may degrade before audio playback is intentionally degraded.

Audio continuity has higher priority than nonessential visual effects.

---

# 99. Lyrics Synchronization

Lyrics timing MUST use playback position.

It MUST NOT infer time by counting UI frames.

Preferred source:

audio playback clock, not screen refresh rate.

---

# 100. Visualization Data

Music visualizations MUST NOT consume the playback ring buffer.

If the UI needs:

- waveform
- spectrum
- VU meter

the audio system should create a separate low-cost analysis stream.

Example:

PCM
 ├──→ Output
 └──→ Analysis → UI

The analysis path MUST never block audio output.

---

# 101. Artwork

Album artwork loading MUST never block PCM production.

The audio thread MUST NOT wait for:

- image decoding
- image scaling
- metadata UI updates
- filesystem artwork searches

---

# 102. Music Library Scanning

Library scanning MUST remain completely independent of the playback buffer.

A large library scan MUST NOT:

- pause audio
- starve decoder CPU
- acquire audio locks for long periods

---

# 103. SD Card Latency

The buffer MUST tolerate ordinary SD-card read latency.

The decoder should read ahead enough that a short filesystem delay does not immediately cause an underrun.

The exact safety margin MUST be measured on the target Pi.

The decoder may experience filesystem latency.

Buffering exists partly to absorb short filesystem delays.

However, the decoder SHOULD use efficient sequential reads and appropriate file buffering.

Do not assume SD-card latency is always negligible.

---

# 104. Removable Storage

If the storage containing the current track disappears:

The decoder MUST report an I/O failure.

The PCM buffer may continue playing already-decoded data.

Once it becomes impossible to continue:

Playback transitions to an appropriate error state.

The application MUST NOT crash.

---

# 105. Power Considerations

Although battery life is not the primary FLACHEAD constraint, unnecessary CPU activity should still be avoided.

The decoder SHOULD sleep when the buffer is sufficiently full.

The buffer manager SHOULD avoid busy-wait loops.

A large buffer may allow the decoder to sleep for longer intervals.

This can reduce unnecessary CPU wakeups.

---

# 106. CPU Utilization

The decoder should operate in batches large enough to be efficient.

Too-small decode operations create:

- more function-call overhead
- more synchronization
- more scheduling events

Extremely small blocks increase:

- synchronization overhead
- function-call overhead
- wakeups
- CPU usage.

Extremely large blocks can increase:

- latency
- memory usage
- seek discard cost.
- temporarily consume CPU
- delay output thread scheduling
- increase seek cancellation delay.

Choose block size based on:

- sample rate
- CPU performance
- backend write behavior
- latency target.

The implementation should use a moderate block size determined by hardware profiling.

The initial value should be configurable during development.

---

# 107. Batch Decoding

Decode enough frames per iteration to make efficient use of the decoder.

The exact batch size should be benchmarked.

Burst decoding is preferable to requiring perfectly uniform decoder timing.

The decoder may use larger refill batches when:

buffer is low

and smaller batches when:

buffer is near target.

This is optional.

---

# 108. Audio Thread Priority

The audio output path is latency-sensitive.

The output thread MUST NOT wait indefinitely for the decoder.

It SHOULD never perform:

- filesystem I/O
- FLAC decoding
- metadata parsing
- artwork processing
- lyrics parsing
- large memory allocations

The audio output path should have priority over:

- UI rendering
- metadata processing
- artwork processing
- library scanning

when scheduling decisions are necessary.

The audio consumer MUST avoid holding long-duration locks while:

- calling ALSA
- decoding
- reading from disk
- performing expensive conversion

Long-held locks can cause priority inversion and audio starvation.

---

# 109. Audio Callback Rules

If the backend uses a real-time audio callback:

the callback must remain extremely lightweight.

The audio callback must not:

- decode FLAC
- access the database
- load artwork
- parse lyrics
- allocate large memory blocks
- perform filesystem operations
- wait on long locks
- perform network operations
- log to slow storage

The callback should primarily:

1. read available PCM
2. provide it to the backend
3. report underrun if insufficient data exists.

Dynamic memory allocation inside a real-time callback should be avoided.

Verbose logging inside the callback should be avoided.

Long mutex waits inside the callback are unacceptable.

The callback should consume already-prepared PCM.

The consumer should perform bounded work.

---

# 110. Decoder Waiting

The decoder worker may safely wait when the buffer is sufficiently full.

Preferred:

Buffer HIGH → Decoder sleeps → Buffer LOW → Decoder wakes

This keeps CPU consumption low.

---

# 111. Shutdown

Shutdown sequence:

1. Stop accepting new audio commands.
2. Signal decoder stop.
3. Stop PCM production.
4. Stop/flush output.
5. Flush PCM buffer.
6. Join worker threads.
7. Release audio resources.

No worker may access destroyed audio resources.

The buffer MUST not be destroyed while another thread can still access it.

Waiting operations must be interruptible by shutdown.

---

# 112. Invalid State Handling

Examples:

Reading from a closed buffer: return controlled failure.

Writing after shutdown: reject operation.

Flushing from an invalid lifecycle state: perform safe no-op or controlled failure.

The implementation MUST NOT access freed memory.

Errors MUST be handled without crashing the application.

---

# 113. Error Conditions

Buffer errors include:

- BUFFER_FULL
- BUFFER_EMPTY
- BUFFER_OVERRUN
- BUFFER_UNDERRUN
- BUFFER_FORMAT_MISMATCH
- BUFFER_GENERATION_MISMATCH
- BUFFER_INVALID_STATE
- INVALID_GENERATION

These should be controlled conditions rather than crashes.

---

# 114. Format Mismatch Protection

If the buffer expects stereo 24-bit PCM and a producer attempts to write stereo 16-bit PCM:

the operation MUST fail safely.

It MUST NOT reinterpret bytes incorrectly.

---

# 115. Memory Failure

If buffer allocation fails:

playback must fail cleanly.

Do not fall back to unbounded allocation.

The application should report: insufficient memory for audio buffer or an equivalent diagnostic.

Runtime allocation failure in the playback path must not crash the application.

Prefer preallocated resources.

---

# 116. Error During Playback

If an unrecoverable audio error occurs:

Output → ERROR → Playback Engine → Controlled state transition → UI notification

The UI MUST NOT need to understand ALSA error codes.

Recoverable errors MAY return to PLAYING after:

- device recovery
- buffer refill
- ALSA recovery

Unrecoverable errors should transition to:

ERROR or PAUSED/STOPPED

according to the Playback Engine policy.

---

# 117. Thread Safety

The audio system MUST be safe under:

- pause during refill
- seek during decode
- stop during output
- next during buffering
- shutdown during ALSA write
- device disconnect during playback

These cases MUST be explicitly tested.

---

# 118. Race Conditions

Potential races include:

- decoder writes after flush
- output reads after flush
- seek while decoder is blocked
- shutdown while producer waits
- device removal while consumer writes

The following must be safe:

decoder writing while: seek invalidates buffer.

The buffer generation/state transition must prevent stale data publication.

The following must be safe:

decoder waiting while: application shuts down.

The wait must be interruptible.

The following must be safe:

output consuming PCM while: stop clears the buffer.

Synchronization must prevent use-after-free or stale reads.

The buffer must remain valid while the old decoder is being terminated.

The implementation MUST establish clear synchronization rules for each.

---

# 119. Memory Fragmentation

Repeated creation/destruction of large PCM buffers can contribute to memory fragmentation.

Prefer:

long-lived buffer allocation during an output-format session.

---

# 120. Interaction With Audio Processing

If ReplayGain, volume processing, or DSP processing exists:

Format Conversion → ReplayGain/Volume/DSP Processing → PCM Buffer → Output

The ring buffer contains FINAL OUTPUT PCM. All format conversion, resampling, channel conversion, and sample-domain processing (ReplayGain, volume, EQ, DSP) MUST occur before PCM enters the ring buffer. The ring buffer MUST NOT contain source PCM or intermediate PCM formats that require further conversion before output.

---

# 121. Interaction With Playback Engine

Playback Engine controls:

PLAY
PAUSE
STOP
SEEK
NEXT
PREVIOUS

The buffer responds to lifecycle changes.

Example:

SEEK: Playback Engine → PCMBuffer.flush() → Decoder.seek() → Decoder fills buffer → Output resumes

The buffer MUST NOT decide queue behavior.

---

# 122. Interaction With Decoder

Decoder asks: "How much space is available?"

Buffer answers: free_frames

Decoder writes only within that capacity.

The decoder MUST stop when:

free_frames = 0 or occupancy >= HIGH_WATERMARK

---

# 123. Interaction With Output Backend

Output asks: "How many frames are available?"

Buffer returns: available_frames

If enough data exists: return requested frames

If insufficient data exists: return available data and let the output/backend policy handle the remainder.

The exact behavior depends on ALSA/output backend semantics.

---

# 124. Profiling

The Pi audit workflow should eventually capture:

- decoder CPU
- audio output CPU
- conversion CPU
- buffer occupancy
- underruns
- memory usage

This allows the team to distinguish:

"buffer problem" from "CPU problem" from "device problem".

---

# 125. Testing

Required unit tests:

- empty buffer read
- single write
- single read
- full buffer
- wraparound
- partial read
- partial write
- read/write boundary
- flush
- format change
- generation change
- EOF with remaining PCM
- underrun
- overrun prevention
- shutdown
- concurrent producer/consumer
- frame alignment
- monotonic counter overflow
- mixed formats (44.1/16, 48/24, 96/24)
- mono/stereo transitions

---

# 126. Wraparound Testing

Explicitly test:

write near end → wrap → continue writing

and:

read near end → wrap → continue reading

Verify that:

- no bytes are skipped
- no bytes are duplicated
- ordering is preserved

---

# 127. PCM Integrity Testing

A known PCM sequence SHOULD be inserted into the ring buffer.

Example: 0, 1, 2, 3, 4, 5, ...

After arbitrary reads/writes and wraparound:

The output MUST remain: 0, 1, 2, 3, 4, 5, ...

No corruption is acceptable.

Use known sample patterns.

Verify:

frames are never split or reordered.

---

# 128. Concurrency Testing

Stress tests SHOULD run:

Producer thread + Consumer thread

for extended periods.

Verify:

- no races
- no corruption
- no deadlocks
- no buffer overrun
- no buffer underflow beyond expected test conditions
- no memory growth

Thread sanitizers SHOULD be used on development systems when practical.

---

# 129. Stress Scenarios

Test playback while:

- navigating UI
- opening launcher
- opening task overview
- viewing lyrics
- changing settings
- scanning library
- loading artwork
- performing metadata operations
- touch input
- animations
- album art display

Audio must remain stable.

Stress tests SHOULD include:

- rapidly seeking
- rapidly changing tracks
- repeatedly pausing/resuming
- opening/closing tracks

The audio pipeline MUST remain memory-safe and crash-free.

---

# 130. Long Playback Test

A long-run test SHOULD play multiple tracks continuously.

Monitor:

- buffer fill
- underruns
- memory
- CPU
- track transitions
- decoder errors
- ALSA errors

The test should detect memory leaks and state corruption.

There MUST be no unbounded resource growth.

---

# 131. Real Hardware Acceptance

The buffering layer is not considered validated until it has been tested with:

Raspberry Pi Zero W + real FLAC files + TANCHJIM BUNNY DSP + real SD card

Testing SHOULD include concurrent system activity.

Final buffering tests must run on the Raspberry Pi Zero W.

Playback should be tested with the actual TANCHJIM BUNNY DSP.

---

# 132. Performance Testing

Measure:

- read throughput
- write throughput
- synchronization overhead
- CPU usage
- memory usage
- underrun rate
- wake/sleep frequency
- decode time per block
- conversion time per block
- buffer write time
- total producer utilization

Testing MUST eventually be repeated on Pi Zero W.

---

# 133. Performance Test Matrix

Test at minimum:

- 44.1 kHz / 16-bit
- 48 kHz / 16-bit
- 44.1 kHz / 24-bit
- 48 kHz / 24-bit
- 96 kHz / 24-bit

where supported by the complete output chain.

Test under:

- normal UI activity
- album art display
- lyrics display
- library browsing
- background filesystem activity
- long playback
- rapid track changes
- realistic SD-card stalls
- high CPU load

---

# 134. Benchmark-on-Pi-Zero-W Policy

Buffer size MUST be benchmarked on the actual Pi Zero W.

The coding agent MUST NOT permanently hard-code a large arbitrary PCM buffer simply because Raspberry Pi hardware is constrained.

Instead:

1. Start with a conservative bounded buffer.
2. Measure underruns.
3. Measure memory.
4. Measure startup latency.
5. Measure seek behavior.
6. Adjust based on evidence.

Do not choose desktop-oriented audio buffer sizes blindly.

The coding agent must not hard-code a large buffer simply because it appears safer.

---

# 135. Acceptance Criteria

The buffering system is considered production-ready when:

- playback remains continuous under normal load
- buffer memory is bounded
- producer/consumer synchronization is correct
- underruns are detected
- EOF is handled correctly
- seeking discards stale PCM
- pause/resume is reliable
- track changes are reliable
- device errors are handled
- shutdown is safe
- UI operations cannot directly corrupt the audio buffer
- long playback does not cause memory growth
- performance is acceptable on Pi Zero W
- ring-buffer wraparound is correct
- no samples are lost
- no samples are duplicated
- no samples are overwritten
- overruns are prevented
- format changes are safe
- buffer occupancy is observable
- normal playback requires no heap allocation
- the implementation performs reliably on Pi Zero W
- PCM is streamed rather than fully preloaded
- full-track PCM is never preloaded
- producer and consumer ownership is explicit
- complete PCM frames are preserved
- startup prebuffering is deterministic
- decoder backpressure works
- output consumption works
- partial writes are handled
- shutdown interrupts waiting operations
- buffer state remains race-free
- memory usage remains stable
- long-duration playback remains stable
- realistic SD-card stalls do not immediately cause dropouts
- high-resolution FLAC remains sustainable
- target hardware testing passes
- buffer access is thread-safe
- overflow cannot overwrite valid unread PCM
- underflow cannot return invalid memory
- frame alignment is preserved
- low/high watermarks work
- hysteresis prevents oscillation
- short storage stalls are absorbed
- output jitter is tolerated
- output drain is distinguished from buffer flush
- buffer telemetry is available for debugging
- no swap dependency exists
- no continuous large allocations occur
- high sample-rate playback is tested
- 24-bit playback is tested
- mono/stereo behavior is tested
- CPU pressure is tested
- UI pressure is tested
- storage pressure is tested
- TANCHJIM BUNNY DSP playback is tested
- final buffer parameters are validated on the Raspberry Pi Zero W
- buffer parameters are centrally configured
- low/high watermarks are validated
- producer throughput exceeds average consumer demand
- the system tolerates short scheduling delays
- long playback produces no memory growth
- repeated track changes remain memory-stable
- mixed-format playback works correctly
- Raspberry Pi Zero W performance is acceptable
- TANCHJIM BUNNY DSP playback is stable.

---

# 136. Implementation Rules for the AI Coding Agent

The coding agent MUST:

1. Implement bounded PCM buffering.
2. Prefer a ring buffer.
3. Keep decoder and output responsibilities separate.
4. Reuse allocated PCM memory.
5. Use explicit low/target/high watermarks.
6. Implement backpressure.
7. Prevent buffer overwrite.
8. Prevent stale PCM after seeking.
9. Track PCM format.
10. Track playback generation.
11. Handle EOF separately from empty-buffer state.
12. Keep output timing independent of UI timing.
13. Avoid busy waiting.
14. Avoid unbounded allocations.
15. Avoid unnecessary PCM copies.
16. Keep buffer synchronization explicit.
17. Make shutdown safe.
18. Measure underruns.
19. Measure buffer occupancy.
20. Validate on Raspberry Pi Zero W.
21. Do not add complicated optimization before profiling.
22. Never sacrifice PCM correctness for performance.
23. Treat PCM playback as a streaming system.
24. Never decode an entire normal track into RAM.
25. Use a finite PCM buffer.
26. Measure occupancy in frames.
27. Preserve complete PCM-frame alignment.
28. Never overwrite unread audio.
29. Never read unavailable audio.
30. Implement producer backpressure.
31. Implement consumer starvation detection.
32. Use distinct low/high watermarks.
33. Use hysteresis where appropriate.
34. Keep real-time output callbacks lightweight.
35. Never decode FLAC inside a real-time audio callback.
36. Never perform filesystem I/O inside a real-time audio callback.
37. Never perform artwork loading inside a real-time audio callback.
38. Never perform lyrics parsing inside a real-time audio callback.
39. Avoid dynamic allocation inside real-time callbacks.
40. Avoid long locks inside real-time callbacks.
41. Flush the buffer on seek.
42. Flush the buffer on stop.
43. Flush the buffer on track change.
44. Flush the buffer on output-format change.
45. Drain the buffer on normal EOF.
46. Drain converter state before final track completion.
47. Account for backend output buffering.
48. Associate buffered data with playback generation.
49. Reject stale PCM from obsolete generations.
50. Prevent old workers from writing into a new session.
51. Keep buffer memory bounded.
52. Keep preload memory bounded.
53. Never preload full tracks as PCM.
54. Prefer sequential storage reads.
55. Allow short storage stalls to be absorbed by buffering.
56. Detect persistent storage starvation.
57. Prioritize audio continuity over nonessential UI rendering.
58. Keep library scanning from starving playback.
59. Keep buffer metrics available for diagnostics.
60. Test ring-buffer wraparound.
61. Test empty-buffer behavior.
62. Test full-buffer behavior.
63. Test underrun recovery.
64. Test seek flushing.
65. Test rapid track changes.
66. Test EOF draining.
67. Test format changes.
68. Test long-duration playback.
69. Test heavy UI activity.
70. Test shutdown races.
71. Benchmark buffer sizes on the actual Raspberry Pi Zero W.
72. Benchmark with the actual TANCHJIM BUNNY DSP.
73. Do not choose unnecessarily large buffers merely to hide architectural problems.
74. Do not choose unnecessarily small buffers merely to reduce latency.
75. Make buffer sizing configurable during development.
76. Document the final production buffer parameters.
77. Prefer deterministic behavior over clever buffering tricks.
78. Never allow buffer corruption to silently continue.
79. Never allow stale PCM to reach the audio device.
80. Keep buffering behavior explicitly documented.
81. Treat PCM frames as the authoritative unit.
82. Preserve channel/sample alignment.
83. Never allocate PCM based on full track duration.
84. Use producer-consumer semantics.
85. Keep read/write ownership explicit.
86. Prevent producer/consumer data races.
87. Allow shutdown to interrupt waiting operations.
88. Handle partial writes.
89. Handle partial decoder output.
90. Maintain exact frame counters.
91. Use startup prebuffering.
92. Use measured high/low water thresholds.
93. Prevent buffer overflow.
94. Clear stale PCM after seeking.
95. Invalidate old PCM after track changes.
96. Use playback generations where required.
97. Never mix incompatible PCM formats.
98. Keep next-track preparation separate from active playback buffering.
99. Avoid queue-wide PCM predecoding.
100. Avoid unnecessary allocations in the steady-state path.
101. Keep buffer synchronization lightweight.
102. Do not introduce lock-free complexity without profiling.
103. Test high-resolution FLAC.
104. Test long-duration playback.
105. Test SD-card latency.
106. Test CPU pressure.
107. Test UI pressure.
108. Test rapid seeking.
109. Test rapid track changes.
110. Test shutdown races.
111. Test device failure.
112. Keep the buffer architecture deterministic and observable.
113. Keep compressed-input buffering separate from output PCM buffering.
114. Keep total audio memory bounded.
115. Reuse buffers where safe.
116. Recreate buffers when format changes require it.
117. Reset buffer state deterministically.
118. Test partial reads/writes.
119. Test overflow and underflow.
120. Test seek races.
121. Test track-change races.
122. Test mixed sample-rate playback.
123. Test mixed bit-depth playback.
124. Test mono/stereo transitions.
125. Test memory stability.
126. Test CPU overhead.
127. Benchmark on the Raspberry Pi Zero W.
128. Test with the TANCHJIM BUNNY DSP.
129. Prefer simple correct buffering over unnecessary lock-free complexity.
130. Do not use buffer size as a substitute for fixing sustained performance problems.
131. Do not depend on swap for audio reliability.
132. Prefer the smallest buffer that provides reliable real-time playback.
133. Document the final buffer size and rationale.
134. Preserve audio continuity as the primary objective.
135. Never allow buffering behavior to become an uncontrolled source of latency.
136. Treat ALSA buffering as separate from application buffering.
137. Measure total output latency rather than assuming the PCM buffer equals latency.
138. Do not treat decoder EOF as audible track completion.
139. Drain remaining PCM before completing a track.
140. Keep next-track PCM isolated from the current track.
141. Test high-resolution formats independently.
142. Keep buffer tuning data-driven rather than arbitrary.
143. Use safe synchronization/atomic semantics.
144. Validate buffer configuration before allocation.
145. Check allocation-size overflow.
146. Prevent producer busy loops.
147. Prevent consumer starvation where the architecture can reasonably avoid it.
148. Keep pause behavior explicit.
149. Never use buffer fill level as the authoritative playback position.
150. Never treat buffered frames as already audible.
151. Keep backend latency separate from ring-buffer occupancy.

---

# 137. Final Architecture

The intended production flow is:

                      FLAC FILE
                          │
                          ▼
                    FILE READER
                          │
                          ▼
                    FLAC DECODER
                          │
                          │ SOURCE PCM
                          ▼
               FORMAT NEGOTIATION
                          │
                          ▼
               FORMAT CONVERSION (if required)
                          │
                          ▼
               AUDIO PROCESSING (optional)
               [ReplayGain, Volume, DSP]
                          │
                          ▼
               ┌────────────────────┐
               │    PCM RING        │
               │      BUFFER        │
               │                    │
               │ LOW/TARGET/HIGH    │
               │ WATERMARKS         │
               │                    │
               │ bounded capacity   │
               │ frame aligned      │
               │ generation aware   │
               └────────────────────┘
                          │
                          │ FINAL OUTPUT PCM frames
                          ▼
                AUDIO OUTPUT BACKEND
                          │
                          ▼
                         ALSA
                          │
                          ▼
                TANCHJIM BUNNY DSP
                          │
                          ▼
                      HEADPHONES

Control path:

UI / Hardware
      │
      ▼
Playback Commands
      │
      ▼
Playback Engine
      │
      ├── Decoder Control
      ├── Buffer Control (flush, generation, watermarks)
      ├── Output Control
      └── Track Control
             │
             ├── play
             ├── pause
             ├── stop
             ├── seek → FLUSH → NEW GENERATION
             ├── next → FLUSH → NEW GENERATION
             └── previous → FLUSH → NEW GENERATION

Normal operation:

BUFFER LOW → PRODUCER REFILLS → BUFFER TARGET → AUDIO CONSUMES → BUFFER LOW → REFILL.

Normal EOF:

DECODER EOF → CONVERTER DRAIN → PCM BUFFER DRAIN → AUDIO BACKEND DRAIN → TRACK COMPLETE.

Seek:

SEEK REQUEST → NEW GENERATION → FLUSH BACKEND → FLUSH PCM BUFFER → RESET DECODER → RESET CONVERTER → REFILL → PLAY.

Track change:

NEW TRACK → INVALIDATE OLD GENERATION → FLUSH OLD PCM → INITIALIZE NEW FORMAT → REFILL → PLAY.

---

# 138. Final Invariants

The central rule is:

THE PCM BUFFER IS THE BOUNDED REAL-TIME HANDOFF BETWEEN THE FLAC DECODER (PRODUCER) AND AUDIO OUTPUT (CONSUMER).

The buffer must remain:

BOUNDED
FORMAT-AWARE
GENERATION-AWARE
FRAME-BASED
THREAD-SAFE
LOW-ALLOCATION
SEEK-SAFE
MEMORY-EFFICIENT
and completely independent from the UI.

The first invariant:

THE PCM BUFFER MUST ALWAYS CONTAIN ONLY VALID PCM BELONGING TO THE CURRENT PLAYBACK GENERATION AND CURRENT AUDIO FORMAT.

The second invariant:

THE BUFFER SHOULD BE LARGE ENOUGH TO ABSORB NORMAL DECODING/I/O JITTER BUT NO LARGER THAN NECESSARY.

The third invariant:

AUDIO OUTPUT STABILITY TAKES PRIORITY OVER AGGRESSIVE LATENCY REDUCTION.

The fourth invariant:

THE PCM BUFFER IS A BOUNDED REAL-TIME HANDOFF BETWEEN DECODING AND OUTPUT.

The fifth invariant:

THE BUFFER MAY ABSORB TEMPORARY DELAYS BUT CANNOT COMPENSATE FOR INSUFFICIENT LONG-TERM DECODER THROUGHPUT.

The sixth invariant:

ONLY COMPLETE, VALID PCM FRAMES MAY BE PUBLISHED TO THE CONSUMER.

The seventh invariant:

STALE PCM FROM A PREVIOUS SEEK OR TRACK MUST NEVER REACH THE AUDIO OUTPUT.

The eighth invariant:

BUFFER MEMORY MUST REMAIN BOUNDED REGARDLESS OF TRACK LENGTH OR FILE SIZE.

The ninth invariant:

THE PRODUCER MUST APPLY BACKPRESSURE WHEN THE BUFFER IS FULL.

The tenth invariant:

THE CONSUMER MUST HANDLE EMPTY/UNDERRUN CONDITIONS WITHOUT CRASHING.

The eleventh invariant:

BUFFER PARAMETERS MUST BE BENCHMARKED ON THE RASPBERRY PI ZERO W, NOT GUESSED FROM DESKTOP HARDWARE.

The twelfth invariant:

FULL TRACK PCM PRELOADING IS PROHIBITED FOR NORMAL PLAYBACK.

The thirteenth invariant:

THE RING BUFFER CONTAINS FUTURE AUDIO, NOT CURRENT AUDIBLE POSITION.

The fourteenth invariant:

BUFFER FILL LEVEL IS A BUFFERING METRIC, NOT A PLAYBACK-CLOCK METRIC.

The fifteenth invariant:

NORMAL EOF DRAINS AUDIO; SEEK AND TRACK CHANGE FLUSH AUDIO.

The sixteenth invariant:

BUFFER BACKPRESSURE MUST NOT BE CONFUSED WITH PLAYBACK PAUSE.

The seventeenth invariant:

THE AUDIO CALLBACK MUST REMAIN BOUNDED AND NON-BLOCKING WHEREVER POSSIBLE.

The eighteenth invariant:

THE BUFFERING SYSTEM MUST BE BENCHMARKED ON THE ACTUAL RASPBERRY PI ZERO W AND TANCHJIM BUNNY DSP.

Final policy:

Decode ahead enough to protect playback.

Do not decode an entire track.

Do not allow the buffer to grow without bound.

Do not allow stale audio to survive a timeline transition.

Do not let UI workload determine audio buffering.

Do not assume desktop-sized buffers are appropriate for the Pi Zero W.

Measure the real hardware.

Tune the final values from actual:

- CPU usage
- RAM usage
- decoder throughput
- buffer occupancy
- ALSA behavior
- underrun rate
- startup latency
- seek latency
- track-transition latency

Final responsibility separation:

FILE READER = SEQUENTIAL SOURCE DATA
FLAC DECODER = SOURCE DATA → PCM
FORMAT PIPELINE = SOURCE PCM → OUTPUT PCM
PCM PRODUCER = PREPARES BUFFER DATA
PCM RING BUFFER = ABSORBS TIMING DIFFERENCES
AUDIO CONSUMER = REMOVES PCM AT PLAYBACK RATE
AUDIO BACKEND = PCM → DEVICE
PLAYBACK CLOCK = DETERMINES PLAYBACK POSITION
PLAYBACK ENGINE = CONTROLS STATE, GENERATIONS, SEEK, STOP, TRACK CHANGE, AND DRAIN/FLUSH POLICY

The buffer is the shock absorber of the audio system.

The decoder may occasionally be delayed.

The SD card may occasionally be slow.

The CPU may occasionally be busy.

The UI may occasionally render slowly.

The buffer exists so these temporary variations do not become audible interruptions.

The ideal behavior is:

Decoder produces PCM
    ↓
Buffer remains comfortably populated
    ↓
Output continuously consumes PCM
    ↓
Decoder sleeps when unnecessary
    ↓
Decoder wakes before the buffer becomes dangerously low
    ↓
Playback remains uninterrupted

On the Raspberry Pi Zero W, the objective is not to maximize buffer size.

The objective is to find the smallest practical buffer that provides reliable uninterrupted playback under real-world system load.
