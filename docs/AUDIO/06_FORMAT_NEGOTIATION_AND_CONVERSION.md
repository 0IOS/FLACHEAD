# FLACHEAD FORMAT NEGOTIATION AND CONVERSION

Document

06_FORMAT_NEGOTIATION_AND_CONVERSION.md

Category

Audio System

Status

Production Specification

Target Hardware

- Raspberry Pi Zero W
- ARMv6
- Raspberry Pi OS
- TANCHJIM BUNNY DSP
- Offline FLAC playback
- 2.8" portrait TFT touchscreen

---

# 1. PURPOSE

This document defines how FLACHEAD determines, validates, preserves, converts, and delivers PCM audio formats between:

- source FLAC files
- decoded PCM
- optional processing stages
- output audio backend
- TANCHJIM BUNNY DSP.

The purpose is to ensure that:

- decoded audio remains correct
- sample formats are handled explicitly
- channel layouts are understood
- sample rates are handled safely
- conversions are predictable and observable
- unnecessary processing is avoided
- memory usage remains suitable for the Raspberry Pi Zero W
- the TANCHJIM BUNNY DSP receives a valid supported PCM stream
- bit-perfect output is the default when hardware permits.

The system MUST ensure that the audio pipeline does not accidentally:

- reduce bit depth
- alter sample rate
- change channel layout
- reinterpret PCM bytes incorrectly
- introduce unnecessary resampling
- introduce unnecessary quantization
- silently convert formats
- silently discard channels
- introduce hidden DSP processing.

---

# 2. CORE PRINCIPLE

The preferred path is:

SOURCE FORMAT
    ↓
IDENTICAL OUTPUT FORMAT

If the BUNNY DSP supports the source format, FLACHEAD SHOULD send the source format directly.

The fundamental rule is:

DO NOT MODIFY AUDIO FORMAT UNLESS THE OUTPUT PATH ACTUALLY REQUIRES IT OR THE USER EXPLICITLY ENABLES A PROCESSING FEATURE.

Bit-perfect passthrough is the default behavior. Conversion is a compatibility mechanism, not the default behavior.

---

# 3. FORMAT TRIPLE DISTINCTION

The audio pipeline must explicitly distinguish:

SOURCE FORMAT

from:

INTERNAL PCM FORMAT

from:

OUTPUT FORMAT.

These formats must never be assumed to be identical.

SOURCE FORMAT ≠ INTERNAL PCM FORMAT ≠ OUTPUT FORMAT

This distinction is essential for FLACHEAD's high-quality FLAC playback goal and for making the final audio pipeline measurable, debuggable, and predictable on the Raspberry Pi Zero W.

---

# 4. WHY FORMAT NEGOTIATION EXISTS

A FLAC file can contain many different PCM formats.

Examples:

44.1 kHz / 16-bit / stereo

48 kHz / 16-bit / stereo

44.1 kHz / 24-bit / stereo

96 kHz / 24-bit / stereo

192 kHz / 24-bit / stereo

The output device may not support every possible combination.

Therefore:

FLAC Source
    ↓
Decoder Format
    ↓
Device Capability Check
    ↓
Output Format
    ↓
Audio Device

The decoder and output device MUST NOT assume that their formats are always identical.

---

# 5. PRIORITY ORDER

The system MUST prioritize:

1. Correct playback.
2. Preservation of source quality.
3. Bit-perfect output when possible.
4. Minimal unnecessary processing.
5. Low CPU usage on Raspberry Pi Zero W.
6. Deterministic behavior.

When choosing an output format, priority SHOULD be:

1. Explicit user mode.
2. Bit-perfect requirement.
3. Source format.
4. Device capability.
5. Compatibility policy.
6. Performance constraints.

The system MUST produce deterministic results.

---

# 6. FORMAT OBJECT AND REPRESENTATION

FLACHEAD MUST represent PCM format explicitly using a canonical format object.

Conceptually:

AudioFormat

    sample_rate
    channels
    channel_layout
    sample_format
    valid_bits
    container_bits
    interleaving
    signedness
    endianness
    bytes_per_sample
    bytes_per_frame
    codec/source information where useful

The exact C++ representation is implementation-specific.

The meaning MUST remain consistent throughout the audio system.

Every active audio stream MUST explicitly define:

- sample rate
- channel count
- channel layout
- sample format
- valid bit depth
- container/storage width
- signedness
- endianness
- interleaving model

The implementation SHOULD have a canonical format object used by:

- decoder
- PCM buffer
- processor
- output
- diagnostics

This avoids duplicated format assumptions.

---

# 7. FORMAT COMPARISON AND IDENTITY

The system SHOULD have a way to determine whether two formats are equivalent.

Conceptually:

format_a == format_b

should answer whether the formats are directly compatible.

Formats should be compared structurally. Do not compare only sample rate.

A format may optionally have a compact identifier/hash for efficient comparison.

For two formats to be directly compatible they must share:

same sample rate

+

same channels

+

same layout

+

same sample format

+

same storage representation.

---

# 8. SOURCE FORMAT

The decoder provides the source format.

The decoder produces the source format as the input to format negotiation.

Source format properties include:

- sample rate
- channel count
- channel layout
- sample depth
- signedness
- sample representation
- bit depth
- PCM sample representation
- channel ordering.

Example:

Source:

codec = FLAC
sample_rate = 44100
channels = 2
bits_per_sample = 24

The Playback Session SHOULD retain the source format.

source_format:

    sample_rate = 96000
    channels = 2
    bit_depth = 24

---

# 9. FLAC SPECIFICITY

FLAC is a lossless compressed representation.

FLAC decoding should reproduce the encoded PCM samples exactly.

The decoder stage should not alter them.

Any subsequent transformation must happen because the output architecture requires it.

The playback engine must never send compressed FLAC frame data directly to a PCM output backend.

The decoder must produce valid PCM first.

The `.flac` extension MUST NOT be treated as sufficient proof of format.

The decoder must inspect the actual FLAC stream.

FLAC metadata may contain:

- sample rate
- bit depth
- channels
- tags
- artwork
- cuesheet information

The audio decoder's actual stream parameters remain authoritative for PCM decoding.

If metadata and decoded stream properties disagree:

the playback pipeline must trust validated decoder state.

---

# 10. INTERNAL CANONICAL FORMAT

FLACHEAD MUST define one canonical internal PCM representation for the audio processing pipeline.

This reduces:

- conversion overhead
- code complexity
- memory copies
- format bugs

The internal format must be:

- efficient on ARMv6
- lossless when no intentional conversion is required
- easy to buffer
- easy to pass between pipeline stages
- explicitly documented.

The final internal representation should be selected based on:

- decoder behavior
- CPU cost on ARMv6
- memory footprint
- DSP/output compatibility
- required precision
- conversion requirements
- output device compatibility
- bit-perfect requirements.

It MUST be benchmarked on the Pi Zero W.

A canonical internal format MUST NOT force unnecessary conversion of every FLAC file.

The canonical format must be documented in the implementation configuration.

Example conceptual configuration:

INTERNAL_PCM_FORMAT

    signed integer
    interleaved
    native-endian
    configured sample width.

The exact production representation is implementation-defined until hardware benchmarking is complete.

Integer PCM should remain integer throughout the pipeline where possible.

For a Pi Zero W, integer PCM is generally preferable when the complete path can remain integer.

Floating-point PCM MAY be used where required by DSP or resampling.

It MUST NOT be introduced unnecessarily.

If the internal representation uses floating point and output requires integer PCM:

conversion MUST explicitly define:

- scaling
- rounding
- clipping
- optional dithering.

No arbitrary reinterpretation is permitted.

When converting integer PCM to floating point:

the scaling convention MUST be explicit.

For example:

full-scale integer range

must map consistently into the chosen floating-point range.

---

# 11. OUTPUT FORMAT AND DEVICE CAPABILITIES

The output backend reports what the physical device supports.

Conceptually:

AudioDeviceCapabilities

- supported_sample_rates
- supported_sample_formats
- supported_channel_counts
- supported_channel_layouts
- supported_access_modes

The negotiation layer compares the source against these capabilities.

A requested format can be:

SUPPORTED

but:

not currently configured.

The output system must distinguish these concepts.

The output format should be determined from:

source format

+

processing policy

+

device capabilities.

It should not be determined by the UI.

---

# 12. FORMAT NEGOTIATION

Before playback begins, the system determines:

1. source format
2. supported output formats
3. required conversion
4. resulting output format
5. buffer frame size
6. clock parameters.

The decoder determines the source format.

The output subsystem determines which formats it can accept.

The playback engine must establish a compatible format between them.

Format negotiation should produce an immutable playback-format description for the active generation.

Negotiation happens once per track open.

Once playback begins, the active format descriptor should not be modified in place.

A format change creates a new playback configuration.

The negotiated output format must be known before PCM enters the final output buffer.

---

# 13. NEGOTIATION LIFECYCLE

The normal lifecycle is:

Open Track
    ↓
Decode STREAMINFO
    ↓
Create Source Format
    ↓
Query Output Capabilities
    ↓
Negotiate
    ↓
Create Conversion Pipeline if needed
    ↓
Configure Output
    ↓
Start Playback

---

# 14. EXACT MATCH

If:

source format ∈ device capabilities

then:

selected output format = source format

This is the preferred outcome.

No conversion should occur.

Example:

Source:

44.1 kHz
24-bit
stereo

Device:

44.1 kHz
24-bit
stereo supported

Result:

44.1 kHz
24-bit
stereo

No conversion.

---

# 15. PARTIAL MATCH

A device may support:

44.1 kHz / 16-bit

but source is:

44.1 kHz / 24-bit

The system MUST determine whether:

- the device supports a compatible 24-bit container
- explicit bit-depth conversion is required
- playback should be rejected in bit-perfect mode

The system MUST NOT silently choose a lower-quality format.

---

# 16. FORMAT NEGOTIATION ORDER

Preferred order:

1. exact source format
2. source rate with compatible sample format
3. required sample-rate conversion
4. required sample-format conversion
5. required channel conversion
6. failure if no valid path exists.

If multiple valid paths exist:

choose the path with the fewest transformations.

If two paths require the same number of transformations:

prefer the path with lower quality loss.

If quality is equivalent:

prefer the lower CPU-cost path.

If CPU cost is comparable:

prefer the lower-latency path.

The selected path should remain stable throughout the track (determinism).

---

# 17. NEGOTIATION RESULT

The negotiation result is represented conceptually as:

AudioSessionFormat

- source_format
- output_format
- conversion_required
- resampling_required
- bit_depth_conversion_required
- channel_conversion_required
- bit_perfect
- device_name

This makes the active audio path inspectable.

The audio system SHOULD expose a conversion state.

Possible values:

DIRECT

RESAMPLED

BIT_DEPTH_CONVERTED

CHANNEL_CONVERTED

FORMAT_CONVERTED

UNSUPPORTED

---

# 18. SAMPLE RATE

Sample rate specifies the number of samples per channel per second.

Common values include:

44100

48000

88200

96000

176400

192000

The format negotiation layer MUST treat sample rate as an exact property.

It MUST NOT approximate:

44100 → 44000

or:

48000 → 47999

without explicit conversion.

The implementation must not assume 44100 Hz for every track.

---

# 19. SAMPLE RATE PRESERVATION

If the complete hardware/software path supports the source rate:

source rate MUST be preserved.

If the audio device supports the source sample rate directly:

prefer native playback.

Example:

FLAC:

96 kHz

Supported output:

96 kHz

Result:

96 kHz

No unnecessary conversion should occur.

---

# 20. UNSUPPORTED SAMPLE RATE

If the output hardware does not support the source rate, FLACHEAD needs an explicit fallback policy.

Example:

Source:

96 kHz

Device:

44.1 kHz and 48 kHz only

The negotiation layer MUST choose according to the configured policy.

Possible policies:

1. STRICT
2. CONVERT
3. REJECT

The system MUST NOT silently choose a different rate without knowing that conversion occurred.

---

# 21. NO FORCED RATE

The implementation must not automatically convert every track to 48 kHz merely because 48 kHz is common.

Similarly, the implementation must not automatically convert every source to 44.1 kHz.

The coding agent MUST NOT decide:

"Everything should be converted to 48 kHz"

unless this is explicitly part of the project's selected configuration.

The preferred output rate is the source rate when the hardware path supports it.

---

# 22. SAMPLE-RATE BUFFERING

Buffer duration depends on sample rate.

Higher sample rates consume buffered frames more quickly.

For:

44,100 Hz

the stream contains:

44,100 frames per second.

---

# 23. BIT DEPTH

Bit depth describes the number of bits representing each sample.

Common FLAC sources include:

16-bit

24-bit

Less common valid formats MAY also exist.

FLAC can also represent other valid depths, but FLACHEAD's initial implementation should prioritize formats relevant to the actual music library and target hardware.

The negotiation system MUST preserve the source bit depth whenever possible.

---

# 24. 16-BIT AUDIO

16-bit PCM is commonly represented as:

signed integer

with:

16 valid bits.

16-bit PCM is memory-efficient and widely supported.

A typical ALSA representation is:

S16_LE

The exact ALSA format MUST be verified during configuration.

---

# 25. 24-BIT AUDIO

24-bit PCM requires special handling.

24-bit PCM must be handled without silently truncating meaningful source data unless the output policy explicitly requires conversion.

If the output device supports 24-bit playback:

prefer preserving the source precision.

A 24-bit source may be represented as:

24 significant bits

inside:

3-byte packed storage

or:

32-bit storage with 24 valid bits.

These are not interchangeable byte layouts.

---

# 26. 24-BIT CONTAINER

Example:

24-bit valid audio

stored inside:

32-bit container.

Conceptually:

[ unused ][ valid 24 bits ]

Some output devices expose 24-bit samples inside 32-bit containers.

24 meaningful bits

+

8 padding bits

The negotiation layer MUST distinguish this from genuine 32-bit PCM.

The negotiation layer MUST distinguish logical bit depth from physical PCM representation.

The output layer MUST use the correct ALSA format.

---

# 27. BIT DEPTH CONVERSION POLICY

If the output device cannot accept 24-bit audio:

the conversion policy must be explicit.

Possible strategies include:

- conversion to 32-bit container
- conversion to 16-bit
- another backend-supported representation.

The selected strategy must be documented.

Any unavoidable reduction should be:

- explicit
- deterministic
- documented
- measurable.

24-bit to 16-bit conversion is potentially lossy.

Therefore it must not occur silently when preserving source quality is a project requirement.

---

# 28. NO BYTE-CASTING

The implementation MUST NOT assume that:

uint8_t* → int32_t*

is a valid PCM conversion.

Alignment, signedness, byte ordering, and representation matter.

Explicit conversion is required when representations differ.

---

# 29. SAMPLE FORMAT AND REPRESENTATION

Bit depth and storage format are related but not identical.

Examples of output representations may include:

signed 16-bit little-endian

signed 24-bit packed

signed 32-bit container carrying 24-bit samples

The negotiation system MUST distinguish:

logical bit depth

from:

physical PCM representation.

The implementation must explicitly distinguish:

- signed integer
- unsigned integer
- floating point.

PCM integer samples should normally use signed representations for the supported output formats.

The implementation MUST know whether the selected ALSA format is:

signed

or:

unsigned.

No implicit reinterpretation is allowed.

---

# 30. BYTES PER SAMPLE

For packed integer PCM:

16-bit:

2 bytes/sample

24-bit packed:

3 bytes/sample

32-bit:

4 bytes/sample

The implementation MUST calculate this according to the actual PCM representation.

It MUST NOT assume:

bits_per_sample / 8

is always sufficient for unusual packed/container formats.

---

# 31. BYTES PER FRAME

A PCM frame contains one sample for every channel.

Therefore:

bytes_per_frame =
    bytes_per_sample × channels

Example:

24-bit stereo packed:

3 × 2

=

6 bytes/frame

This value is important for ring-buffer accounting and ALSA writes.

For stereo 16-bit PCM:

2 channels × 2 bytes

=

4 bytes per frame.

At 44.1 kHz stereo 16-bit:

44,100 × 4

=

176,400 bytes per second.

Buffering and playback-position systems must use the negotiated frame definition.

---

# 32. ENDIANNESS

The internal PCM representation and ALSA output format MUST have explicitly defined endianness.

Linux Raspberry Pi systems are normally little-endian.

ALSA formats such as:

S16_LE

explicitly encode little-endian storage.

The implementation should use ALSA's format definitions rather than manually assuming byte order.

The code MUST use explicit format constants and conversions.

---

# 33. CHANNEL COUNT AND LAYOUT

The format MUST explicitly contain channel count.

The active stream MUST explicitly specify the number of channels.

Examples:

1 channel = mono

2 channels = stereo

More channels = multi-channel

The output backend MUST know exactly how many channels the PCM buffer contains.

Channel count alone may not fully describe multi-channel audio.

For stereo:

LEFT
RIGHT

channel 0 = Left

channel 1 = Right

The architecture should maintain this mapping consistently.

For multi-channel formats, the system SHOULD retain channel-layout information where supported.

If the BUNNY DSP/output path only supports stereo, a defined downmix policy is required for multi-channel sources.

The system MUST NOT silently discard channels.

---

# 34. MONO

Mono FLAC MUST be supported.

The output policy should define whether mono is sent as:

one-channel output

or:

duplicated into left/right

If mono is duplicated:

LEFT = MONO

RIGHT = MONO.

The preferred behavior is to preserve mono when the complete output chain supports it.

If mono-to-stereo duplication is required, it MUST be an explicit conversion.

The behavior MUST be deterministic.

---

# 35. STEREO

Stereo is the primary target format.

The output system MUST preserve:

Left → Left

Right → Right

No channel swap is acceptable.

The output pipeline MUST NOT swap:

Left

and:

Right.

---

# 36. MULTI-CHANNEL AUDIO

The initial FLACHEAD design is primarily intended for headphone/stereo playback.

Multi-channel FLAC SHOULD NOT be silently downmixed.

If multi-channel output is unsupported, the system should use an explicit policy:

- controlled downmix
- format conversion
- reject playback

Downmixing MUST be explicit.

If multichannel-to-stereo support is added:

the channel-mixing matrix must be explicitly defined.

Never implement multichannel conversion by simply discarding channels without an explicit policy.

If multichannel FLAC is not supported:

the decoder/output pipeline should reject it with:

UNSUPPORTED_CHANNEL_CONFIGURATION.

Do not silently discard channels.

---

# 37. CHANNEL ORDER TESTING

Hardware testing SHOULD include a channel-identification file.

The test MUST verify:

Left signal → left channel

Right signal → right channel

This catches:

- swapped channels
- incorrect interleaving
- incorrect conversion

Channel-order tests should be part of validation.

---

# 38. INTERLEAVING AND PCM LAYOUT

The internal PCM representation MUST explicitly specify whether samples are:

interleaved

or:

planar/non-interleaved.

For interleaved stereo:

L R L R L R ...

This is the preferred initial representation for the output path when ALSA supports it.

A planar buffer MUST NOT be passed as interleaved data.

Planar representation:

L0 L1 L2 L3 ...

R0 R1 R2 R3 ...

should only be used if required by another processing stage.

If a decoder or processing library produces planar PCM and ALSA requires interleaved PCM:

an explicit conversion/interleave stage is required.

This stage MUST be accounted for in CPU/memory profiling.

Interleaving must preserve:

- frame order
- channel order
- sample values.

A PCM frame must always contain the complete set of channel samples.

Samples must remain in chronological order.

Channel ordering must remain consistent through every conversion stage.

---

# 39. PASSTHROUGH POLICY (BIT-PERFECT MODE)

For bit-perfect mode:

- no software resampling
- no hidden format conversion
- no automatic channel mixing
- no software volume if prohibited by the selected mode
- source sample rate preserved
- source bit depth preserved where physically possible

Bit-perfect mode means no sample-domain processing.

Therefore it MUST bypass:

- ReplayGain
- software volume
- EQ
- resampling
- limiter
- compressor
- channel mixing
- DSP processing

unless the device itself performs processing externally.

If bit-perfect output is a project goal, volume should be handled through a suitable hardware/device mechanism when possible.

"Bit-perfect" should refer to the digital PCM path actually implemented.

The application should not claim bit-perfect playback merely because the source is FLAC.

If this cannot be achieved:

the system SHOULD report that bit-perfect output is unavailable.

---

# 40. STRICT MODE

Strict mode means:

Never convert the source format.

If the hardware cannot accept the source format:

Playback fails with:

OUTPUT_FORMAT_UNSUPPORTED

This is the preferred policy for a true bit-perfect mode.

In strict/bit-perfect mode:

No fallback involving conversion is allowed.

If exact output is unavailable:

stop playback with a clear error.

This protects the meaning of bit-perfect mode.

---

# 41. NORMAL COMPATIBILITY MODE

Normal playback MAY allow conversion.

Example:

FLAC:

96 kHz / 24-bit

Device:

48 kHz / 24-bit

Compatibility mode:

96 kHz
    ↓
48 kHz resampler
    ↓
24-bit
    ↓
Output

Normal mode may allow:

- ReplayGain
- software volume
- resampling
- channel conversion
- other explicitly enabled processing

Every active processing stage SHOULD be visible to diagnostics.

The user SHOULD NOT need to manually convert the music file.

---

# 42. FORMAT CONVERSION RULES

The format-conversion subsystem exists to bridge:

SOURCE AUDIO FORMAT

and:

OUTPUT DEVICE FORMAT.

It must not become an automatic audio-quality degradation layer.

The architecture recognizes three primary conversion categories:

1. sample-rate conversion
2. sample-format conversion
3. channel conversion.

These operations may occur independently or together.

---

# 43. SAMPLE-RATE CONVERSION

Sample-rate conversion changes the number of samples representing one second of audio.

FLACHEAD should avoid sample-rate conversion whenever the output device supports the source rate.

Resampling MUST occur only when necessary or explicitly requested.

Preferred:

44.1 kHz source
    ↓
44.1 kHz output

Avoid:

44.1 kHz source
    ↓
48 kHz
    ↓
44.1 kHz

Multiple conversions are prohibited unless explicitly required.

---

# 44. RESAMPLER REQUIREMENTS

If conversion is required, use a mature, tested sample-rate conversion implementation.

Do not implement a production-grade resampler from scratch without a strong reason.

The resampler should provide sufficient quality that conversion does not introduce obvious audible artifacts.

The resampler must process audio faster than real time with sufficient margin.

Resampler memory usage must remain bounded.

The resampler's internal latency must be known or measurable.

This matters for:

- playback position
- seeking
- lyrics synchronization
- pause
- track changes.

If a resampler provides multiple quality levels:

select the highest quality that maintains stable real-time playback.

The resampler should sit between the decoded PCM buffer and output backend.

Preferred:

FLAC Decoder
    ↓
PCM Buffer
    ↓
Resampler
    ↓
Output Backend

The FLAC decoder MUST NOT perform device-specific resampling.

---

# 45. RESAMPLER STATE MANAGEMENT

A resampler may maintain internal filter/history state.

That state belongs to the current playback generation.

After a seek:

the resampler state must be reset appropriately.

On track change:

the resampler must not carry state from the previous track into the next track.

If the sample rate changes:

the resampler must be reconfigured or recreated.

Old-track resampler history must never affect the next track.

---

# 46. RESAMPLER DELAY AND LATENCY

A high-quality resampler may introduce filter delay.

This delay must be understood when calculating audible position.

Conversion latency should be included in the audio timing model where necessary.

---

# 47. BIT-DEPTH CONVERSION

Bit-depth conversion MUST be explicit.

Example:

24-bit source
    ↓
24-bit output

requires no conversion.

If hardware only accepts 16-bit:

24-bit source
    ↓
Bit-depth converter
    ↓
16-bit output

This conversion MUST be recorded in the active audio session.

Reducing bit depth can discard information.

Therefore it should be avoided unless required.

---

# 48. DITHERING

If reducing bit depth:

24-bit
    ↓
16-bit

the conversion stage SHOULD consider appropriate dithering.

The exact policy MUST be documented.

Dithering MUST NOT be silently inserted into a declared bit-perfect path.

Dithering MUST only be performed when actually reducing effective sample precision.

If no bit-depth reduction occurs:

dither should not be applied.

---

# 49. CLIPPING

When converting to a smaller integer range:

samples outside the destination range MUST be handled safely.

The conversion MUST NOT overflow into wrapped integer values.

Bad:

maximum + 1 → negative minimum

Correct:

explicit saturation/clipping policy.

Conversions must avoid unintended clipping.

---

# 50. INTEGER RANGE

For signed N-bit PCM, the valid numerical range must be respected.

Example 16-bit signed PCM:

-32768 through +32767.

The implementation MUST not produce values outside the valid representation.

---

# 51. CHANNEL CONVERSION

Channel conversion changes the number or arrangement of channels.

Examples:

stereo → mono

mono → stereo

multichannel → stereo.

Channel conversion must be explicit.

The initial FLACHEAD target is:

stereo music playback.

Stereo source should normally remain stereo.

---

# 52. MONO TO STEREO

A mono source may be:

- sent as mono if supported
- duplicated into stereo if required by the output path.

The behavior must be explicit.

If mono is duplicated:

LEFT = MONO

RIGHT = MONO.

---

# 53. STEREO TO MONO

Stereo-to-mono downmixing must not simply discard one channel.

A defined mixing strategy is required.

---

# 54. DOWNMIXING

Downmixing is a signal-processing operation.

It must not occur implicitly.

If implemented, it belongs to the audio-processing layer.

For multichannel audio:

downmixing must use an explicit channel-mapping policy.

---

# 55. NO REDUNDANT CONVERSION

Do not perform:

24-bit → float

then:

float → 24-bit

when the output already accepts 24-bit integer PCM.

Every conversion can introduce:

- CPU cost
- rounding
- possible errors

The architecture should minimize conversion stages.

Avoid unnecessary sequences such as:

16-bit
    ↓
float
    ↓
32-bit
    ↓
float
    ↓
24-bit

---

# 56. NO DOUBLE RESAMPLING

Never:

44100 → 48000

then:

48000 → 44100

without an explicit technical reason.

If resampling is required:

perform it once.

---

# 57. CONVERSION PIPELINE OWNERSHIP AND ORDERING

The conversion pipeline SHOULD be represented as an explicit audio-processing component.

Possible structure:

AudioProcessor
    ├── Resampler
    ├── BitDepthConverter
    ├── ChannelConverter
    ├── ReplayGain
    └── VolumeProcessor

The output backend should consume the final negotiated PCM.

If multiple transformations are required, the order MUST be explicitly defined.

Recommended conceptual order:

FLAC DECODER
    ↓
CHANNEL CONVERSION IF REQUIRED
    ↓
SAMPLE-RATE CONVERSION IF REQUIRED
    ↓
SAMPLE-FORMAT CONVERSION IF REQUIRED
    ↓
OUTPUT PCM.

The exact order may differ if required by the chosen libraries/backend, but it must be explicit.

The coding agent MUST NOT invent processing order independently.

Only perform stages actually required.

---

# 58. CONVERSION PATH SEPARATION

The direct path should be the fastest path through the system.

The conversion path must remain separate from the direct path.

This allows performance testing to identify the cost of conversion.

The direct path minimizes:

- CPU
- latency
- memory
- conversion artifacts.

---

# 59. FORMAT PIPELINE OBJECT

Conceptually:

AudioFormatPipeline

    source_format
    destination_format
    sample_rate_converter
    sample_format_converter
    channel_converter
    latency
    processing_required

Exact implementation names may differ.

When no conversion is required:

processing_required = false.

When conversion is required:

processing_required = true.

---

# 60. CONVERSION PLAN

A conversion plan should explicitly specify:

- source rate
- destination rate
- source sample format
- destination sample format
- source channels
- destination channels
- required processing stages.

---

# 61. CONVERSION EXAMPLES

Source:

44100 Hz / 24-bit stereo

Output:

44100 Hz / 24-bit stereo

Result:

NO CONVERSION.

Source:

44100 Hz / 24-bit stereo

Output:

48000 Hz / 24-bit stereo

Result:

SAMPLE-RATE CONVERSION ONLY.

Source:

44100 Hz / 24-bit stereo

Output:

44100 Hz / 16-bit stereo

Result:

BIT-DEPTH CONVERSION ONLY.

Source:

44100 Hz / 16-bit mono

Output:

44100 Hz / 16-bit stereo

Result:

CHANNEL CONVERSION ONLY.

---

# 62. CONVERSION BUFFERING AND PLACEMENT

When conversion is required, buffering may need to exist on both sides.

Example:

Decoder
    ↓
Source PCM Buffer
    ↓
Resampler
    ↓
Output Buffer
    ↓
ALSA

Conversion must occur before PCM enters the output PCM buffer if that buffer represents final output PCM.

The output PCM buffer should contain:

PCM in the format expected by the audio backend.

This means the audio consumer does not need to understand source-format conversion.

The implementation SHOULD avoid excessive buffering.

Every buffer added increases:

- memory usage
- latency
- synchronization complexity

Only add buffers that solve a measured problem.

The responsibilities become:

DECODER = SOURCE PCM

CONVERTER = OUTPUT PCM

BUFFER = OUTPUT PCM STORAGE

BACKEND = OUTPUT PCM CONSUMPTION.

---

# 63. BUFFER DEPENDENCY

Buffer frame size depends on the output PCM format.

The buffer must use the negotiated frame size.

The buffer must account for actual storage width, not only nominal precision.

Regardless of storage representation:

buffer occupancy must be measured in frames.

---

# 64. BLOCK PROCESSING

Conversion should operate on PCM blocks rather than individual samples where practical.

Conversion block size should be compatible with:

- decoder output blocks
- PCM ring buffer
- output backend.

It does not need to be identical.

The converter must correctly handle blocks that contain fewer frames than the normal block size.

---

# 65. SAMPLE-RATE CONVERSION OUTPUT SIZE

Resampling changes the number of output frames.

The converter must accurately report how many frames were produced.

The conversion stage must track:

input_frames

output_frames.

---

# 66. POSITION MAPPING

When resampling:

source frame position and output frame position are different coordinate systems.

Playback position should remain expressed in source-track time.

The playback engine should not interpret converted output-frame counts as source-frame positions without applying the appropriate ratio.

---

# 67. CONVERSION STATE MANAGEMENT

Converter resources should be initialized before playback starts.

Do not initialize expensive resampler/filter state inside the real-time output callback.

Where possible:

allocate conversion buffers before playback begins.

The conversion path should avoid repeated heap allocations during continuous playback.

Converter lifetime should normally match the active playback generation.

Conversion state must be owned by the playback session or designated audio worker.

Do not use one mutable global resampler/converter for unrelated playback sessions.

Conversion state should be associated with the active generation.

---

# 68. SEEK AND CONVERSION STATE

Seek requests should be expressed in source time/frame coordinates.

Recommended seek pipeline:

source seek
    ↓
decoder reset
    ↓
converter reset
    ↓
PCM buffer clear
    ↓
output flush
    ↓
decode
    ↓
convert
    ↓
prebuffer
    ↓
output.

The converter must expose a mechanism to reset/discard pending internal state.

After seek:

- decoder state resets
- resampler state resets
- PCM buffer resets
- ALSA queued audio resets.

Every stage must agree on the new playback generation.

Old filter/history state must not contaminate the new position.

---

# 69. TRACK CHANGE AND CONVERSION STATE

On track change:

the resampler must not carry state from the previous track into the next track.

Track change normally requires:

reset converter.

The converter must not carry state from the previous track into the next track.

Any channel-mixing stage must also reset on:

- seek
- track change
- stop.

---

# 70. STOP AND CONVERSION STATE

Stop destroys or invalidates conversion state.

---

# 71. EOF AND CONVERSION DRAIN

At normal track end, a converter may contain delayed samples due to filter state.

The pipeline must drain the converter correctly before declaring output complete.

NORMAL EOF:

DRAIN converter.

SEEK/STOP:

RESET/FLUSH converter.

Recommended EOF pipeline:

DECODER EOF
    ↓
RESAMPLER FLUSH
    ↓
PCM BUFFER DRAIN
    ↓
OUTPUT DRAIN
    ↓
TRACK COMPLETE.

A resampler must not lose valid tail samples during normal EOF.

A resampler must not continue generating samples after its valid internal state has been fully drained.

---

# 72. SEEK WITH RESAMPLING

Seek requests should be expressed in source time/frame coordinates.

Recommended:

source seek
    ↓
decoder reset
    ↓
converter reset
    ↓
PCM buffer clear
    ↓
output flush
    ↓
decode
    ↓
convert
    ↓
prebuffer
    ↓
output.

---

# 73. TRACK CHANGES AND FORMAT TRANSITIONS

When switching tracks:

Track A Format
    ↓
Track A finishes
    ↓
Track B Format
    ↓
Compare formats

If identical:

continue using the existing output configuration when possible.

If different:

reconfigure the output path.

If the format changes:

Track A:

44.1 kHz / 16-bit

Track B:

96 kHz / 24-bit

then:

Drain/stop Track A
    ↓
Flush stale PCM
    ↓
Close/reconfigure output
    ↓
Configure Track B format
    ↓
Refill
    ↓
Resume

The exact transition behavior depends on gapless requirements.

---

# 74. AVOID UNNECESSARY RECONFIGURATION

If two consecutive tracks have:

same sample rate

same channel count

same sample format

the backend SHOULD avoid closing/reopening ALSA.

This reduces:

- transition latency
- CPU overhead
- possible audible gaps.

---

# 75. GAPLESS PLAYBACK

If formats are identical:

Track A PCM
    ↓
Track B PCM

may be continuous.

If formats differ:

a truly gapless transition may not be possible with every output device.

The system MUST prioritize correct output over pretending to provide gapless playback where hardware reconfiguration introduces unavoidable latency.

---

# 76. FORMAT RECONFIGURATION SAFETY

A format change requires:

- stopping or flushing output
- invalidating old PCM
- resetting conversion state
- configuring the new output format
- rebuilding buffering state.

A format change must create a new playback generation.

No new-format PCM should be sent through an old-format output configuration.

Never change the device format while old-format PCM remains queued.

The current track should not be considered successfully transitioned if a format change fails during track transition.

The Playback Engine SHOULD enter a controlled error or paused state.

It MUST NOT send incompatible PCM to the device.

---

# 77. FORMAT GENERATION AND IMMUTABILITY

The output format should be associated with the current playback generation.

Old-generation PCM must never be interpreted using the new track's format.

Within one buffer generation:

all PCM frames MUST have the same format.

No mixed-format buffers are allowed.

A format change must occur at a clear boundary:

old generation ends

new generation begins.

This greatly simplifies:

- buffering
- output
- seeking
- debugging.

Once a track begins decoding, its source format should be treated as immutable for that session.

If the decoder unexpectedly changes format midstream:

the session should enter an error/reconfiguration path.

The source format MUST NOT change during a normal FLAC stream.

If the decoder reports an unexpected format change:

Playback MUST stop or reconfigure safely.

The system MUST NOT reinterpret existing PCM under the new format.

---

# 78. PAUSE AND FORMAT CHANGE

Pause normally does not reset conversion state.

If a format change is requested while paused:

the engine may reconfigure before resuming.

Resume only after the output format and PCM pipeline agree.

---

# 79. BUNNY DSP PREFERENCES AND CAPABILITIES

The implementation MUST determine the actual formats exposed by the connected BUNNY DSP rather than assuming capabilities based on marketing specifications.

The coding agent MUST inspect the real ALSA device.

Example diagnostics:

aplay -D <device> --dump-hw-params

The exact command may vary.

Hardware test results SHOULD be recorded in project documentation.

Do not hard-code assumptions such as:

"BUNNY supports every 24-bit rate."

Instead:

query the ALSA device

and record actual supported parameters.

The TANCHJIM BUNNY DSP is an external USB audio/DSP device.

The implementation must treat it as an external output device rather than assuming arbitrary native format support.

The TANCHJIM BUNNY DSP may perform its own processing.

The software must distinguish:

software conversion

from:

hardware/DSP processing.

FLACHEAD cannot assume the DSP performs no processing unless verified.

If DSP configuration is exposed:

it belongs to the audio-device layer, not the FLAC decoder.

The pipeline should document:

SOURCE

↓

SOFTWARE PROCESSING

↓

DEVICE/DSP PROCESSING

↓

HEADPHONES.

If software processing is unnecessary:

do not add processing merely because the DSP already provides a relevant capability.

---

# 80. ALSA INTEGRATION

The expected Linux path is:

FLACHEAD
    ↓
ALSA
    ↓
USB Audio Driver
    ↓
BUNNY DSP

FLACHEAD SHOULD NOT bypass the Linux audio stack without a specific architectural reason.

The ALSA backend is responsible for applying the negotiated format.

Conceptually:

snd_pcm_hw_params_set_rate()

snd_pcm_hw_params_set_channels()

snd_pcm_hw_params_set_format()

The exact API usage belongs to the ALSA implementation.

All failures MUST be checked.

---

# 81. ALSA FAILURE

If ALSA rejects a negotiated format:

The backend MUST report:

OUTPUT_FORMAT_CONFIGURATION_FAILED

It MUST NOT simply continue using an unknown configuration.

---

# 82. FORMAT VALIDATION

After configuring ALSA, the backend SHOULD verify the actual active configuration.

Diagnostics SHOULD report:

Requested:

44.1 kHz / 24-bit / stereo

Actual:

44.1 kHz / 24-bit / stereo

If ALSA selected something different:

the backend MUST detect and report it.

Before playback begins verify:

sample rate > 0

channels > 0

bytes per sample > 0

frame size > 0.

The audio system should validate:

sample rate > 0

channels > 0

valid bit depth > 0

container width >= valid bit depth

supported channel layout

supported output range

before allocating or configuring buffers.

---

# 83. HARDWARE VS SOFTWARE CONVERSION

FLACHEAD SHOULD prefer hardware/device-native operation where it preserves the desired source format.

However, the application MUST know whether ALSA or another layer has inserted software conversion.

The system should avoid relying on hidden system-level conversion if bit-perfect playback is required.

---

# 84. ALSA PLUG DEVICES

ALSA `plug` devices may transparently perform:

- resampling
- format conversion
- channel conversion

Therefore, bit-perfect mode SHOULD avoid an implicit conversion layer.

The exact device configuration MUST be documented.

---

# 85. DEVICE CAPABILITY CACHING

Device capabilities MAY be cached during a session.

However, the system MUST refresh them when:

- device reconnects
- device changes
- output configuration changes
- ALSA reports changed capabilities

Do not assume capabilities remain unchanged across hotplug events.

---

# 86. DEVICE RECONNECTION

After BUNNY DSP reconnect:

1. Rediscover device.
2. Rediscover capabilities.
3. Recreate output session.
4. Re-negotiate current format.
5. Recreate conversion pipeline if required.
6. Resume according to configured reconnect policy.

Capabilities from the previous device session MUST NOT be blindly reused.

---

# 87. OUTPUT DEVICE REOPENING

The output backend SHOULD only reopen the device when necessary.

Reasons include:

- sample rate change
- channel configuration change
- sample format change
- device disconnect
- device reset
- unrecoverable ALSA state

---

# 88. THREADING AND CONCURRENCY

The UI must never perform synchronous heavy PCM conversion.

Conversion belongs to the audio pipeline.

Conversion state must be owned by the playback session or designated audio worker.

Do not use one mutable global resampler/converter for unrelated playback sessions.

Conversion state should be associated with the active generation.

Album-art operations must not affect PCM conversion timing.

UI thread must not perform synchronous heavy PCM conversion.

Background conversion belongs to the audio pipeline.

---

# 89. MEMORY SAFETY AND OVERFLOW PROTECTION

Format calculations MUST protect against overflow.

Potential calculations include:

sample_rate × channels × bytes_per_sample

and:

total_samples × bytes_per_frame

The implementation MUST validate values before multiplication.

Malformed files MUST NOT cause integer overflow leading to invalid memory allocation.

Frame-size calculations must use sufficiently wide integer types.

Memory calculations should use:

frames × frame_bytes

with overflow checks.

PCM memory must account for:

frame count

×

channels

×

bytes per sample.

The buffer must account for actual storage width, not only nominal precision.

The implementation should use safe integer arithmetic.

Large sample rates or malformed metadata must not cause:

integer overflow

when calculating:

frame counts

buffer sizes

byte counts.

Validate before allocation.

---

# 90. BUFFER ALLOCATION SAFETY

If the required buffer allocation exceeds the configured memory budget:

playback initialization must fail safely or reduce the configured buffer size within an approved range.

The engine must never allocate an arbitrarily large buffer because a format reports an unexpected frame size.

Untrusted or malformed metadata must not directly control dangerous memory allocation.

---

# 91. FORMAT SANITIZATION

The negotiation layer MUST reject impossible values.

Examples:

sample rate = 0

channels = 0

bits per sample = 0

absurd channel count

overflowing buffer size

unsupported sample format

These conditions MUST produce controlled errors.

The audio system should validate:

sample rate > 0

channels > 0

valid bit depth > 0

container width >= valid bit depth

supported channel layout

supported output range

before allocating or configuring buffers.

---

# 92. MALFORMED FORMAT AND CORRUPTION

A malformed source format must not crash the decoder thread.

If the decoder cannot produce valid PCM:

the audio output layer should never receive undefined data.

The decoder must report the failure.

A corrupted FLAC stream may fail during decoding.

Decoder errors should transition playback into a controlled error state.

Partially decoded invalid data must not be treated as valid audio merely to keep playback moving.

---

# 93. FORMAT NEGOTIATION FAILURE

If no acceptable format exists:

Playback MUST fail cleanly.

The Playback Engine SHOULD expose:

"Unsupported audio format"

rather than:

"Decoder failed"

when the FLAC itself is valid.

A format negotiation failure must never result in invalid PCM being sent to the device.

A format negotiation failure should expose a useful error category.

Example:

UNSUPPORTED_AUDIO_FORMAT.

---

# 94. FALLBACK POLICY

Fallbacks MAY be attempted in compatibility mode.

Example:

Try:

96 kHz / 24-bit

If unsupported:

Try:

48 kHz / 24-bit

If supported and resampling enabled:

use it.

If not:

report failure.

Fallback selection MUST be deterministic.

In strict/bit-perfect mode:

No fallback involving conversion is allowed.

If exact output is unavailable:

stop playback with a clear error.

---

# 95. NO SILENT QUALITY LOSS

The following are prohibited unless explicitly configured:

- automatic downsampling
- automatic bit-depth reduction
- automatic mono conversion
- automatic normalization
- automatic compression
- hidden DSP processing
- hidden conversion layers

The system MUST be able to answer:

"What exactly happens to this track between FLAC decoding and the BUNNY DSP?"

The answer MUST be represented by explicit stages.

The system MUST NOT dynamically change formats without reporting the selected output configuration.

---

# 96. NO ARTIFICIAL ENHANCEMENT

The core playback pipeline must not automatically apply:

- EQ
- compression
- normalization
- loudness enhancement
- artificial spatialization.

unless explicitly enabled by the user/system configuration.

Digital gain should not be applied unless explicitly requested.

Unnecessary gain processing can:

- alter samples
- introduce clipping
- destroy bit-perfect output.

Automatic loudness normalization MUST NOT be enabled by default.

It changes PCM data.

If a future normalization system is added, it must be an explicit audio-processing stage.

ReplayGain MAY be supported in the future.

It MUST remain disabled unless configured.

Its gain application must be clearly separated from the direct PCM path.

If gain processing exists:

the audio-processing layer should define clipping behavior.

The output layer should not silently modify samples to prevent clipping.

An equalizer, if implemented later, must be an explicit DSP stage.

---

# 97. HIGH-RESOLUTION POLICY

High-resolution FLAC MUST be treated as a real source format.

The system MUST NOT automatically assume that:

192 kHz = unnecessary

or:

24-bit = unnecessary

Those are configuration/user-quality decisions.

The implementation's responsibility is to correctly support them where hardware permits.

High-resolution FLAC MUST be treated as a real source format.

The project should support high-resolution FLAC when:

- decoder supports it
- memory permits it
- CPU permits it
- BUNNY DSP supports it
- ALSA supports it.

High-resolution support should not be claimed merely because FLAC can encode it.

---

# 98. UPSAMPLING

Upsampling a lower-rate source does not create additional source information.

Therefore FLACHEAD should not upsample merely for the appearance of a higher sample rate.

---

# 99. AUDIO FORMAT LOGGING AND DIAGNOSTICS

Diagnostics MUST distinguish:

SOURCE:

96 kHz / 24-bit

OUTPUT:

48 kHz / 24-bit

PROCESSING:

Resampler

This prevents confusion during debugging.

At track start, debug logs SHOULD be able to report:

Source:
    FLAC
    96 kHz
    24-bit
    Stereo

Output:
    96 kHz
    24-bit
    Stereo

Conversion:
    None

If conversion occurs:

Source:
    96 kHz / 24-bit

Output:
    48 kHz / 24-bit

Conversion:
    Resampling

This makes hidden transformations visible.

When conversion is active, diagnostics SHOULD show:

Source:

96 kHz / 24-bit / stereo

Output:

48 kHz / 24-bit / stereo

Conversion:

Resampling

Bit-perfect:

No

This helps identify unexpected processing.

The exact conversion should be recorded in the active audio session.

Format information should be logged at pipeline initialization rather than for every PCM block.

Do not log every PCM block.

Detailed conversion timing may be enabled for profiling.

Do not log individual converted samples during normal operation.

Useful diagnostics include:

- source format
- selected output format
- conversions applied
- converter initialization result
- device/backend
- resampler type
- input frames
- output frames
- conversion latency
- conversion CPU time
- generation.

---

# 100. UI INTEGRATION

The library UI SHOULD be able to display the source format independently of the negotiated output format.

Example:

Track:

FLAC
96 kHz
24-bit

Current output:

48 kHz
24-bit

These are different concepts.

The UI should not incorrectly display:

"48 kHz FLAC"

when the source file is actually 96 kHz.

The UI may display information such as:

FLAC

24-bit

96 kHz

but it must not decide:

"convert this to 48 kHz."

Audio policy belongs to the audio system.

The UI should not falsely claim that the device is receiving the source format if conversion is occurring.

The UI should not claim:

"lossless output"

unless the actual output path preserves the relevant source information.

The UI progress bar should remain based on source-track time.

The UI should not visibly lag behind the actual audio more than necessary.

Lyrics synchronization should use the playback clock rather than decoder progress.

Lyrics timestamps refer to source-track time.

The conversion pipeline must not alter lyric timestamps.

The UI may eventually show:

96 kHz · 24 bit · FLAC

This should come from the actual decoded/source metadata.

If output differs, diagnostic information may optionally expose:

96 kHz source → 48 kHz output.

---

# 101. PERFORMANCE AND PI ZERO W CONSTRAINTS

Resampling may be expensive on ARMv6.

Therefore:

- avoid resampling when unnecessary
- prefer direct device rates
- benchmark resampling on Pi Zero W
- measure CPU usage at 44.1 → 48 kHz
- measure high-rate conversions separately

The coding agent MUST NOT assume desktop resampling performance applies to the Pi Zero W.

Bit-depth conversion is generally cheaper than resampling but still requires processing.

The system SHOULD avoid repeated conversion.

High sample rates increase:

- decoded PCM throughput
- memory bandwidth
- buffer size requirements
- output transfer rate.

The Pi Zero W has limited system resources.

The implementation should avoid unnecessary copies of large PCM blocks.

Preferred:

decode once
    ↓
write directly into appropriate buffer
    ↓
process only when required
    ↓
write to ALSA

True zero-copy is not mandatory.

Correctness and reliability are more important.

However, unnecessary copies should be identified during profiling.

At high sample rates, repeated full-buffer copies can become expensive.

Conversion should ideally write directly into available ring-buffer regions when practical.

The audio backend should consume directly from contiguous ring-buffer regions where possible.

If a block crosses the end of the ring:

the implementation may use two segments.

Do not allocate a large temporary PCM block for every callback.

If a converter requires temporary storage:

allocate it during session initialization.

Temporary conversion storage should be sized according to the active format.

Do not introduce:

- custom SIMD
- custom ARM assembly
- custom resampling algorithms
- complicated lock-free pipelines

before profiling.

The Pi Zero W constraints are real, but correctness must come first.

---

# 102. OPTIMIZATION ORDER

If format conversion becomes expensive:

1. Measure.
2. Identify bottleneck.
3. Confirm conversion is actually necessary.
4. Avoid unnecessary conversion.
5. Optimize memory copies.
6. Optimize processing implementation.
7. Re-measure on Pi Zero W.

---

# 103. RESOURCE LIMITS

Format negotiation MUST respect:

- CPU
- memory
- output hardware
- decoder capability
- buffer capacity

A format should not be accepted merely because it is theoretically valid if the complete playback pipeline cannot sustain it.

---

# 104. CONVERSION CPU MEASUREMENT

Measure conversion CPU usage separately from decoding CPU usage.

Test:

decoder only

versus:

decoder + converter.

The complete pipeline must remain faster than real time.

Run the converter while:

- UI animation is active
- touchscreen input is active
- metadata operations occur.

Verify:

real-time playback remains stable.

Run conversion while SD-card reads experience realistic latency.

Verify:

PCM buffering protects playback.

---

# 105. CONFIGURATION

Format behavior SHOULD be controlled through configuration.

Possible settings:

output_device

output_mode

bit_perfect

allow_resampling

preferred_sample_rate

preferred_bit_depth

channel_mode

software_volume

The exact configuration schema is defined elsewhere.

This document defines their audio semantics.

---

# 106. USER-FACING ERRORS

The user-facing error should remain concise.

Example:

"Unsupported audio format."

Technical logs should contain the actual reason.

Diagnostics should identify:

- source format
- requested output
- available device capabilities
- failed conversion stage.

---

# 107. CONVERSION ERROR CATEGORIES

Useful conversion errors include:

UNSUPPORTED_SAMPLE_RATE

UNSUPPORTED_SAMPLE_FORMAT

UNSUPPORTED_CHANNEL_LAYOUT

UNSUPPORTED_CHANNEL_CONFIGURATION

RESAMPLER_INIT_FAILED

CONVERTER_INIT_FAILED

CONVERSION_RUNTIME_ERROR

CONVERSION_PERFORMANCE_FAILURE.

Conversion errors belong to the audio processing/output subsystem.

They should be translated into playback-level errors.

A conversion failure should normally terminate the active track cleanly rather than continuing with corrupted output.

---

# 108. ACCEPTANCE CRITERIA

The format negotiation and conversion system is production-ready when:

- every active stream has explicit format metadata
- source format is correctly detected
- output format is explicitly identified
- internal PCM format is defined
- source and output formats are never implicitly assumed identical
- supported formats remain unchanged
- unsupported formats have explicit fallback behavior
- native formats bypass conversion
- exact-format playback uses a direct path
- unnecessary conversions are avoided
- source sample rate is preserved when supported
- source bit depth is preserved when supported
- no hidden resampling occurs
- no hidden downmixing occurs
- no hidden bit-depth reduction occurs
- no hidden conversion layers exist
- PCM representation is unambiguous
- 24-bit formats are handled correctly
- channel order is preserved
- channel configuration is handled explicitly
- resampling is explicit
- bit-depth reduction is explicit
- channel conversion is explicit
- conversion order is documented
- conversion state is session-local
- converter state belongs to the playback generation
- resampler state resets correctly
- seek resets conversion state
- track changes reset conversion state
- EOF flushes converter state correctly
- stop resets converter state
- output PCM is valid
- output buffer contains final negotiated PCM
- buffer accounting uses final frame size
- frame alignment is preserved
- playback clock uses correct sample-rate information
- backend capabilities are validated
- unsupported configurations fail cleanly
- malformed metadata cannot cause unsafe allocations
- format changes are handled safely
- format diagnostics are available
- high-resolution playback is benchmarked on Pi Zero W
- memory usage remains bounded
- steady-state allocation is minimized
- unnecessary copies are avoided
- conversion sustains real-time playback
- CPU usage remains sustainable on Pi Zero W
- device reconnection renegotiates format
- TANCHJIM BUNNY DSP capabilities are verified
- TANCHJIM BUNNY DSP output is tested on actual hardware
- long-duration converted playback is stable
- no unexplained processing is introduced
- no artificial audio enhancement is applied by default
- no memory growth over long playback
- no format-state corruption
- no resampler-state leakage
- no cumulative timing drift
- no increasing underruns.

---

# 109. ACCEPTANCE TESTS

The test suite should cover at minimum:

**Format Match Tests:**

- exact format match (source = device): direct path, zero conversion stages
- sample-rate mismatch with conversion
- bit-depth mismatch with conversion
- channel mismatch with conversion
- unsupported format rejection

**Mode Tests:**

- strict/bit-perfect mode rejection on mismatch
- compatibility mode conversion

**Sample Rate Tests:**

- matching sample rates: direct output, no resampler
- mismatched sample rates: correct resampling, stable playback
- verify resampler does not introduce audible artifacts

**Bit Depth Tests:**

- matching bit depth: direct output
- bit-depth reduction: verify expected numerical conversion, no overflow, expected rounding/clipping
- 24-bit playback on compatible output: no unintended truncation
- 24-bit FLAC on compatible path: no unnecessary conversion

**Channel Tests:**

- stereo preservation: Left → Left, Right → Right
- mono playback: correct output channel behavior
- mono → stereo duplication: correct mapping
- stereo → mono downmix: defined mathematical policy
- channel-order verification with known test signals: no channel swap
- multichannel rejection if unsupported

**Conversion State Tests:**

- seek during active resampling: verify resampler state is reset, no stale converter state
- track change between different formats: verify converter reconfigured correctly, old state not reused
- EOF with resampler: verify resampler tail output is drained correctly
- rapid track switching: verify old format PCM cannot reach new output
- stop: verify conversion state is invalidated

**Format Transition Tests:**

- track format change between 44.1 kHz and 96 kHz: verify output reconfiguration
- format transition: old output flushed, new format configured, PCM interpreted correctly
- avoid unnecessary ALSA close/reopen when formats match

**Long-Duration and Stress Tests:**

- run high-resolution FLAC continuously: monitor CPU, RAM, buffer occupancy, XRUNs
- run converted playback for several hours: verify no memory growth, no cumulative timing error, no increasing latency, no conversion failures
- play mixed-format FLAC files for several hours: verify no memory leak, no format-state corruption, no resampler-state leakage, no timing drift, no underrun escalation

**CPU and Performance Tests:**

- measure decoder CPU vs decoder + converter CPU
- run conversion under realistic UI load: verify real-time playback remains stable
- conversion must sustain real-time playback with sufficient margin

**Device Tests:**

- test converted playback through actual TANCHJIM BUNNY DSP
- rapidly switch 44.1 kHz → 48 kHz → 44.1 kHz and other combinations
- force converter initialization failure: verify controlled playback error
- change output device/configuration: verify new capabilities are negotiated

**Memory Tests:**

- verify conversion buffers remain within configured memory budget

**Dither Tests:**

- if dithering is implemented: verify it only occurs when configured/required

---

# 110. AI CODING AGENT RULES

The coding agent MUST:

1. Represent source and output formats explicitly.
2. Define an explicit internal PCM representation.
3. Inspect real BUNNY DSP capabilities.
4. Prefer exact source-format output.
5. Prefer direct PCM output whenever source and device formats match.
6. Negotiate output format before normal playback begins.
7. Avoid unnecessary resampling.
8. Avoid unnecessary bit-depth conversion.
9. Avoid unnecessary channel conversion.
10. Avoid unnecessary conversion of any kind.
11. Never resample without a documented reason.
12. Never force all audio to one sample rate without an architectural requirement.
13. Never reduce bit depth unnecessarily.
14. Never silently discard channels.
15. Never silently resample.
16. Never silently downmix.
17. Never silently reduce bit depth.
18. Never silently truncate audio.
19. Never silently downsample audio.
20. Never allow format conversion behavior to be ambiguous.
21. Keep conversion stages explicit.
22. Keep conversion inside an explicit processing stage.
23. Keep conversion separate from FLAC decoding.
24. Keep conversion separate from device management.
25. Keep conversion separate from UI code.
26. Keep conversion separate from queue logic.
27. Keep conversion separate from metadata parsing.
28. Keep channel mapping explicit.
29. Keep channel ordering correct.
30. Keep PCM frame alignment correct.
31. Keep the PCM buffer format-consistent.
32. Keep conversion state associated with the playback generation.
33. Keep output capability handling explicit.
34. Distinguish source metadata from output configuration.
35. Distinguish source format and output format as separate concepts.
36. Distinguish valid bits from storage/container width.
37. Distinguish logical bit depth from physical PCM representation.
38. Never reinterpret PCM bytes without an explicit format.
39. Preserve source format whenever supported.
40. Preserve source precision whenever possible.
41. Preserve existing audio abstractions.
42. Preserve the highest practical audio fidelity throughout the pipeline.
43. Preserve source format whenever the complete output path supports it.
44. Preserve channel count unless explicitly converting.
45. Preserve bit depth unless explicitly converting.
46. Preserve sample rate unless explicitly converting.
47. Support strict/bit-perfect behavior.
48. Support compatibility behavior where configured.
49. Handle unsupported formats safely.
50. Validate ALSA's actual negotiated configuration.
51. Validate ALSA capabilities before playback.
52. Query or validate backend/device capabilities.
53. Avoid hidden conversion layers.
54. Avoid unnecessary output reconfiguration.
55. Prevent integer overflow during conversion.
56. Prevent integer overflow during size calculations.
57. Prevent unintended clipping.
58. Protect size calculations against integer overflow.
59. Use wide integer types for frame-size and memory calculations.
60. Check allocation-size overflow.
61. Never allow malformed metadata to trigger unbounded allocation.
62. Use appropriate dithering when reducing precision.
63. Avoid unnecessary float conversions.
64. Avoid repeated conversion stages.
65. Prefer one resampling stage.
66. Prefer frame-aware interfaces over raw byte interfaces.
67. Reset resampler state on seek.
68. Reset resampler state on track change.
69. Reset conversion state on stop.
70. Reset conversion state on format change.
71. Drain converter state at EOF.
72. Do not carry converter state across incompatible tracks.
73. Prevent old-format PCM from reaching a new track.
74. Associate conversion state with playback generation.
75. Account for converter latency.
76. Do not modify lyrics timestamps during conversion.
77. Do not modify source metadata during conversion.
78. Keep conversion errors structured.
79. Keep conversion memory bounded.
80. Avoid unnecessary PCM copies.
81. Avoid large runtime allocations.
82. Avoid unnecessary dynamic allocation during steady-state playback.
83. Never initialize converters for every PCM block.
84. Benchmark conversion independently.
85. Benchmark conversion on Raspberry Pi Zero W.
86. Benchmark high-resolution FLAC playback.
87. Benchmark mixed-format queues.
88. Test all officially supported source formats.
89. Test unsupported formats.
90. Test device reconnection.
91. Test rapid format switching.
92. Test seek while conversion is active.
93. Test long-duration converted playback.
94. Test CPU pressure.
95. Test UI pressure.
96. Test actual TANCHJIM BUNNY DSP output.
97. Test mono and stereo handling.
98. Test high sample rates.
99. Test high bit depths.
100. Test required resampling.
101. Test required format conversion.
102. Test EOF during conversion.
103. Test track changes between different formats.
104. Test output-device changes.
105. Do not silently reduce conversion quality.
106. Keep format negotiation deterministic.
107. Never claim bit-perfect playback without verifying the actual pipeline.
108. Never claim bit-perfect output without validating the entire output path.
109. Never apply EQ, normalization, compression, or enhancement by default.
110. Keep software processing separate from DSP hardware processing.
111. Do not introduce hidden conversion stages.
112. Do not invent unsupported output formats.
113. Do not invent unsupported hardware capabilities.
114. Do not hide conversion decisions from diagnostics.
115. Fail cleanly when no valid format path exists.
116. Document the selected canonical internal PCM representation.
117. Document the selected output-format policy.
118. Log negotiated formats during pipeline initialization.
119. Keep diagnostic format information separate from user-facing messaging.
120. Keep UI decisions separate from audio-format policy.
121. Never force everything to 48 kHz.
122. Never force everything to 16-bit.
123. Prefer correctness and audio fidelity over premature optimization.
124. Never send incompatible PCM to the hardware.
125. Never silently degrade audio quality.
126. Never allow format changes to happen implicitly inside unrelated components.
127. Never automatically reduce sample rate, bit depth, or channel count to reduce CPU usage.
128. Never claim "Hi-Res" or "Bit-perfect" unless the actual audio path satisfies the documented conditions.
129. Never assume that all FLAC files have the same audio format.
130. Never assume stereo for every file.
131. Never assume a fixed FLAC sample rate.
132. Never assume a fixed bit depth.
133. Never assume channel order without knowing the source and destination layout.
134. Never assume that raw PCM bytes are automatically in the correct byte order.
135. Never assume the TANCHJIM BUNNY DSP supports a format without validation.
136. Never assume desktop resampling performance applies to the Pi Zero W.
137. Keep all conversion memory bounded.
138. Keep the negotiated format immutable for the active playback generation.
139. Ensure final PCM matches the output buffer format.
140. Keep buffer accounting frame-correct.
141. Keep playback-clock calculations format-aware.

---

# 111. FINAL DECISION TREE

The intended negotiation algorithm is:

START
  │
  ▼
Read Source Format
  │
  ▼
Query Device Capabilities
  │
  ▼
Exact Match?
  │
 ┌┴───────────────┐
YES               NO
  │                 │
  ▼                 ▼
DIRECT        Conversion Allowed?
                   │
              ┌────┴────┐
             YES        NO
              │          │
              ▼          ▼
        Find Best      REJECT
        Compatible
          Format
              │
              ▼
      Build Conversion
          Pipeline
              │
              ▼
        Configure ALSA
              │
              ▼
          Validate
              │
              ▼
          PLAYBACK

---

# 112. FINAL ARCHITECTURE

The complete format-aware pipeline is:

                         FLAC FILE
                             │
                             ▼
                       FLAC DECODER
                             │
                             ▼
                     SOURCE PCM FORMAT
                             │
                             ▼
                      PCM RING BUFFER
                             │
                             ▼
                  FORMAT NEGOTIATION
                             │
              ┌──────────────┴──────────────┐
              │                             │
         EXACT MATCH                   CONVERSION
              │                             │
              │                    ┌────────┴────────┐
              │                    │                 │
              │                RESAMPLER       FORMAT CONVERTER
              │                (sample rate)   (bit depth, channels)
              │                    │                 │
              │                    └────────┬────────┘
              │                             │
              └──────────────┬──────────────┘
                             ▼
                     OUTPUT PCM FORMAT
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

---

# 113. INVARIANTS

The central invariants are:

1. If the BUNNY DSP can accept the source format directly, FLACHEAD should send it directly. Conversion is a compatibility mechanism, not the default behavior.

2. SOURCE FORMAT MUST BE EXPLICITLY DISTINGUISHED FROM OUTPUT FORMAT.

3. THE DIRECT PATH IS PREFERRED WHEN THE SOURCE FORMAT IS ALREADY SUPPORTED.

4. NO UNNECESSARY CONVERSION SHOULD OCCUR.

5. SOURCE RESOLUTION SHOULD BE PRESERVED WHEN THE COMPLETE OUTPUT PATH SUPPORTS IT.

6. THE PLAYBACK PIPELINE MUST NEVER SILENTLY REDUCE AUDIO QUALITY.

7. ANY FORMAT MODIFICATION MUST BE EXPLICIT, TESTABLE, AND TRACEABLE.

8. SOURCE-TIME POSITION REMAINS THE AUTHORITATIVE PLAYBACK POSITION EVEN WHEN SAMPLE-RATE CONVERSION CHANGES THE NUMBER OF OUTPUT FRAMES.

9. CONVERTER STATE BELONGS TO THE CURRENT PLAYBACK GENERATION.

10. SEEK, STOP, TRACK CHANGE, AND FORMAT CHANGE MUST INVALIDATE APPROPRIATE CONVERSION STATE.

11. NORMAL END-OF-TRACK MUST DRAIN CONVERSION STATE BEFORE OUTPUT IS DECLARED COMPLETE.

12. THE OUTPUT PCM BUFFER CONTAINS PCM IN THE FINAL NEGOTIATED OUTPUT FORMAT.

13. BUFFER FRAME SIZE AND PLAYBACK-CLOCK CALCULATIONS MUST USE THE ACTIVE FORMAT.

14. THE SYSTEM MUST NOT INVENT HARDWARE CAPABILITIES.

15. NO AUDIO PROCESSING SHOULD BE APPLIED BY DEFAULT UNLESS EXPLICITLY REQUIRED OR CONFIGURED.

16. THE CONVERSION PIPELINE MUST MAINTAIN REAL-TIME PERFORMANCE ON THE RASPBERRY PI ZERO W.

17. AUDIO QUALITY MUST NOT BE REDUCED MERELY FOR CONVENIENCE OR FOR CPU USAGE REDUCTION.

18. THE FINAL FORMAT POLICY MUST BE VALIDATED AGAINST THE ACTUAL TANCHJIM BUNNY DSP AND RASPBERRY PI ZERO W.

19. THE AUDIO FORMAT MUST ALWAYS BE KNOWABLE AT EVERY STAGE:

SOURCE
    ↓
DECODER
    ↓
BUFFER
    ↓
PROCESSING
    ↓
OUTPUT
    ↓
ALSA
    ↓
DSP

20. This architecture preserves the native FLAC format whenever possible while providing a controlled, measurable, and hardware-aware compatibility path for formats that cannot be sent directly to the output device.

The system must always know:

what the source format is,

what the output format is,

and what transformations occurred between them.

---

Final responsibility separation:

FLAC DECODER = COMPRESSED FLAC → SOURCE PCM

FORMAT NEGOTIATOR = SOURCE FORMAT → VALID OUTPUT PLAN

CHANNEL CONVERTER = CHANNEL REPRESENTATION CONVERSION

RESAMPLER = SAMPLE-RATE CONVERSION

SAMPLE FORMAT CONVERTER = PCM PRECISION/REPRESENTATION CONVERSION

PCM BUFFER = FINAL OUTPUT PCM QUEUING / BOUNDED AUDIO STORAGE

AUDIO BACKEND = OUTPUT PCM → DEVICE STREAM

TANCHJIM BUNNY DSP = EXTERNAL AUDIO PROCESSING/OUTPUT DEVICE

AUDIO CLOCK = AUDIBLE PLAYBACK TIMING / TRACK TIMELINE BASED ON ACTUAL OUTPUT FORMAT

PLAYBACK ENGINE = OWNS GENERATION, STATE, SEEK, FORMAT LIFECYCLE, AND PIPELINE CONTROL

This guarantees that FLACHEAD has a deterministic format path from lossless FLAC decoding through final device output while minimizing unnecessary processing and preserving the highest practical source quality on the Raspberry Pi Zero W.
