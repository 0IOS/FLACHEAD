# FLACHEAD FLAC Decoder Architecture

Document

03_FLAC_DECODER.md

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

This document defines the complete FLAC decoding architecture for FLACHEAD.

The decoder is responsible for converting compressed FLAC data into validated PCM samples for the downstream audio pipeline.

The fundamental transformation is:

FLAC FILE → FLAC BITSTREAM → FLAC DECODER → PCM FRAMES → PCM BUFFER → AUDIO OUTPUT

The decoding pipeline MUST be:

- lossless
- streaming
- memory-bounded
- CPU-efficient
- asynchronous
- recoverable
- safe against malformed files
- suitable for Raspberry Pi Zero W

The decoder MUST NOT load an entire FLAC file into memory.

The decoder MUST NOT block the UI.

The decoder MUST NOT directly communicate with the UI.

The decoder MUST remain independent from:

- UI rendering
- queue management
- lyrics
- metadata presentation
- audio-device management
- playback controls
- playback policy

---

# 2. Core Principle

The decoder performs:

FLAC FILE → COMPRESSED FRAMES → DECODED PCM

It does not perform:

- UI rendering
- lyrics synchronization
- artwork processing
- queue management
- audio-device policy
- volume control
- playback state management
- repeat/shuffle logic
- screen navigation
- database logic
- network access

The primary objective is:

DECODE FLAC RELIABLY AND EFFICIENTLY WHILE PRESERVING AUDIO DATA AND KEEPING THE REAL-TIME AUDIO PATH STABLE.

---

# 3. Pipeline Overview

The complete conceptual pipeline is:

                         FLAC FILE
                             │
                             ▼
                       FILE READER
                             │
                             ▼
                    COMPRESSED FLAC DATA
                             │
                             ▼
                     FLAC STREAM PARSER
                             │
                             ▼
                       STREAMINFO
                             │
                             ▼
                      FLAC DECODER
                             │
                             ▼
                       SOURCE PCM
                             │
                             ▼
                    FORMAT PROCESSING
                   (optional / bypassable)
                             │
                             ▼
                     OUTPUT PCM
                             │
                             ▼
                       PCM BUFFER
                             │
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

Runtime control:

PlaybackEngine → Decoder → PCM Buffer Manager

The decoder produces PCM.

The decoder does NOT own playback state.

---

# 4. Decoder Ownership

## 4.1 The decoder is responsible for:

- opening FLAC streams
- validating FLAC structure
- reading FLAC data
- parsing STREAMINFO
- parsing metadata blocks required by playback
- exposing stream metadata
- decoding audio frames
- subframe decoding
- entropy decoding
- channel reconstruction
- producing PCM
- supporting seeking
- reporting duration
- reporting source format
- reporting decoder errors
- detecting end-of-stream
- closing resources

## 4.2 The decoder is NOT responsible for:

- queue management
- playlist management
- playback state
- volume
- UI
- artwork rendering
- lyrics
- audio device selection
- output-device configuration
- audio-device logic
- volume policy
- screen navigation
- shuffle mode
- repeat mode
- database logic
- network access
- playback policy

---

# 5. Decoder Abstraction

The rest of FLACHEAD SHOULD communicate through a generic decoder interface.

Conceptually:

AudioDecoder
    ├── FLACDecoder
    ├── FutureWAVDecoder
    ├── FutureMP3Decoder
    └── FutureOpusDecoder

The Playback Engine SHOULD NOT need to know the internal implementation of FLAC decoding.

Example conceptual interface:

    open(path)
    readMetadata()
    readFrames()
    getFormat()
    getDuration()
    decode()
    seek(position)
    tell()
    eof()
    error()
    getState()
    close()

The exact C++ APIs may differ.

The architectural boundary MUST remain.

---

# 6. FLAC as the Primary Format

FLACHEAD is designed around high-quality FLAC playback.

The initial decoder implementation MUST prioritize FLAC.

Other codecs MAY be supported later through separate decoder implementations.

The architecture SHOULD therefore use an abstraction such as AudioDecoder rather than hard-coding FLAC-specific assumptions throughout the Playback Engine.

---

# 7. Library Selection

The implementation should use a mature FLAC decoder library rather than implementing the FLAC codec from scratch.

A suitable native decoder library should be selected based on:

- ARMv6 compatibility
- Raspberry Pi OS compatibility
- decoding correctness
- CPU usage
- memory usage
- licensing
- seeking support
- maintenance status
- integration complexity
- binary size
- API stability
- streaming support
- error handling
- cross-compilation compatibility

Potential implementation options MAY include:

- libFLAC
- another well-tested lightweight FLAC decoder

FLACHEAD should initially use a proven native FLAC decoding library.

The application should wrap the library behind its own decoder interface.

This prevents the playback engine from becoming dependent on one specific library.

The coding agent MUST NOT introduce a large dependency without checking its impact on the Pi Zero W.

The coding agent must not invent a custom FLAC decoder unless explicitly instructed.

## 7.1 Why Not Implement FLAC Decoding Ourselves

FLAC decoding contains substantial codec complexity.

Implementing it independently introduces unnecessary risk involving:

- bitstream parsing
- Rice coding
- prediction
- residual reconstruction
- channel decorrelation
- CRC validation
- malformed-stream handling

A mature decoder is preferable.

## 7.2 libFLAC Consideration

If libFLAC is selected, FLACHEAD SHOULD use its streaming decoder interfaces rather than loading entire files.

The implementation SHOULD allow FLACHEAD to receive decoded PCM through callbacks or equivalent streaming mechanisms.

The callback path MUST remain lightweight.

Decoded PCM SHOULD be written into the FLACHEAD PCM buffer rather than processed by the UI or Playback Engine directly.

---

# 8. Streaming Requirement

FLAC files MUST be decoded incrementally.

Correct:

    File → Read small chunk → Decode → PCM buffer → Repeat

Incorrect:

    File → Load entire file → Decode entire file → Store PCM → Playback

The second approach is prohibited because it creates unnecessary memory pressure on the Pi Zero W.

FLAC file size must not determine decoder memory consumption.

A multi-gigabyte FLAC must remain streamable.

---

# 9. File Reader

The File Reader is responsible for providing sequential compressed data to the decoder.

Responsibilities include:

- opening the file
- reading compressed bytes
- seeking compressed/source positions when required
- reporting I/O errors
- closing the file

It SHOULD use buffered file I/O.

It SHOULD avoid:

- reading one byte at a time through expensive system calls
- loading entire files
- unnecessary file seeking
- repeated open/close operations

The reader SHOULD provide sufficiently large sequential reads to reduce filesystem overhead while keeping memory bounded.

## 9.1 File Read Buffer

The decoder/library MAY use a file-read buffer.

Its size MUST be bounded.

It should be tuned based on actual SD-card performance.

Tiny file reads can increase system-call overhead.

Very large reads can increase memory use and reduce responsiveness.

## 9.2 Storage Read-Ahead

Limited read-ahead may be used to reduce storage latency impact.

Never allow compressed-data prefetch to grow without limits.

Blocking filesystem reads should not occur inside a real-time-sensitive audio callback.

## 9.3 File Caching

The operating system's file cache may provide useful read buffering.

Do not automatically implement a large custom file cache without profiling.

---

# 10. Offline-First Design

The decoder must not require network access.

All normal playback must work without:

- internet
- remote metadata
- streaming services
- cloud APIs

The primary storage medium is the FLACHEAD SD card.

The decoder must tolerate realistic SD-card variability.

## 10.1 SD Card Considerations

The Raspberry Pi Zero W may use removable SD storage.

The decoder MUST tolerate:

- filesystem latency
- temporary read delays
- removable media errors
- variable SD-card read latency

Prebuffering SHOULD absorb normal filesystem latency.

The decoder MUST NOT assume desktop-class storage performance.

The PCM ring buffer provides protection against short delays.

The decoder should therefore stay sufficiently ahead of playback.

## 10.2 SD-Card Corruption

Filesystem or storage corruption should surface as a controlled file/decoder error.

---

# 11. File Validation

Before decoding, FLACHEAD MUST verify that the file is accessible.

Minimum checks:

- file exists
- file is readable
- file is not a directory
- file can be opened
- file has valid FLAC identification
- basic metadata is valid

A failed validation MUST produce a controlled error.

The application MUST NOT crash.

## 11.1 FLAC Identification

The decoder MUST verify the FLAC stream marker before treating the file as FLAC data.

A file with an invalid FLAC signature MUST be rejected as invalid or unsupported.

The decoder MUST NOT blindly interpret arbitrary file data as FLAC.

## 11.2 Validation On Open

When opening a track:

1. Validate path.
2. Open file.
3. Validate FLAC container.
4. Read STREAMINFO.
5. Read relevant metadata.
6. Determine source format.
7. Determine total sample count if available.
8. Prepare decoder.
9. Enter READY.

---

# 12. Metadata Parsing

FLAC metadata blocks appear before audio frames.

The decoder MUST parse the metadata required to correctly decode the stream.

At minimum, STREAMINFO MUST be handled.

## 12.1 Metadata Blocks

The decoder may encounter metadata such as:

- STREAMINFO
- VORBIS_COMMENT
- PICTURE
- SEEKTABLE
- CUESHEET
- APPLICATION
- PADDING

The decoder layer should only extract information required for decoding or explicitly expose metadata to the metadata subsystem.

## 12.2 STREAMINFO

STREAMINFO is authoritative for fundamental stream properties.

The decoder MUST obtain:

- minimum block size
- maximum block size
- minimum frame size
- maximum frame size
- sample rate
- channel count
- bits per sample
- total samples
- MD5 signature where available

These values determine the PCM format and duration.

Duration can be calculated conceptually as:

    duration = total_samples / sample_rate

The implementation MUST avoid floating-point accumulation errors when maintaining playback position.

## 12.3 SEEKTABLE

If a FLAC file contains a SEEKTABLE, the decoder SHOULD use it.

If it does not, the decoder MAY perform an approximate seek followed by frame scanning.

The system MUST prioritize correctness over absolute seek speed.

Playback must still function correctly without a SEEKTABLE.

## 12.4 Vorbis Comments

Vorbis comments may provide:

- title
- artist
- album
- album artist
- genre
- date
- track number
- disc number

The metadata subsystem may consume these.

VORBIS_COMMENT belongs primarily to the metadata subsystem.

The decoder may expose it without owning its presentation.

## 12.5 Metadata Ownership

The playback metadata subsystem should own application-level metadata.

The decoder should not become the permanent owner of UI metadata.

The decoder SHOULD provide metadata through a separate metadata interface or object.

Do not make the Playback Engine responsible for parsing arbitrary Vorbis comments.

Persistent metadata such as:

- title
- artist
- album
- genre
- artwork
- lyrics

belongs to the library/metadata layer.

## 12.6 Metadata Timing

Metadata should preferably be read before playback begins.

Metadata required for playback should be processed during track initialization.

After the audio path starts, nonessential metadata processing should occur asynchronously.

Large metadata blocks must not delay real-time audio once playback is running.

## 12.7 Metadata Caching

Repeated metadata extraction may be cached by the library/indexing subsystem.

The implementation should avoid opening/parsing the same file repeatedly when the required information is already available.

If library metadata already contains sample rate, channels, bits per sample, and duration, the Playback Engine MAY use cached metadata for UI purposes.

However, the decoder MUST still verify the actual opened stream before relying on it for output configuration.

Cached library metadata is not authoritative for the actual audio stream.

The opened file is authoritative.

---

# 13. Source Format

The decoder MUST expose a source-format object containing at least:

- codec = FLAC
- sample rate
- channels
- channel layout where known
- bits per sample
- sample representation
- total samples
- total frames
- duration
- lossless = true

The Playback Engine uses this information for output negotiation.

Conceptually:

    AudioFormat
        sample_rate
        channels
        bits_per_sample
        sample_format
        total_samples
        total_frames

---

# 14. Sample Rate

The decoder MUST preserve the source sample rate.

Examples:

- 44.1 kHz → 44.1 kHz PCM
- 48 kHz → 48 kHz PCM
- 88.2 kHz → 88.2 kHz PCM
- 96 kHz → 96 kHz PCM
- 192 kHz → 192 kHz PCM when supported by the complete output pipeline

The implementation must not assume 44.1 kHz.

Resampling belongs downstream of decoding.

The FLAC decoder MUST NOT silently resample.

## 14.1 Sample Rate Validation

The source sample rate must be validated before timing or buffer calculations.

---

# 15. Bit Depth Preservation

The decoder MUST preserve the source bit depth.

Examples:

- 16-bit FLAC → 16-bit PCM
- 24-bit FLAC → 24-bit PCM

The decoder MUST NOT reduce 24-bit → 16-bit unless an explicit downstream conversion is required.

The decoder itself SHOULD produce source-equivalent PCM.

24-bit FLAC MUST be handled without reducing it unnecessarily.

The decoder MUST NOT reduce 24-bit sources to 16-bit merely for convenience.

The decoder MUST NOT silently discard lower-order sample information.

---

# 16. Channel Preservation

The decoder MUST preserve source channel layout as supported by the decoder.

Stereo MUST remain stereo.

Mono MUST remain mono unless the output layer explicitly requires another representation.

The decoder MUST NOT perform unnecessary mixing.

The decoder MUST NOT downmix unless explicitly designed as a separate processing stage.

## 16.1 Channel Order

Channel ordering MUST follow the FLAC specification and decoder-library contract.

For stereo:

- channel 0 = left
- channel 1 = right

Any transformation must be explicit.

The output layer must use a documented channel order.

## 16.2 Mono and Multi-Channel

Mono files MUST decode correctly.

Stereo files MUST decode correctly.

Multi-channel files SHOULD be supported by the decoder where possible.

If the selected output device only supports stereo, a clearly defined channel-conversion policy MUST be applied.

The system MUST NOT silently discard channels.

The primary music-player target should prioritize stereo music playback.

If downmixing is eventually supported, it must occur in a clearly defined audio-processing stage. It must not be hidden inside the decoder abstraction.

## 16.3 Channel Validation

The channel count must be validated before downstream buffer sizing.

---

# 17. Sample Order and Representation

## 17.1 Sample Order

Decoded samples must remain in chronological order.

The decoder MUST preserve exact sample order.

If source contains: L0 R0 / L1 R1 / L2 R2, output must remain: L0 R0 / L1 R1 / L2 R2.

No samples may be dropped or duplicated.

## 17.2 PCM Representation

The decoder must expose a defined PCM representation.

Possible representations include:

- signed integer PCM
- fixed-width integer samples
- interleaved PCM
- planar PCM

The chosen representation must be documented by the implementation.

A compact signed-integer PCM representation is generally preferable on the Pi Zero W when compatible with the output backend.

## 17.3 Interleaved PCM

If interleaved PCM is selected, frame layout must remain deterministic.

Stereo example: L R L R L R ...

## 17.4 Planar PCM

If planar PCM is ever used, channel buffers must have explicit ownership and lifetime rules. The initial implementation should avoid unnecessary complexity.

## 17.5 Integer PCM

Integer PCM is preferred when the output pipeline naturally supports it.

## 17.6 Float PCM

Floating-point PCM may be used when required by the chosen audio API or processing pipeline.

FLAC decoding itself should not be forced through floating-point PCM unless the selected decoder architecture requires it.

For integer FLAC sources, integer PCM is generally preferable.

## 17.7 No Unnecessary Conversion

Do not convert integer → float → integer unless required.

The decoder should not convert integer FLAC PCM to floating point unless a later processing stage genuinely requires it.

## 17.8 Signedness

Signedness must be explicitly interpreted.

---

# 18. FLAC Frame Decoding

Audio is stored in FLAC frames.

The decoder processes:

    FLAC Frame → Frame Header → Subframe → Residual → Reconstruction → PCM Samples

Each decoded frame contributes PCM samples to the output buffer.

A FLAC frame contains compressed audio information for a sequence of samples.

The decoder converts the frame into PCM.

A decoder frame represents one synchronized sample across all channels.

For stereo, one frame contains: LEFT sample + RIGHT sample.

## 18.1 FLAC Format Specifics

The decoder MUST correctly handle:

- frame headers
- block sizes
- sample rate codes
- channel assignments
- sample sizes
- subframes
- fixed prediction
- linear prediction
- residual coding
- Rice coding
- channel decorrelation
- CRC validation where supported by the decoder implementation

## 18.2 Frame Order

FLAC frames must be decoded sequentially during normal playback.

Seeking may reposition the decoder to an appropriate FLAC frame and sample offset.

The playback engine must not assume that UI-level seek positions correspond directly to FLAC frame boundaries.

## 18.3 Frame Accounting

The decoder must track the number of PCM frames produced.

## 18.4 Decoder Loop

Conceptually:

    while playback is active:
        read compressed FLAC frame
        decode frame
        produce PCM
        publish PCM to buffer

---

# 19. PCM Production

## 19.1 Decode Unit

The decoder SHOULD operate in PCM frames or blocks.

It SHOULD NOT require the Playback Engine to understand FLAC frame boundaries.

The Playback Engine only sees PCM.

## 19.2 Output Contract

Every successful decode operation MUST report:

- PCM pointer/buffer
- number of frames
- source format
- end-of-stream information if relevant

The PCM memory ownership MUST be explicit.

## 19.3 Decoder Output Chunk

The decoder should produce PCM in bounded chunks.

Chunk size should be compatible with the downstream ring buffer and conversion pipeline.

## 19.4 Partial Decode

A decode call may produce fewer frames than requested. This is normal.

The caller MUST use the returned frame count.

It MUST NOT assume: requested_frames == produced_frames.

## 19.5 Final Decode Block

The last decode operation may contain fewer frames than normal.

Example: Requested 1024 frames, Available 317 frames. The decoder returns 317 frames, then eventually: EOF.

---

# 20. Format Processing Stage

## 20.1 Format Conversion

Format conversion belongs to the audio format-processing layer.

It should not be hidden inside unrelated decoder code.

The decoder should report the source format.

The audio pipeline determines whether conversion is necessary.

If the source format differs from the negotiated output format, conversion occurs after decoding.

If source and output formats match, the decoder output may be passed directly to the output-buffer producer.

## 20.2 No Hidden Conversion

The decoder must not silently resample or change precision merely to make the output backend easier.

Any transformation must be explicit and traceable.

## 20.3 Resampling

Resampling should not occur unless required.

If the output device accepts the source sample rate, prefer direct playback at that rate.

If resampling is required, use a well-defined resampling stage. Do not implement an ad-hoc resampler.

Resampling can significantly increase CPU usage on the Pi Zero W. It must therefore be benchmarked on actual hardware.

The decoder MUST never hide the resampling operation from the pipeline.

## 20.4 Channel Conversion

Channel conversion should occur only when necessary.

## 20.5 Downstream Conversion State

If resampling exists, its state must be reset appropriately after seeking.

Any stateful format converter must be reset after seeking.

If future DSP/filter stages are added, stateful filters must define seek reset behavior.

## 20.6 No Giant PCM Allocation

The decoder must not allocate the entire track as PCM.

---

# 21. Decoder and DSP Separation

The decoder MUST NOT know whether the BUNNY DSP is present.

The decoder only produces PCM.

The downstream pipeline determines whether:

- PCM → Direct Output

or:

- PCM → ReplayGain → DSP → Output

The TANCHJIM BUNNY DSP is an external audio device.

The decoder should not assume DSP-specific behavior.

The decoder produces source PCM.

The output path determines how that PCM reaches the external DSP.

This separation prevents device-specific logic from contaminating the decoder.

---

# 22. Decoder and ALSA Separation

The decoder MUST NOT include ALSA-specific code.

Incorrect: FLACDecoder → ALSA

Correct: FLACDecoder → PCM Buffer → AudioOutputBackend → ALSA

This allows the decoder to be tested independently.

---

# 23. Decoder and UI Separation

The decoder MUST NOT:

- call UI functions
- update widgets
- render artwork
- render lyrics
- access screen state
- read touchscreen input

It only reports decoding state and PCM.

---

# 24. ReplayGain Metadata

FLAC/Vorbis comments may contain ReplayGain information.

The decoder MAY expose this metadata.

It MUST NOT apply gain itself.

ReplayGain processing belongs to the audio-processing layer.

Correct separation: FLAC → Decoder → PCM → ReplayGain Processor → Output

Incorrect: FLAC → Decoder silently modifies samples → PCM

---

# 25. Artwork

Embedded album artwork MUST NOT be decoded inside the real-time audio decoder path.

Artwork processing belongs to the artwork/resource system.

Audio decoding SHOULD expose the artwork reference or metadata needed by the library system.

The audio output thread MUST never process artwork.

If artwork is extracted from FLAC metadata, perform that work outside the audio output path.

Large image blocks MUST NOT delay PCM production unnecessarily.

---

# 26. Lyrics

Lyrics MUST NOT be parsed by the decoder's real-time path.

Lyrics belong to the metadata/lyrics system.

The decoder only needs to provide accurate audio position.

Lyrics synchronization uses: Audio Output Position → Lyrics Synchronization System

---

# 27. Memory Management

## 27.1 Memory Allocation

The normal decoding path SHOULD NOT allocate memory for every PCM block.

Preferred: Initialize → Allocate bounded pool → Reuse blocks → Playback → Release/recycle blocks

Avoid: decode() → malloc() → copy → free() → repeat

Repeated heap allocation can increase CPU overhead and fragmentation.

## 27.2 Decoder Memory Limits

Memory usage MUST remain bounded.

The decoder MUST NOT:

- cache complete tracks
- retain unlimited metadata
- create unlimited PCM blocks
- retain decoded frames unnecessarily

The decoder SHOULD release temporary resources as soon as they are no longer required.

## 27.3 Metadata Memory

FLAC metadata may contain arbitrary application-specific blocks.

The decoder MUST NOT blindly allocate enormous amounts of memory based on metadata sizes.

Metadata required for playback SHOULD be extracted selectively.

Large metadata blocks SHOULD be bounded or skipped when they are not required.

## 27.4 Memory Consumption

Memory consumption SHOULD depend primarily on:

- decoder state
- FLAC frame/block size
- output block size
- PCM ring buffer

It MUST NOT depend linearly on total track duration.

## 27.5 Compressed vs PCM Buffer

The compressed input buffer and PCM output buffer are separate resources.

Compressed FLAC data is small relative to decoded PCM.

PCM buffering is handled by the audio buffering system.

## 27.6 Decoder Internal Buffer

The decoder MAY maintain an internal temporary PCM buffer.

Its size MUST be bounded.

It MUST NOT grow indefinitely based on track size.

## 27.7 Backpressure

If the PCM buffer is full, the decoder MUST stop producing PCM temporarily.

It MUST NOT:

- allocate unlimited memory
- overwrite unread samples
- discard decoded audio

Backpressure prevents:

- excessive CPU usage
- unnecessary PCM allocation
- memory growth
- wasted decoding work

When PCM is sufficiently buffered, decoder work may pause or reduce activity.

## 27.8 PCM Block Reuse

Reusable PCM blocks are preferred over repeatedly allocating/freeing buffers.

A bounded PCM buffer pool may be used.

Pool size must remain bounded.

If no PCM block is available, decoder should wait or apply backpressure. Never allocate indefinitely.

---

# 28. PCM Ownership

Ownership MUST be explicit.

Recommended: Decoder produces PCM Block → owned by PCM Buffer Manager → consumed by Audio Output Backend

The decoder MUST NOT retain ownership of PCM blocks after handing them to the buffer manager.

The output backend MUST NOT modify source PCM unless an explicitly configured processing stage requires it.

The decoder MUST NOT expose a pointer that becomes invalid while the output system is using it.

Each PCM block must have clear ownership.

A block should exist only until consumed, discarded after seek, or invalidated after track change.

---

# 29. Memory Copy Optimization

The implementation SHOULD minimize FLAC data copies, PCM copies, and buffer copies.

Prefer: Decoder → Reusable PCM block → Ring Buffer → Output

rather than repeatedly copying the same PCM data between temporary buffers.

Where practical, move PCM through the pipeline without unnecessary copying.

If copying is required: Decoder buffer → memcpy / equivalent → PCM ring buffer

The copy SHOULD be sequential and efficient. The number of copies should be minimized.

If copying simplifies correctness and remains inexpensive on the Pi Zero W, correctness takes priority over premature zero-copy optimization.

Zero-copy may be considered if the decoder library exposes stable PCM buffers.

However, zero-copy MUST NOT compromise ownership, lifetime, thread safety, buffer reuse, or seek correctness.

A simple copy into the ring buffer is preferable to a fragile zero-copy design.

---

# 30. Decoder Thread

Decoding MUST occur outside the UI thread.

The UI MUST never block waiting for a FLAC frame to decode.

Recommended: Playback Control → Decoder Worker → PCM Ring Buffer

The decoder worker SHOULD sleep when the PCM buffer is sufficiently full.

It SHOULD wake when the buffer reaches the low-water threshold.

This prevents unnecessary CPU consumption.

## 30.1 Thread Ownership

Decoding should normally occur on the playback/decoder worker rather than the UI thread.

The UI thread must never synchronously decode a FLAC frame.

## 30.2 Decoder Worker

The decoder worker may perform:

- file reads
- decoder calls
- PCM production
- buffer writes
- seek operations

## 30.3 Decoder Priority

The decoder must receive sufficient CPU time to maintain the PCM buffer.

However, it must not monopolize the Pi Zero W.

The decoder is important but does not have the same timing requirements as the final audio output path.

The actual audio-output path has higher real-time importance than decoder throughput.

## 30.4 File I/O Thread

Blocking filesystem reads should not occur inside a real-time-sensitive audio callback.

File I/O must remain outside the UI thread.

## 30.5 Audio Path Locking

The real-time output path should avoid contended locks wherever possible.

## 30.6 Thread Safety

The implementation must not assume that a decoder instance is thread-safe.

Only the designated decoder thread/control context should directly manipulate decoder state unless the selected decoder library explicitly guarantees thread safety.

---

# 31. Decoder Scheduling

The decoder MUST NOT continuously consume CPU while there is sufficient buffered PCM.

Recommended logic:

    if buffer < LOW_WATERMARK: decode
    if buffer >= HIGH_WATERMARK: wait

This is especially important on Raspberry Pi Zero W.

The decoder should run frequently enough to keep the PCM buffer above the low watermark.

It should yield when:

- buffer is sufficiently full
- playback is paused
- stopping is requested
- the current session becomes obsolete

## 31.1 Avoid Busy Waiting

The decoder MUST NOT continuously poll: while(buffer_full) {}

Instead it SHOULD:

- block appropriately
- wait on an event
- yield
- use condition-variable signaling
- use an efficient producer/consumer mechanism

## 31.2 No Busy Loop

The decoder must not consume an entire CPU core by continuously polling when PCM buffering is already sufficient.

When the buffer is sufficiently full, the decoder should wait efficiently.

The decoder should resume when:

- buffer space becomes available
- a seek occurs
- a track change occurs
- shutdown occurs

---

# 32. PCM Block Size

The decoder SHOULD produce PCM in bounded blocks.

Block size SHOULD balance:

- CPU overhead
- memory usage
- latency
- output requirements
- decoder efficiency
- ring-buffer efficiency
- scheduling

Very small blocks increase function-call and synchronization overhead.

Very large blocks increase latency and memory usage.

The final block size MUST be benchmarked on the actual Pi Zero W.

A reasonable starting point is a moderate number of PCM frames per decode operation. The exact number MUST NOT be treated as a permanent requirement.

Benchmark: 256 frames, 512 frames, 1024 frames, larger values if necessary on the Pi Zero W.

---

# 33. Prebuffering

Before starting playback, the decoder may fill a minimum amount of PCM data.

Prebuffering reduces the chance that initial SD-card or decoder latency causes an immediate underrun.

Excessive prebuffering increases startup latency.

Choose prebuffer levels through measurement on the Raspberry Pi Zero W.

---

# 34. PCM Buffer Occupancy

The decoder should monitor available PCM capacity.

If the PCM buffer is full: decoder production should throttle.

If the PCM buffer becomes empty while playback requires audio: an underrun occurs.

The playback engine should:

- record the underrun
- attempt controlled recovery
- avoid crashing

---

# 35. Seeking

## 35.1 Seek Overview

FLAC seeking SHOULD use the decoder's native seeking capabilities where available.

General process: Seek Request → Calculate target → Decoder seek → Discard previous PCM → Decode from new location → Refill buffer → Resume output

The decoder MUST NOT allow stale PCM from before the seek to remain in the output buffer.

## 35.2 Seek Process

A seek should conceptually perform:

requested_time → requested_sample → nearest usable FLAC position → decoder reposition → discard pre-target samples → produce target PCM

## 35.3 Seek Unit

Public seek operations should normally use time or PCM sample/frame position.

## 35.4 Time to Sample

For constant sample rate: target_sample = target_time × sample_rate

## 35.5 Rounding

The implementation must define how fractional sample positions are rounded.

Seek conversions must define their rounding behavior.

## 35.6 Seek Table

If a FLAC stream contains a seek table, the decoder should use it to accelerate seeking.

If no seek table exists, the decoder may need to use available frame/index information or sequential decoding from a suitable earlier position.

If a FLAC SEEKTABLE exists, the decoder/library may use it to improve seek efficiency.

Playback must still function correctly without a SEEKTABLE.

## 35.7 Seek Accuracy

The final playback position MUST correspond as closely as possible to the requested target.

The decoder MUST expose the actual resulting sample position if exact seeking is not possible.

FLAC seeking may locate a nearby frame rather than the exact target sample. The decoder/playback layer must compensate by decoding forward to the requested sample position where necessary.

## 35.8 Seek Precision

The final decoded position must be corrected to the requested sample/time as accurately as the decoder allows.

## 35.9 Seek Discard

If decoding begins before the requested sample, discard samples before the exact target.

No samples belonging to the old playback position must ever reach the output after a seek.

## 35.10 Seek Flush

Any previously decoded PCM must be invalidated after a seek.

## 35.11 Seek Workflow

Recommended: Playback Engine → stop/flush output → flush PCM buffer → decoder.seek(target) → decode forward → buffer refill → resume output

The decoder MUST NOT independently restart playback.

The complete seek workflow: invalidate playback generation → stop/flush audio output → clear PCM buffer → request decoder seek → decoder locates appropriate FLAC frame → decoder resumes decoding → discard samples before exact target if required → publish new PCM → prebuffer → resume output.

## 35.12 Seek Safety

During seek:

- old PCM becomes invalid
- decoder state changes
- buffer is flushed
- output position is reset
- new PCM is generated

A seek operation MUST be serialized with decoder operations.

Two simultaneous decoder seeks MUST NOT occur.

## 35.13 Seek Optimization

For local FLAC files, seeking SHOULD use available seek points or efficient frame navigation.

The decoder SHOULD avoid scanning from byte zero for every seek unless unavoidable.

If the selected decoder library supports seek tables, FLACHEAD SHOULD take advantage of them.

## 35.14 Seek Performance

Seeking performance depends on:

- SEEKTABLE availability
- file size
- decoder implementation
- SD-card performance
- storage latency
- distance from seek point
- target position

The implementation SHOULD benchmark seeks on the actual SD card and Pi Zero W.

## 35.15 Seek Failure

If a seek fails, the playback engine must receive a structured failure.

It must not continue from an unknown position while claiming the requested position was reached.

## 35.16 Seek While Decoding

A seek request may arrive while the decoder is decoding.

The decoder MUST have a cancellation/invalidation mechanism.

The current decode operation must not continue filling the buffer with pre-seek PCM after the seek becomes authoritative.

## 35.17 Decoder Reset After Seek

If the selected library requires decoder reset/reinitialization after seek, the implementation must perform it safely.

After seek: reset frame state → reset PCM conversion state → discard old output → reposition sample state.

## 35.18 No Full Re-decode

A seek should not require decoding the entire track from the beginning unless the format/library makes that unavoidable.

---

# 36. Playback Generations

## 36.1 Decoder Generation

Each decoder context should logically belong to an active playback generation.

Each decoder session should logically belong to an active playback generation.

Each decoder instance receives a generation identifier.

## 36.2 Stale Decoder Output

If a decoder operation finishes after a seek/track change, its output must not be published to the active PCM buffer.

If generations differ, discard the decoded data.

## 36.3 Publish Rule

Decoded PCM may only be published if: decoder_generation == active_generation.

## 36.4 Track Change

A track change must create a new decoder context or otherwise invalidate the previous decoder context.

On track change: invalidate old decoder generation → stop old decoding → close/release old decoder → open new file → initialize new decoder → negotiate format → initialize conversion → refill buffer.

## 36.5 Stale Decoder

A stale decoder should exit as soon as practical.

---

# 37. Decoder End-of-Stream

## 37.1 EOF Handling

When the final FLAC frame has been decoded:

Decoder state: EOF

The decoder MUST signal the Playback Engine.

The PCM buffer may still contain audio.

Therefore: Decoder EOF ≠ Audio playback ended

Playback ends only after all decoded PCM has been consumed by the output device.

EOF does not mean the last sample has been audibly played.

The decoder reaching EOF does not mean the last audible sample has already played.

The playback engine must account for buffered/output audio.

## 37.2 Normal EOF

At normal EOF:

- no more PCM is produced
- decoder reports end-of-stream
- already buffered PCM remains valid
- playback engine waits for output drain

## 37.3 Drain Handling

At end-of-stream: Decoder → EOF → PCM Buffer contains remaining audio → Output drains remaining PCM

Only after the output is drained should the Playback Engine transition to track completion.

The playback engine considers the track complete only after: decoder EOF + converter drained + ring buffer drained + backend completion policy satisfied.

## 37.4 Conversion EOF

The decoder reaching EOF does not necessarily mean the output pipeline is immediately finished.

The converter/resampler may still have pending output.

## 37.5 EOF vs Error

The decoder must explicitly distinguish END OF FILE from DECODER ERROR and from temporary no-data state.

When EOF occurs, decoder.eof() = true. No additional PCM will be produced.

However, previously decoded PCM may remain in the Playback Engine's buffer.

The Playback Engine handles final draining.

EOF is a normal stream condition and must be distinguished from decoder failure.

---

# 38. Decoder Error Model

## 38.1 Error Categories

Minimum decoder errors:

- DECODER_FILE_OPEN_FAILED
- DECODER_INVALID_FLAC
- DECODER_INVALID_METADATA
- DECODER_UNSUPPORTED_STREAM
- DECODER_CORRUPT_FRAME
- DECODER_CRC_ERROR
- DECODER_SEEK_FAILED
- DECODER_OUT_OF_MEMORY
- DECODER_IO_ERROR
- DECODER_EOF

Additional useful categories:

- FILE_NOT_FOUND
- FILE_READ_FAILED
- CORRUPT_METADATA
- DECODER_INTERNAL_ERROR
- DECODER_INITIALIZATION_FAILED
- STREAM_INFO_INVALID

## 38.2 Decode Error

If decoding fails, the decoder MUST report a structured error.

Fields MAY include:

- file
- decoder status
- frame position
- sample position
- error message
- session ID
- error category
- generation

## 38.3 Error Propagation

Decoder: Error → Playback Engine → Media Session → UI

The decoder MUST NOT display UI notifications itself.

The UI should receive a high-level error.

Developer diagnostics may expose detailed decoder information.

## 38.4 Error Isolation

A decoder failure MUST NOT directly terminate the UI process.

Decoder errors must not crash the entire application.

A decoder failure must not corrupt unrelated playback state.

The error should propagate: Decoder → Playback Engine → Application State → UI

## 38.5 User Error Display

Low-level decoder diagnostics should not necessarily be shown directly to the user.

The UI receives a simplified message such as: "Unable to play this track."

The logging subsystem receives technical details. The UI receives a simplified playback error.

## 38.6 Cancellation Is Not an Error

If the decoder stops because NEXT, STOP, SEEK, or SHUTDOWN was requested, this MUST NOT be reported as a playback failure.

It is an expected control transition.

## 38.7 Decoder Error Model

If decoding fails, the decoder MUST return a controlled decoding error.

It MUST NOT:

- crash
- access invalid memory
- continue indefinitely
- generate unbounded output

Depending on decoder capabilities, FLACHEAD MAY attempt controlled recovery.

However, silent corruption MUST NOT be presented as successful playback when the decoder knows the stream is invalid.

## 38.8 Corrupt Frame Handling

If a FLAC frame is corrupted, the decoder should report the failure according to the capabilities of the selected library.

Possible policy: stop track, or attempt controlled recovery if the decoder library supports it.

The default behavior SHOULD prioritize audio correctness over playing damaged data.

## 38.9 Recovery Policy

The initial implementation should prioritize correctness.

Do not silently skip corrupted audio unless explicit recovery behavior has been designed and tested.

Automatic recovery MUST NOT silently produce altered audio.

If recovery is attempted:

- it must be supported by the decoder
- the behavior must be deterministic
- diagnostics must record it

## 38.10 CRC Errors

CRC errors SHOULD be reported by the decoder where available.

The system SHOULD distinguish warning/recoverable corruption from fatal corruption.

Diagnostics SHOULD contain: track, frame/position if available, error type.

The user-facing UI SHOULD remain concise.

## 38.11 I/O Errors

If reading the FLAC file fails, the decoder MUST stop safely.

Examples: SD card error, filesystem error, file removed, read failure.

The application MUST remain running.

Playback Engine SHOULD attempt recovery only when the failure is plausibly transient.

If a filesystem read fails, the decoder MUST report an I/O error. It MUST NOT interpret missing bytes as valid FLAC data.

## 38.12 Partial File

A truncated FLAC should be treated as incomplete/corrupt when the decoder reaches invalid or missing data.

## 38.13 CRC Validation

FLAC provides integrity mechanisms such as frame CRCs.

The selected decoder should perform the appropriate validation.

---

# 39. File Removal During Playback

If the current FLAC file disappears while playing, the decoder SHOULD continue using already buffered data if possible.

Once further reads fail, decoder reports I/O error. Playback Engine handles the failure.

The application MUST NOT crash.

## 39.1 File Deletion

If a file is deleted after opening, Linux may allow the open file descriptor to continue reading.

The decoder should continue if data remains accessible.

If reading eventually fails, report an I/O error.

## 39.2 File Replacement

If a file changes while playing, the current decoder SHOULD continue using its currently opened stream.

The library can detect the change separately.

A future playback attempt SHOULD reopen the current version.

The decoder MUST NOT unexpectedly restart because metadata changed externally.

A library rescan should not silently replace the active decoder state.

---

# 40. Compression Level

FLAC compression level affects decoding CPU usage and file size.

The decoder MUST support normal FLAC compression levels.

The Playback Engine MUST NOT assume that high-compression FLAC is always inexpensive to decode.

Performance testing SHOULD include high-compression files.

Higher FLAC compression may require more decoding CPU.

The implementation must be benchmarked with representative files.

---

# 41. High Sample Rate

The decoder MUST support high sample-rate FLAC where the selected decoder library supports it.

Examples: 96 kHz, 192 kHz.

The complete system MUST separately validate whether the output device supports the resulting format.

The decoder MUST NOT silently downgrade the source.

High sample-rate files MUST NOT automatically be rejected merely because they are uncommon.

If the hardware/output pipeline supports them and the Pi can sustain decoding, they should be playable.

High-resolution FLAC should be benchmarked separately.

---

# 42. High Bit Depth

24-bit FLAC MUST be handled without reducing it unnecessarily.

The decoder SHOULD output the source bit depth.

Any later conversion must occur in a clearly defined downstream stage.

When the downstream pipeline supports the source resolution, the decoder must preserve it.

---

# 43. Lossless Guarantee

FLAC decoding itself MUST be mathematically lossless.

The decoder MUST NOT apply:

- EQ
- compression
- normalization
- limiting
- volume
- resampling

unless these are explicitly performed by downstream stages.

The decoder's responsibility ends at PCM reconstruction.

---

# 44. Bit-Perfect Boundary

The decoding stage should conceptually be:

FLAC → Exact PCM reconstruction → PCM Pipeline

Any modification after that point belongs to the processing/output layer.

This allows the system to determine whether the complete pipeline is actually bit-perfect.

---

# 45. Duration

Duration SHOULD be calculated from STREAMINFO when total sample count is available.

Conceptually: duration = total_samples / sample_rate

The Playback Engine SHOULD maintain position using integer sample/frame counters internally where practical.

The UI may represent the resulting value as milliseconds.

The authoritative duration should be derived from sample counts rather than rounded UI time.

---

# 46. Position Representation

The decoder SHOULD internally represent position using samples or another exact stream-relative representation.

Avoid using repeatedly accumulated floating-point seconds as the authoritative decoder position.

Preferred: sample_position → converted when necessary to milliseconds, seconds, UI timecode.

Time should be derived from source_frame_position / sample_rate.

Avoid accumulating floating-point time deltas for authoritative position tracking.

Prefer integer/sample-based timing internally to avoid cumulative floating-point drift.

## 46.1 Byte Offset

Compressed byte position may be useful internally for seeking.

It must not be treated as equivalent to audio time.

## 46.2 Decoder Position vs Audible Position

The decoder provides progress information.

The playback/output layer determines the authoritative audible position.

Decoder position alone is not necessarily equal to currently audible position.

Buffered PCM may exist after the decoder position.

The playback clock must account for buffered-but-not-yet-consumed PCM where required.

---

# 47. Thread Synchronization

Decoder and output communicate through the PCM buffer.

The synchronization mechanism SHOULD minimize contention.

The decoder MUST NOT hold a global application mutex while decoding.

The output thread MUST NOT wait indefinitely for the decoder.

If no PCM is available, output reports underrun or waits according to backend semantics.

Synchronization must avoid unnecessary global locks.

Use bounded lock-free or low-contention structures where justified by profiling.

Do not introduce complex lock-free code without a demonstrated need.

Correctness is more important than theoretical lock-free performance.

---

# 48. Decoder Shutdown

Shutdown sequence:

    Stop decoder requests
        ↓
    Finish/abort current decode operation
        ↓
    Stop file reads
        ↓
    Release decoder
        ↓
    Release file
        ↓
    Recycle buffers
        ↓
    Exit worker

Shutdown MUST be safe even if the decoder is:

- buffering
- seeking
- waiting for data
- at EOF
- reporting an error

Recommended shutdown order: signal decoder stop → wake decoder if blocked → wait for decoder worker → invalidate PCM production → close decoder → release file.

## 48.1 Thread Lifetime

A decoder worker MUST NOT outlive the Playback Session that owns it.

A decoder worker must not outlive the playback engine without an explicit ownership model.

No detached decoder.

Before destroying decoder resources: request stop → wake decoder → join worker → destroy decoder.

## 48.2 No Use-After-Free

Decoder resources must not be destroyed while a decoder worker can still access them.

## 48.3 Close Order

Recommended: stop decoder → ensure decoder no longer accesses file → destroy decoder context → close file.

The decoder must never access a file after its handle has been closed.

---

# 49. Decoder Cancellation

Long decoder operations SHOULD be cancellable.

Example: User selects another track while decoding.

The current decoder should receive CANCEL rather than forcing the UI to wait for the current operation to finish.

Cancellation MUST leave the decoder in a safe state before destruction.

The decoder loop SHOULD periodically check: stop_requested, session_valid, seek_requested, shutdown_requested.

This allows responsive control.

Cancellation should occur within a bounded amount of time.

The decoder SHOULD NOT perform an enormous decode operation that prevents STOP, SEEK, or NEXT from being processed promptly.

## 49.1 Track Change Cancellation

When a new track is loaded, old decoder work should stop as quickly as safely possible.

## 49.2 Seek Cancellation

When a new seek supersedes an old seek, the older operation must not publish stale PCM.

## 49.3 Shutdown Cancellation

Shutdown must stop decoder work before releasing shared resources.

---

# 50. Track Switching

When switching tracks:

    Current Decoder → Cancel / drain according to transition → Release → Next Decoder → Prebuffer

For gapless playback, a second decoder context MAY be prepared before the first is destroyed.

## 50.1 Decoder Lifecycle

Normal lifecycle: CREATED → OPENING → READING_METADATA → READY → DECODING → EOF → CLOSING → CLOSED

Error path: ANY STATE → ERROR → CLOSING → CLOSED

## 50.2 Decoder State Machine

Minimum states: CLOSED, OPENING, READY, DECODING, SEEKING, EOF, ERROR

Additional state: DRAINING (downstream stages may still be consuming the final decoded data)

The decoder MUST NOT expose ambiguous combinations such as READY + ERROR or CLOSED + DECODING.

The implementation must reject invalid state transitions.

## 50.3 Pause

Pause should not necessarily destroy decoder state.

Pause should not necessarily destroy decoder state.

Resume should continue from the current decoder position unless a seek or other invalidation occurred.

Once sufficient PCM is buffered, the decoder should not continue decoding indefinitely while playback is paused.

## 50.4 Stop

Stop invalidates the decoder generation.

On stop, decoder production must terminate or become invalid for the current generation.

## 50.5 Decoder Reuse

A decoder instance MAY be reused for another track if the underlying library supports safe reset.

However, reuse MUST NOT allow state leakage.

For simplicity, destroying and recreating the decoder between tracks is acceptable if performance is sufficient.

On the Pi Zero W, simpler resource lifecycle may be preferable to aggressive object reuse.

Do not optimize object allocation at the expense of correctness.

## 50.6 Safe Default

Create a clean decoder context per track.

A new track should generally create a fresh decoder context.

## 50.7 Reinit After Error

After a decoder error, the failed decoder context must not be reused unless the decoder library explicitly guarantees safe recovery.

---

# 51. Gapless Playback

If gapless playback is implemented, the decoder architecture must support preparing the next decoder without allowing it to publish PCM into the active buffer until the playback engine authorizes the transition.

The decoder should support preparing the next compatible stream before the current stream completely finishes when the higher-level playback engine enables this.

Optional future optimization: prepare the next decoder context while the current track is playing.

Only prepare the immediately relevant next track. Do not decode the queue in advance.

Pre-decoding consumes CPU and memory. It should be enabled only if measurements demonstrate meaningful transition improvements.

---

# 52. Crossfade

Crossfade is outside the baseline decoder responsibility.

If implemented later, it belongs to a higher-level audio mixing stage.

---

# 53. Decoder Callbacks

If the selected FLAC library uses callbacks, callbacks must remain lightweight and deterministic.

Decoder callbacks may:

- provide compressed data
- receive decoded PCM
- report metadata
- report errors

They must not perform UI work.

Avoid unnecessary allocation inside decoder callbacks.

Callback errors must be translated into decoder state/errors.

## 53.1 PCM Callback

If the library provides decoded PCM through a callback, the callback should write into the PCM buffer or an intermediate block.

## 53.2 Callback Backpressure

If the callback can block safely, it may wait for PCM buffer space.

If the library requires non-blocking callbacks, the architecture must provide an appropriate staging mechanism.

## 53.3 Callback Contract

The exact callback behavior must be documented when the library is selected.

---

# 54. Decoder Resource Ownership

The decoder owns:

- decoder context
- input file handle where applicable
- decoder-specific temporary resources

## 54.1 Playback Engine Ownership

The playback engine owns:

- active playback generation
- track selection
- decoder lifecycle requests
- PCM buffer coordination

## 54.2 File Handle

A file handle must remain valid for the decoder's entire active session.

The active track may keep its file descriptor open throughout playback.

The descriptor must be closed when the decoder is destroyed or the track is abandoned.

## 54.3 Resource Cleanup

Cleanup must occur when:

- normal playback ends
- user stops
- user changes track
- seek replaces decoder state
- an error occurs
- application shuts down

Every decoder initialization path must have a matching cleanup path.

If initialization fails after opening the file, the file must still be closed.

If decoding fails, all associated resources must be released after the playback engine decides how to handle the failure.

Regardless of implementation language or library, all decoder resources must have deterministic cleanup.

## 54.4 Exception/RAII Safety

The implementation must use deterministic cleanup. RAII is preferred where applicable.

If implemented in C++, decoder handles should be wrapped in ownership-aware types. Raw ownership should be minimized.

## 54.5 No Resource Leak

Repeated track changes must not leak file descriptors, decoder objects, buffers, or conversion state.

---

# 55. Callback Memory

Avoid unnecessary allocation inside decoder callbacks.

Decoder callbacks must never directly trigger UI rendering.

Decoder callbacks must not wait on UI operations.

---

# 56. Security

FLAC files are untrusted input.

The decoder MUST safely handle:

- malformed headers
- invalid block sizes
- invalid sample rates
- invalid channel assignments
- malformed metadata
- invalid frame sizes
- corrupted frames
- maliciously large metadata
- truncated files

The decoder implementation SHOULD rely on mature parsing code wherever possible.

## 56.1 No Silent Data Loss

The decoder MUST NOT silently:

- drop channels
- truncate samples
- change sample rate
- reduce bit depth
- skip corrupted sections without reporting them

Any transformation must be explicit and traceable.

## 56.2 Path Security

The decoder should receive validated filesystem paths from the library/music subsystem.

It should not construct arbitrary paths from untrusted metadata.

## 56.3 Stream Limits

Resource limits should be enforced by the overall audio system where appropriate.

## 56.4 Watchdog

If the decoder becomes stuck unexpectedly, the playback system should have a mechanism for detecting abnormal inactivity.

Timeouts must not be implemented in a way that incorrectly kills valid long-running decode operations. They should target actual stalled states.

A decoder stall should generate diagnostics.

If recovery is impossible, the playback engine should terminate the current track safely.

---

# 57. CPU Optimization

Decoder optimization priorities:

1. Maintain reliable PCM production.
2. Avoid unnecessary memory copies.
3. Reuse buffers.
4. Avoid unnecessary synchronization.
5. Avoid unnecessary metadata processing.
6. Avoid decoding when buffer is full.
7. Use efficient library APIs.
8. Benchmark actual Pi Zero W behavior.

Do NOT trade lossless correctness for CPU savings.

## 57.1 Decoder Efficiency

The decoder SHOULD minimize:

- memory allocations
- data copies
- unnecessary format conversions
- metadata parsing during playback
- filesystem seeks

## 57.2 Steady-State Allocation

Avoid allocations in the steady-state decode loop.

Necessary allocations may occur during track initialization.

Once playback is running, allocation activity should be minimal and predictable.

---

# 58. Decoder Performance

The decoder MUST be benchmarked on Raspberry Pi Zero W.

Required measurements:

- average decoder CPU
- peak decoder CPU
- memory usage
- decode throughput
- PCM production rate
- buffer occupancy
- underrun count

Tests MUST include:

- 44.1 kHz / 16-bit
- 48 kHz / 16-bit
- 44.1 kHz / 24-bit
- 48 kHz / 24-bit
- 96 kHz / 24-bit
- 192 kHz / 24-bit if supported
- high-compression FLAC
- low-compression FLAC

## 58.1 Real-Time Margin

The decoder MUST maintain sufficient throughput above the audio consumption rate.

Conceptually: decode_rate > playback_consumption_rate

There MUST be enough margin to tolerate short filesystem or CPU delays.

The decoder SHOULD NOT operate permanently at the exact minimum throughput.

A decoder that runs at 1.01x real-time is technically faster but not robust.

The system SHOULD target a comfortable margin above real-time.

The exact target MUST be measured on Pi Zero W.

## 58.2 Decode Throughput

The decoder should be capable of decoding faster than real-time.

Define: decode_speed = decoded_audio_seconds / wall_clock_seconds

For reliable playback: decode_speed > 1.0 with sufficient margin.

## 58.3 PCM Throughput

PCM data rate is approximately: sample_rate × channels × bytes_per_sample

Example: 44100 × 2 × 3 = 264600 bytes/sec for packed 24-bit stereo.

At 96 kHz / 24-bit stereo: 96000 × 2 × 3 = 576000 bytes/sec.

This affects memory bandwidth, buffer sizing, output writes, and processing cost.

## 58.4 CPU Budget

The decoder MUST be benchmarked under the worst expected playback formats.

At minimum: 44.1 kHz / 16-bit, 44.1 kHz / 24-bit, 96 kHz / 24-bit, 192 kHz / 24-bit if supported by the target output path.

The goal is sustained real-time decoding with margin.

## 58.5 Worst-Case Format

The system MUST NOT be considered stable solely because CD-quality FLAC works.

High-resolution sources may produce substantially higher PCM throughput.

The Pi Zero W must be tested under representative worst-case files.

## 58.6 Worst-Case Performance

If 1 second of audio requires 1.1 seconds of CPU time, the decoder cannot sustain playback.

A large buffer can temporarily hide slow decoding. It cannot make a permanently slower-than-real-time decoder sustainable.

## 58.7 Benchmark

A decoder benchmark SHOULD report: Track, sample rate, bit depth, channels, file size, duration, decode time, real-time factor, CPU usage, peak memory.

This makes performance comparisons meaningful.

## 58.8 Multitasking

Benchmark decoding while: UI rendering, touchscreen input, album artwork display, lyrics synchronization, filesystem activity.

## 58.9 Performance Testing

Tests SHOULD measure: cold file cache, warm file cache, slow SD conditions, high CPU load, UI activity, library scanning, artwork loading, lyrics display, high-resolution FLAC.

---

# 59. Memory Alignment

The implementation SHOULD use sensible alignment for PCM buffers.

Do not introduce platform-specific alignment requirements unless measurements demonstrate a benefit.

## 59.1 ARMv6 Compatibility

Potential optimization areas include: reducing copies, reducing allocations, sequential memory access, avoiding floating-point conversion, efficient buffer operations, minimizing synchronization overhead.

The implementation MUST first establish correctness.

The decoder MUST be optimized for ARMv6 without assuming modern ARM instructions.

The implementation MUST NOT require ARMv7, ARMv8, NEON, or 64-bit-only instructions unless a separate optional implementation explicitly detects and supports them.

Any architecture-specific optimization must be verified for ARMv6 compatibility.

Compiler optimizations must not sacrifice decoder correctness.

The decoder should be compiled with the project's established ARMv6 toolchain configuration.

Cross-compiled decoder binaries must be tested on actual Pi Zero W hardware.

---

# 60. Metadata Scan vs Playback Decode

The music library may need to inspect thousands of FLAC files.

Metadata scanning SHOULD use a separate metadata reader path.

The Playback Decoder should be optimized for active playback, not library indexing.

The scanner may read: STREAMINFO, Vorbis comments, embedded picture metadata.

The active Playback Decoder should only open the selected track.

---

# 61. Logging

Normal successful decoding SHOULD produce no per-frame logs.

Debug logs MAY include:

- file opened
- format detected
- metadata parsed
- seek
- decoder error
- EOF
- decode performance

Per-frame logging is prohibited in production playback because it can consume excessive CPU and storage bandwidth.

Never log individual decoded samples during normal operation.

---

# 62. Diagnostics

Developer diagnostics SHOULD expose:

- file path or track identifier
- generation
- codec
- sample rate
- channels
- bit depth
- total samples
- current sample
- decoded frames
- decode CPU time
- decode throughput
- EOF state
- error state
- seek capability
- seek latency
- buffer fill
- decoder CPU
- decoder state
- decoder errors

The diagnostics MUST be optional.

Diagnostics should be disabled or reduced in normal production operation where they would create unnecessary overhead.

Useful counters: decoded_frames, decode_calls, decode_time, seek_count, seek_time, decoder_errors, eof_count.

---

# 63. Testing Architecture

Decoder tests SHOULD be independent of the audio output device.

Test: FLAC → Decoder → PCM → Verify

This allows exact PCM output to be compared against known-good reference data.

## 63.1 PCM Verification

For known test FLAC files, the decoder SHOULD be validated against reference PCM.

Validation SHOULD verify: sample count, channel count, sample values, bit depth, sample rate.

A correct FLAC decoder should reconstruct the original PCM exactly.

## 63.2 Regression Tests

Every decoder bug that is discovered SHOULD produce a regression test.

Example: Corrupt frame bug → Minimal reproducing file → Automated test → Permanent regression coverage.

This prevents future optimization from reintroducing decoder failures.

## 63.3 Bit-Perfect Verification

For bit-perfect mode, decoded PCM SHOULD be compared against a trusted reference decoder where practical.

The purpose is to verify sample values, channel ordering, sample count, sample rate, bit depth.

## 63.4 Reference Testing

A reference decoder can generate expected PCM.

FLACHEAD output: Decoder A → PCM A

Reference: Decoder B → PCM B

Compare: PCM A == PCM B for representative test files.

## 63.5 Decoder Verification

A test suite SHOULD include known FLAC files with:

- silence
- impulse
- sine wave
- stereo channel identification
- low amplitude
- full-scale samples
- 16-bit
- 24-bit
- different sample rates

These reveal decoding and channel-order bugs.

## 63.6 Test File Set

The test suite should contain:

- normal 16-bit FLAC
- normal 24-bit FLAC
- multiple sample rates
- multiple compression levels
- long tracks
- short tracks
- files with metadata
- files without optional metadata
- files with SEEKTABLE
- files without SEEKTABLE
- corrupted files
- truncated files

---

# 64. Hardware Validation

Final validation MUST be performed on Raspberry Pi Zero W with TANCHJIM BUNNY DSP using real FLAC files.

Desktop success is not sufficient.

Desktop performance MUST NOT be used as proof of Pi Zero W performance.

All performance acceptance tests must be executed on actual Raspberry Pi Zero W hardware.

End-to-end playback must be tested through the actual TANCHJIM BUNNY DSP.

---

# 65. Acceptance Criteria

The FLAC decoding pipeline is complete when:

- FLAC files open reliably
- STREAMINFO is parsed correctly
- source sample rate is preserved
- source bit depth is preserved
- channel count is preserved
- PCM is produced correctly
- decoding is streaming
- entire files are never loaded into RAM
- normal playback does not continuously allocate memory
- decoder CPU usage is measurable
- decoder works asynchronously
- seeking works
- seek flushes stale PCM
- EOF is correctly distinguished from output completion
- corrupt files fail safely
- missing files fail safely
- malformed metadata cannot crash the application
- high-resolution FLAC is handled correctly where supported
- decoder remains independent of ALSA
- decoder remains independent of UI
- decoder remains independent of queue management
- output-device changes do not require decoder redesign
- reference PCM tests pass
- Pi Zero W hardware testing passes
- valid FLAC files decode correctly
- source format is preserved
- sample count is correct
- channel order is correct
- EOF is reliable
- errors are structured
- cancellation works
- memory is bounded
- no playback thread leaks occur
- decode speed is sufficient on Pi Zero W
- high-resolution files are handled according to policy
- decoder behavior remains independent of the UI
- FLAC decoding is isolated from queue logic
- source PCM format is explicit
- source sample rate is validated
- source channel count is validated
- source bit depth is preserved where supported
- decoder output is chunked
- decoder output feeds the conversion pipeline
- output PCM is eventually written to the ring buffer
- decoder state is generation-aware
- stale decoders cannot publish PCM
- seek is supported correctly
- seek flushes downstream state
- converter draining is handled after decoder EOF
- decoder errors propagate cleanly
- malformed FLAC cannot crash the application
- file descriptors are released correctly
- decoder resources are released correctly
- repeated track changes do not leak resources
- decoder performance is sufficient for real-time playback
- high-resolution FLAC is benchmarked
- ARMv6 compatibility is verified
- actual Pi Zero W testing is performed
- actual TANCHJIM BUNNY DSP playback is tested
- decoder memory remains bounded regardless of track length
- decoding occurs off the UI thread
- decoder backpressure works
- stable PCM production remains sufficient under multitasking
- real hardware testing succeeds
- format transitions are clean

---

# 66. Acceptance Tests

## 66.1 Format Test

Play all supported source formats. Verify: correct sample rate, correct channel count, correct bit depth.

## 66.2 Lossless Test

Decode known test vectors. Verify exact PCM equivalence.

## 66.3 Seek Test

Perform seeks across the entire track. Verify correct audio position.

## 66.4 Seek After Pause Test

Pause. Seek. Resume. Verify correct target audio.

## 66.5 Rapid Seek Test

Perform rapid consecutive seeks. Verify only the final valid seek state produces audio.

## 66.6 Track Change Test

Change tracks during decoding. Verify old PCM cannot leak into the new track.

## 66.7 Buffer Test

Monitor PCM buffer occupancy during normal playback. Verify it remains within expected bounds.

## 66.8 Underrun Test

Introduce controlled storage/CPU delays. Verify underruns are detected and handled without crashing.

## 66.9 Long Playback Test

Play continuously for several hours. Verify decoder stability and resource stability.

## 66.10 Metadata Test

Load FLAC files with standard metadata, large metadata, embedded artwork, unusual comments. Verify playback remains reliable.

## 66.11 Corruption Test

Test corrupted and truncated FLAC files. Verify controlled failure.

## 66.12 Shutdown Test

Terminate playback during decoding, seeking, buffering, track initialization. Verify clean resource release.

## 66.13 Seek Verification Tests

Seek tests SHOULD verify: seek target, actual sample position, first output samples, subsequent sample continuity.

No duplicate or skipped samples should occur because of seek handling.

Test seeking near 0%, 25%, 50%, 75%, 99%, 100% of a long FLAC file.

This catches seek-index and end-of-file bugs.

## 66.14 Track Transition Testing

Test Track A → Track B where same format, different sample rate, different bit depth, different duration, different channel configuration where supported.

The decoder must reset correctly every time.

## 66.15 Resource Leak Testing

Repeatedly open, decode, close the same track. Monitor RAM, file descriptors, threads, decoder objects. The counts should remain stable.

## 66.16 Repeated Seek Testing

Perform hundreds of seeks during one track. Monitor memory, CPU, decoder state, PCM correctness, output continuity. No unbounded resource growth is acceptable.

## 66.17 Seek Stress

Perform many random seeks during playback. Verify no stale audio, no crashes, no memory growth, no decoder corruption.

## 66.18 Track Switch Stress

Rapidly switch between different FLAC formats. Example: 44.1 kHz → 48 kHz → 96 kHz → 44.1 kHz.

Verify output reconfiguration remains correct.

## 66.19 Format Switch

Changing format must not leave the output device configured incorrectly.

Verify actual output behavior when consecutive tracks use different sample rates.

Verify transitions between different source bit depths.

Verify mono/stereo transitions if supported.

## 66.20 Long-Run Test

Decode/play a large collection continuously. Verify no memory leak, no descriptor leak, no progressive latency increase, no decoder corruption.

## 66.21 Storage Stress

Test playback while the filesystem is under realistic load.

## 66.22 UI Stress

Navigate the UI heavily during decoding. Verify audio remains stable.

## 66.23 Lyrics Stress

Load and synchronize lyrics during decoding. Verify audio remains stable.

## 66.24 Artwork Stress

Load large artwork during decoding. Verify audio remains stable.

## 66.25 CPU Stress

Measure decoder behavior while the system is under realistic CPU load.

## 66.26 Mixed Queue Test

Play: 44.1 kHz / 16-bit, then 48 kHz / 24-bit, then 96 kHz / 24-bit.

Verify format transitions are clean.

## 66.27 Individual Unit Tests

- Open a known-good FLAC. Verify decoder reaches READY.
- Attempt to open a non-FLAC file. Verify controlled initialization failure.
- Decode a corrupted FLAC. Verify decoder error without process crash.
- Decode stereo 16-bit FLAC. Verify sample format and output PCM.
- Decode stereo 24-bit FLAC. Verify no unintended truncation.
- Decode mono FLAC. Verify channel count and conversion behavior.
- Decode 44.1 kHz source. Verify correct sample-rate reporting.
- Decode 48 kHz source. Verify correct sample-rate reporting.
- Decode a high-resolution FLAC. Verify stable memory usage and acceptable CPU.
- Seek repeatedly. Verify correct source position.
- Seek while buffered playback is active. Verify old PCM never reaches output.
- Change tracks rapidly. Verify old decoder is safely invalidated.
- Force a generation change while decoding. Verify stale PCM is discarded.
- Reach normal EOF. Verify decoder reports completion without error.
- Open and close many tracks. Verify no file-descriptor or memory leaks.
- Decode/play continuously for several hours. Verify stable memory and CPU behavior.

---

# 67. AI Coding Agent Rules

The coding agent MUST:

1. Treat FLAC → PCM as the decoder's only core responsibility.
2. Use a mature FLAC decoder library.
3. Never implement a custom FLAC decoder unless explicitly required.
4. Hide the selected decoder library behind FLACHEAD's decoder abstraction.
5. Keep decoding separate from UI.
6. Keep decoding separate from queue management/playlist logic.
7. Keep decoding separate from lyrics.
8. Keep decoding separate from artwork.
9. Keep decoding separate from database logic.
10. Keep decoding separate from audio-device logic/volume policy.
11. Keep decoder responsibilities limited to audio decoding.
12. Decode FLAC incrementally/streaming.
13. Never load an entire FLAC into memory.
14. Never load an entire track as PCM.
15. Keep decoder memory bounded regardless of track length.
16. Maintain bounded compressed-input memory.
17. Maintain bounded PCM output memory.
18. Preserve source sample rate.
19. Preserve source bit depth.
20. Preserve source channel count.
21. Preserve channel ordering.
22. Never resample inside the decoder.
23. Never apply volume inside the decoder.
24. Never apply ReplayGain inside the decoder.
25. Never mix channels inside the decoder.
26. Never assume 44.1 kHz.
27. Never assume stereo.
28. Never assume 16-bit audio.
29. Avoid unnecessary resampling.
30. Avoid unnecessary format conversion.
31. Avoid unnecessary channel conversion.
32. Avoid unnecessary floating-point conversion.
33. Avoid unnecessary PCM copies.
34. Reuse buffers.
35. Avoid allocations in the steady-state decode loop.
36. Avoid unnecessary allocations during steady-state decoding.
37. Keep decoder work outside the real-time audio callback.
38. Never block the UI thread.
39. Never decode FLAC on the UI thread.
40. Support cancellation.
41. Support seeking.
42. Distinguish EOF from errors.
43. Distinguish cancellation from errors.
44. Report decoder errors.
45. Handle malformed files safely.
46. Handle corrupted FLAC safely.
47. Handle invalid FLAC safely.
48. Handle missing files safely.
49. Handle seek failures safely.
50. Handle truncated files safely.
51. Handle underruns safely.
52. Use seek tables when available.
53. Correct seek positions to the requested sample/time.
54. Discard samples before the requested seek position.
55. Flush stale PCM after seeking.
56. Invalidate decoder output after track changes.
57. Use playback generations to reject stale work.
58. Do not allow decoder operations from previous tracks to publish PCM.
59. Prevent stale decoder threads from publishing PCM.
60. Never allow stale decoder output into a new playback session.
61. Apply PCM-buffer backpressure.
62. Do not decode indefinitely while the buffer is full.
63. Validate decoder output before buffer allocation.
64. Validate sample rate.
65. Validate channel count.
66. Validate sample depth.
67. Protect all memory calculations from overflow.
68. Keep filesystem access outside the audio callback.
69. Keep decoder operations outside the audio callback.
70. Avoid busy-waiting.
71. Avoid unbounded buffering.
72. Avoid unnecessary permanent threads.
73. Make decoder resources deterministic and leak-free.
74. Keep decoder resource ownership explicit.
75. Release decoder resources deterministically.
76. Do not allow a decoder worker to outlive its owner.
77. Never destroy decoder state while a worker can still access it.
78. Preserve the separation between decoding, buffering, and output.
79. Do not assume the TANCHJIM BUNNY DSP inside the decoder.
80. Treat the decoder as a reusable PCM producer.
81. Never silence resampling.
82. Never silent DSP.
83. Never silent channel conversion.
84. Never silent bit-depth reduction.
85. Keep decoder timing separate from UI timing.
86. Use frame/sample positions for audio positioning.
87. Never treat compressed byte offsets as audio time.
88. Use decoder-supported seek mechanisms.
89. Flush downstream PCM during seek.
90. Invalidate old decoder generations during seek.
91. Allow downstream converter draining after EOF.
92. Release file descriptors correctly.
93. Release decoder resources correctly.
94. Allow decoder callbacks to be lightweight.
95. Do not perform UI operations from decoder callbacks.
96. Do not perform expensive metadata operations during real-time decoding.
97. Avoid per-frame logging.
98. Avoid per-frame heap allocation.
99. Keep decoder callbacks lightweight.
100. Keep decoder workers lifecycle-safe.
101. Do not add network dependencies.
102. Do not invent hardware capabilities.
103. Benchmark decoder CPU usage.
104. Benchmark high-resolution FLAC.
105. Benchmark mixed-format playback.
106. Benchmark long-duration playback.
107. Test multiple FLAC compression levels.
108. Test multiple sample rates.
109. Test supported bit depths.
110. Test long-duration playback.
111. Test corrupted files.
112. Test repeated seeks.
113. Test shutdown races.
114. Test decoder behavior under CPU pressure.
115. Verify ARMv6 compatibility.
116. Test on actual Raspberry Pi Zero W hardware.
117. Test through the TANCHJIM BUNNY DSP.
118. Verify decoded PCM against reference data where practical.
119. Do not sacrifice audio correctness for premature optimization.
120. Do not sacrifice lossless correctness for CPU savings.
121. Measure decoder CPU and memory usage before declaring the implementation optimized.
122. Do not allow decoder failures to crash the entire player.

---

# 68. Final Architecture

The intended production pipeline is:

                         FLAC FILE
                             │
                             ▼
                       FILE READER
                             │
                             ▼
                    COMPRESSED FLAC DATA
                             │
                             ▼
                     FLAC STREAM PARSER
                             │
                             ▼
                       STREAMINFO
                             │
                             ▼
                      FLAC DECODER
                             │
                             ▼
                       EXACT PCM
                             │
                             ▼
                    FORMAT PROCESSING
                   (optional / bypassable)
                             │
                             ▼
                       PCM BUFFER
                             │
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

Control:

                     PLAYBACK ENGINE
                             │
                 ┌───────────┼───────────┐
                 │           │           │
               OPEN         SEEK       STOP
                 │           │           │
                 ▼           ▼           ▼
              DECODER     DECODER    INVALIDATE
                 │           │           │
                 └───────────┼───────────┘
                             │
                       GENERATION ID

Control API:

    Playback Engine
          │
          ├── open()
          ├── decode()
          ├── seek()
          ├── stop()
          ├── cancel()
          └── close()

---

# 69. Final Invariants

The central invariant is:

THE DECODER CONVERTS FLAC TO PCM; IT DOES NOT CONTROL PLAYBACK POLICY.

The second invariant is:

SOURCE AUDIO INFORMATION MUST BE PRESERVED UNTIL A DOWNSTREAM STAGE EXPLICITLY REQUIRES CONVERSION.

The third invariant is:

NO STALE PCM MAY REACH THE AUDIO OUTPUT AFTER A SEEK OR TRACK CHANGE.

The fourth invariant is:

THE DECODER MUST NEVER BLOCK THE REAL-TIME AUDIO OUTPUT PATH.

The fifth invariant is:

ALL DECODER MEMORY AND FILE RESOURCES MUST REMAIN BOUNDED AND DETERMINISTIC.

The sixth invariant is:

DECODER FAILURE MUST NEVER CRASH THE APPLICATION.

The seventh invariant is:

DECODING MUST REMAIN FASTER THAN REAL-TIME WITH SUFFICIENT MARGIN ON THE RASPBERRY PI ZERO W.

The eighth invariant is:

THE DECODER MUST REMAIN INDEPENDENT OF THE TANCHJIM BUNNY DSP AND THE SPECIFIC AUDIO OUTPUT BACKEND.

The ninth invariant is:

EOF MEANS NO MORE SOURCE AUDIO; IT DOES NOT AUTOMATICALLY MEAN THE LAST SAMPLE HAS BEEN AUDIBLY PLAYED.

The tenth invariant is:

MALFORMED FLAC INPUT MUST NOT BE ABLE TO CRASH THE PLAYER.

---

# 70. Final Responsibility Separation

FILE READER = PROVIDES COMPRESSED FLAC DATA

FLAC DECODER = COMPRESSED FLAC → SOURCE PCM

FORMAT CONVERTER = SOURCE PCM → OUTPUT PCM

PCM BUFFER = BOUNDED PCM STORAGE / REAL-TIME HANDOFF

AUDIO BACKEND = CONSUMES OUTPUT PCM / PCM → AUDIO DEVICE

PLAYBACK ENGINE = OWNS TRACK, GENERATION, SEEK, STOP, AND DECODER LIFECYCLE / EXECUTION AND STATE

PLAYBACK CONTROLLER = PLAYBACK POLICY

TANCHJIM BUNNY DSP = FINAL EXTERNAL AUDIO PROCESSING/OUTPUT DEVICE

And always preserve the separation:

FLAC FILE = COMPRESSED SOURCE

FILE READER = STORAGE ACCESS

FLAC DECODER = LOSSLESS DECOMPRESSION

PCM = DECODED AUDIO

FORMAT PROCESSOR = REQUIRED CONVERSION ONLY

PCM BUFFER = REAL-TIME HANDOFF

AUDIO OUTPUT = HARDWARE DELIVERY

PLAYBACK ENGINE = EXECUTION AND STATE

PLAYBACK CONTROLLER = PLAYBACK POLICY

---

The decoder's fundamental responsibility is simple:

Take valid FLAC data and reconstruct its PCM audio accurately, efficiently, and safely.

Everything else — volume, ReplayGain, DSP, device management, UI synchronization, queue management, and presentation — belongs outside the decoder.

The Raspberry Pi Zero W must be treated as a constrained real-time system throughout the implementation.

This architecture keeps FLAC decoding deterministic, memory-bounded, generation-safe, and independent from the rest of FLACHEAD while providing the correct source PCM to the downstream format-conversion and buffering pipeline.
