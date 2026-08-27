# FLACHEAD AUDIO ARTWORK ARCHITECTURE

Document

27_AUDIO_ARTWORK_ARCHITECTURE.md

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

This document defines the architecture for album artwork and other track-associated artwork in FLACHEAD.

The artwork system is responsible for:

- discovering artwork
- reading embedded artwork
- discovering external artwork
- associating artwork with tracks/albums
- extracting artwork
- decoding artwork
- resizing artwork
- caching artwork
- invalidating stale artwork
- providing artwork to the UI
- handling missing/corrupt artwork
- keeping memory usage appropriate for the Pi Zero W.

The primary objective is:

PROVIDE FAST, OFFLINE, MEMORY-EFFICIENT ARTWORK ACCESS WITHOUT BLOCKING AUDIO PLAYBACK OR THE UI THREAD.

---

# 2. CORE PRINCIPLE

Artwork is:

PRESENTATION ASSET.

It is not:

AUDIO DATA.

Artwork processing must remain separate from:

- audio decoding
- audio output
- playback timing
- queue management
- lyric synchronization.

---

# 3. ARCHITECTURAL RELATIONSHIP

The intended architecture is:

                         TRACK
                           │
                           ▼
                     METADATA SYSTEM
                           │
                           ▼
                     ARTWORK SOURCE
                           │
                ┌──────────┴──────────┐
                ▼                     ▼
          EMBEDDED ARTWORK       EXTERNAL ARTWORK
                │                     │
                └──────────┬──────────┘
                           ▼
                    ARTWORK LOADER
                           │
                           ▼
                    IMAGE DECODER
                           │
                           ▼
                    IMAGE PROCESSOR
                           │
                           ▼
                     ARTWORK CACHE
                           │
                           ▼
                           UI

---

# 4. ARTWORK OWNERSHIP

The artwork subsystem owns:

- artwork discovery
- source selection
- extraction
- decoding
- resizing
- caching
- cache invalidation
- artwork availability state.

---

# 5. ARTWORK DOES NOT OWN

Artwork does not own:

- track metadata authority
- queue state
- playback state
- audio decoding
- audio output
- lyric synchronization.

---

# 6. SUPPORTED SOURCES

Initial artwork sources should support:

1. embedded FLAC artwork
2. external artwork files.

Possible external formats:

- JPEG
- PNG
- WebP if the selected image library supports it efficiently.

---

# 7. EMBEDDED ARTWORK

FLAC files may contain embedded pictures.

The artwork system should be able to extract the appropriate embedded picture.

---

# 8. EXTERNAL ARTWORK

External artwork may exist beside the audio files.

Common names may include:

- cover.jpg
- cover.png
- folder.jpg
- folder.png
- front.jpg
- front.png.

The exact discovery priority must remain deterministic.

---

# 9. SOURCE PRIORITY

Recommended priority:

1. explicitly selected artwork
2. embedded front-cover artwork
3. external album artwork
4. external directory artwork
5. configured fallback.

---

# 10. FRONT COVER

If multiple embedded pictures exist:

front-cover artwork should normally be preferred.

---

# 11. OTHER EMBEDDED PICTURES

Other picture types may include:

- back cover
- artist
- leaflet
- media
- icon.

These may be supported later.

Initial implementation only requires the primary display artwork.

---

# 12. TRACK VS ALBUM ARTWORK

Artwork may be associated with:

- individual track
- album
- artist.

The initial architecture should prioritize:

track artwork

and:

album artwork.

---

# 13. ALBUM ARTWORK

If all tracks in an album use the same artwork:

the cache should allow reuse of the decoded asset.

---

# 14. ARTWORK IDENTITY

Artwork should have a stable internal identity.

Conceptually:

ArtworkKey

    source_type
    source_identity
    source_signature
    artwork_variant

---

# 15. SOURCE IDENTITY

For embedded artwork:

source identity may include:

track_id

+

embedded picture index/type.

For external artwork:

source identity may include:

canonical file path.

---

# 16. ARTWORK SIGNATURE

Artwork cache validity should depend on the source version.

Possible signature:

- file size
- modification time
- source metadata signature.

---

# 17. EMBEDDED ARTWORK SIGNATURE

Embedded artwork changes when the source FLAC metadata changes.

The artwork cache should therefore be associated with the relevant file/version signature.

---

# 18. EXTERNAL ARTWORK SIGNATURE

External artwork should use:

canonical path

+

file size

+

modification time.

---

# 19. CONTENT HASH

A full image hash is optional.

Do not require full hashing on every library scan.

---

# 20. ARTWORK DISCOVERY

Artwork discovery should be performed asynchronously.

---

# 21. DISCOVERY WORKER

A background worker may:

- inspect metadata
- inspect nearby artwork files
- determine the best source
- produce an artwork reference.

---

# 22. UI THREAD

The UI must not:

- open large image files
- decode JPEG/PNG
- extract embedded FLAC artwork.

---

# 23. AUDIO THREAD

The audio thread must never perform artwork operations.

---

# 24. EXTRACTION

Embedded artwork should be extracted only when needed.

Do not extract every album's artwork into RAM at startup.

---

# 25. LAZY LOADING

Artwork should generally be loaded lazily.

Example:

When the full player opens:

request artwork.

When an album list is displayed:

request appropriately sized artwork.

---

# 26. PRELOADING

The system may preload artwork for:

- current track
- next track
- currently visible UI items.

---

# 27. PRELOAD LIMIT

Preloading must remain bounded.

Do not preload artwork for the entire music library.

---

# 28. IMAGE DECODING

The decoder converts the source image into a format usable by the renderer.

---

# 29. RENDERING FORMAT

The final internal format should be selected based on the SDL/rendering pipeline.

A compact pixel format should be preferred when appropriate.

---

# 30. ALPHA

If the image contains transparency:

preserve it where the UI requires it.

If transparency is unnecessary:

a non-alpha representation may reduce memory usage.

---

# 31. IMAGE DIMENSIONS

The source artwork may be significantly larger than the 2.8" display.

Do not retain unnecessarily large decoded images.

---

# 32. DOWNSCALING

Artwork should be resized to the largest size actually required by the UI.

---

# 33. ORIGINAL IMAGE RETENTION

Do not retain the full-resolution source image in RAM after processing unless specifically required.

---

# 34. THUMBNAILS

The artwork system should support multiple size classes.

Example:

THUMBNAIL

MEDIUM

PLAYER

---

# 35. SIZE CLASSES

A practical initial model:

- small list artwork
- medium card artwork
- full-player artwork.

Exact pixel dimensions should be defined by the rendering implementation.

---

# 36. CACHE VARIANTS

Different size variants should have independent cache identities.

---

# 37. WHY

A 64×64 artwork asset should not require decoding a 1000×1000 image every time.

---

# 38. RESIZE QUALITY

Use a suitable high-quality resampling algorithm.

The implementation should balance:

- quality
- CPU cost
- memory.

---

# 39. PI ZERO CONSTRAINT

The Pi Zero W has limited CPU resources.

Avoid unnecessarily expensive image processing during active playback.

---

# 40. BACKGROUND PROCESSING

Image decoding and resizing should happen outside the UI thread.

---

# 41. CACHE TYPES

The system should conceptually maintain:

SOURCE CACHE

and:

DECODED CACHE.

---

# 42. SOURCE CACHE

Source cache may contain:

- extracted embedded image
- external image path
- source signature.

---

# 43. DECODED CACHE

Decoded cache contains renderer-ready image assets.

---

# 44. CACHE MEMORY

Decoded images can consume significantly more memory than compressed files.

For example:

a compressed JPEG may be hundreds of KB

while a decoded RGB image can consume several MB.

Therefore:

CACHE SIZE MUST BE CONTROLLED.

---

# 45. MEMORY BUDGET

Artwork cache should use an explicit memory budget.

The exact value belongs to the implementation configuration.

---

# 46. LRU EVICTION

An LRU cache is recommended for decoded artwork.

---

# 47. CACHE EVICTION

When the artwork cache exceeds its budget:

evict least-recently-used assets that are not currently referenced.

---

# 48. ACTIVE ASSETS

The current player's artwork should not be evicted while actively being rendered.

---

# 49. REFERENCE COUNTING

The implementation may use reference counting or ownership handles for assets currently in use.

---

# 50. UI ASSET HANDLE

The UI should receive an artwork handle/reference rather than directly owning the cache.

---

# 51. CACHE THREAD SAFETY

Artwork cache access must be synchronized appropriately.

---

# 52. NO GLOBAL UNSAFE STATE

Do not use unsynchronized global image pointers.

---

# 53. ASYNC REQUESTS

Artwork requests should be asynchronous.

Conceptually:

request_artwork(track_id, size_class)

---

# 54. REQUEST RESULT

A request may result in:

ARTWORK_READY

ARTWORK_MISSING

ARTWORK_INVALID

ARTWORK_ERROR.

---

# 55. REQUEST ID

Artwork requests should have an identifier.

---

# 56. STALE REQUEST PROTECTION

If the user changes tracks before artwork loading completes:

the old artwork result must not replace the new track's artwork.

---

# 57. PLAYBACK GENERATION

Artwork requests associated with the player should carry the active playback generation where relevant.

---

# 58. EXAMPLE

Track A artwork is loading.

User skips to Track B.

Track B becomes current.

A's artwork finishes loading.

The UI must not display A's artwork as B's current artwork.

---

# 59. TRACK ID CHECK

At minimum:

artwork result must be checked against the currently requested track.

---

# 60. GENERATION CHECK

For player-critical artwork:

track ID + playback generation is preferred.

---

# 61. SCREEN-SPECIFIC REQUESTS

Artwork loaded for a library screen may remain valid even if playback changes.

Therefore:

not every artwork request needs playback generation.

---

# 62. REQUEST CONTEXT

Artwork requests should identify their consumer/context when necessary.

Examples:

- library
- album view
- player
- mini-player.

---

# 63. PRIORITY

Player artwork should have higher priority than distant library thumbnails.

---

# 64. REQUEST QUEUE

The artwork worker should prioritize visible/current requests.

---

# 65. VISIBLE UI

Visible artwork should be loaded before off-screen artwork.

---

# 66. THUMBNAIL FLOOD

Scrolling through a large library should not create an unlimited backlog of artwork requests.

---

# 67. REQUEST COALESCING

If the same artwork is requested multiple times:

combine requests when possible.

---

# 68. SAME SOURCE

Example:

Album A contains 12 tracks.

All tracks use the same album artwork.

Do not decode the same image 12 times.

---

# 69. SHARED ARTWORK

Decoded artwork should be reusable between tracks when their artwork keys match.

---

# 70. ALBUM ARTWORK KEY

An album-level artwork key may be used where the artwork source is shared.

---

# 71. TRACK-SPECIFIC ARTWORK

If two tracks contain different embedded covers:

their artwork keys must remain distinct.

---

# 72. EXTERNAL FILE PRIORITY

If an external cover is explicitly selected:

do not silently replace it with embedded artwork unless the selection policy changes.

---

# 73. USER OVERRIDE

The architecture should permit future user-selected artwork.

---

# 74. USER OVERRIDE PERSISTENCE

A future implementation may persist artwork overrides per:

track

or:

album.

---

# 75. MISSING ARTWORK

Missing artwork is a normal state.

It must not be treated as an audio failure.

---

# 76. FALLBACK ARTWORK

The UI should have a lightweight fallback asset.

---

# 77. FALLBACK ASSET

Fallback artwork should be:

- small
- locally stored
- always available
- inexpensive to render.

---

# 78. INVALID ARTWORK

Malformed image data must not crash FLACHEAD.

---

# 79. DECODER FAILURE

If image decoding fails:

return ARTWORK_INVALID/ERROR.

Do not propagate the failure into playback.

---

# 80. CORRUPT EMBEDDED IMAGE

A corrupt embedded picture should not invalidate the FLAC file.

---

# 81. CORRUPT EXTERNAL IMAGE

A corrupt external image should not prevent track playback.

---

# 82. FALLBACK CHAIN

Recommended fallback:

embedded front cover

↓

external cover

↓

external directory artwork

↓

fallback asset.

---

# 83. FALLBACK FAILURE

If even the fallback asset cannot be loaded:

the UI should render without artwork.

---

# 84. NO ARTWORK MODE

The player must remain completely functional without artwork.

---

# 85. COLOR EXTRACTION

Artwork may be used to derive UI colors.

This should be a separate operation.

---

# 86. COLOR EXTRACTION OWNERSHIP

Artwork system may expose a decoded image.

Theme engine decides how to derive colors.

---

# 87. COLOR EXTRACTION COST

Do not repeatedly analyze the same artwork every frame.

---

# 88. COLOR CACHE

Derived theme colors may be cached alongside artwork identity.

---

# 89. ARTWORK ANIMATION

Artwork transitions belong to the UI/rendering layer.

The artwork service should provide stable assets.

---

# 90. CROSSFADE

If artwork crossfading is used:

the renderer controls the transition.

---

# 91. ARTWORK ROTATION

Artwork should not be rotated automatically unless metadata/UI rules require it.

---

# 92. ASPECT RATIO

Preserve source aspect ratio.

---

# 93. FIT MODES

The UI may request:

- contain
- cover
- crop.

The artwork subsystem should provide the image; the renderer decides final placement.

---

# 94. CROPPING

Cropping must not modify the cached source image.

---

# 95. UPSCALING

Avoid excessive upscaling of small artwork.

---

# 96. LOW-RES ARTWORK

If source artwork is smaller than the requested display size:

use the best available source without inventing detail.

---

# 97. IMAGE FORMAT NORMALIZATION

The cache may normalize multiple source formats into one internal representation.

---

# 98. JPEG

JPEG is suitable for photographic album artwork.

---

# 99. PNG

PNG is suitable for artwork containing transparency or sharp graphics.

---

# 100. WEBP

WebP may be supported if the chosen decoder is stable and lightweight enough for the Pi Zero W.

---

# 101. UNSUPPORTED FORMAT

Unsupported artwork formats should fall through to another source.

---

# 102. FILE ACCESS

External artwork file access should use safe path handling.

---

# 103. PATH SECURITY

Artwork discovery must not allow malformed metadata/path data to escape configured music directories when resolving relative artwork.

---

# 104. SYMLINK POLICY

The implementation should define whether artwork discovery follows symlinks.

It should avoid unexpected access outside configured music roots.

---

# 105. DIRECTORY BOUNDARIES

Relative artwork lookup should remain within permitted library paths unless explicitly configured otherwise.

---

# 106. EMBEDDED EXTRACTION SAFETY

Embedded image sizes must be validated before allocation.

---

# 107. SIZE LIMIT

Reject or safely handle absurdly large embedded images.

---

# 108. DECOMPRESSION BOMB PROTECTION

Image decoders should not be allowed to allocate uncontrolled memory from maliciously large dimensions.

---

# 109. DIMENSION VALIDATION

Validate:

- width
- height
- channels
- decoded size.

before committing large allocations.

---

# 110. INTEGER OVERFLOW

Image-size calculations must guard against integer overflow.

---

# 111. MALFORMED IMAGE

Malformed images must fail gracefully.

---

# 112. CACHE CORRUPTION

If a cached artwork asset is corrupt:

invalidate that cache entry and regenerate it.

---

# 113. CACHE VERSION

Artwork cache format should include a version.

---

# 114. DECODER VERSION

If the internal decoded representation changes:

cache invalidation should occur.

---

# 115. CACHE DIRECTORY

Artwork cache should live under the FLACHEAD application data/cache area.

---

# 116. ATOMIC CACHE WRITES

Disk cache files should be written atomically.

Avoid exposing partially written artwork.

---

# 117. CACHE CLEANUP

The system should support cleanup of:

- stale files
- oversized cache
- obsolete variants.

---

# 118. CACHE SIZE LIMIT

A configurable maximum cache size should exist.

---

# 119. STARTUP CACHE SCAN

Do not perform expensive full-cache validation on every startup.

Use lightweight metadata/index information where possible.

---

# 120. LAZY VALIDATION

Validate individual cached artwork when it is requested.

---

# 121. CACHE MISS

On cache miss:

load source

↓

decode

↓

resize

↓

cache

↓

publish.

---

# 122. CACHE HIT

On cache hit:

load decoded asset/reference

↓

publish immediately.

---

# 123. CACHE FAILURE

If disk cache fails:

continue using memory/cache pipeline.

Playback remains unaffected.

---

# 124. ARTWORK AND PLAYBACK

Artwork must never be a prerequisite for starting audio.

---

# 125. TRACK START

Recommended sequence:

track opens

↓

audio playback begins

↓

artwork request begins/continues asynchronously.

---

# 126. WHY

A slow artwork decoder must not delay audio unnecessarily.

---

# 127. ARTWORK LOAD FAILURE

If artwork fails:

audio continues.

---

# 128. ARTWORK ARRIVAL

If artwork finishes after playback begins:

the UI may replace the fallback artwork with the loaded artwork.

---

# 129. NO AUDIO COUPLING

Do not make:

PLAY

wait for:

ARTWORK_READY.

---

# 130. QUEUE INTEGRATION

Queue entries reference tracks.

Artwork resolves independently from queue ordering.

---

# 131. METADATA INTEGRATION

Metadata exposes artwork source information.

Artwork system performs actual image processing.

---

# 132. LYRICS INTEGRATION

Lyrics and artwork may share track identity.

Neither subsystem should depend on the other.

---

# 133. PLAYER INTEGRATION

The full player requests:

current track artwork.

---

# 134. MINI PLAYER

The mini-player may request a smaller artwork size class.

---

# 135. LIBRARY LIST

Library lists should request thumbnail-size assets.

---

# 136. ALBUM VIEW

Album views may request medium-size artwork.

---

# 137. SETTINGS

Settings screens should not unnecessarily load large artwork.

---

# 138. TASK OVERVIEW

Task overview cards should use cached thumbnails or lightweight references.

---

# 139. SCREEN TRANSITION

Changing screens must not cause redundant artwork decoding if the required asset is already cached.

---

# 140. ARTWORK PREFETCH

When opening the full player:

current artwork should have highest priority.

---

# 141. NEXT TRACK PREFETCH

The next track's artwork may be prefetched if memory allows.

---

# 142. PREFETCH CANCELLATION

If the user rapidly changes tracks:

obsolete low-priority artwork requests may be cancelled.

---

# 143. REQUEST CANCELLATION

Cancellation should be cooperative.

Do not leave worker threads blocked indefinitely.

---

# 144. WORKER SHUTDOWN

Artwork workers must shut down cleanly during application exit.

---

# 145. RESOURCE LIFECYCLE

Artwork resources should follow the application's documented resource lifecycle:

load

↓

use

↓

release

↓

evict.

---

# 146. GPU/RENDERER OWNERSHIP

If SDL textures are used:

the renderer should own the GPU/renderer-specific texture object.

The artwork cache may own CPU-side decoded data.

---

# 147. TEXTURE CREATION

Texture creation should occur on the appropriate rendering thread/context.

---

# 148. THREAD RESTRICTION

Do not create renderer-dependent resources from arbitrary worker threads if the rendering backend does not permit it.

---

# 149. CPU IMAGE VS RENDER TEXTURE

Keep these conceptually separate:

CPU decoded image

and:

renderer texture.

---

# 150. WHY

This prevents rendering-context ownership problems and makes caching more predictable.

---

# 151. TEXTURE CACHE

A separate renderer texture cache may be used.

It should have its own memory budget.

---

# 152. TEXTURE EVICTION

Renderer textures may be evicted independently of CPU decoded assets.

---

# 153. DISPLAY SCALE

The cache should avoid creating dozens of nearly identical sizes.

Use predefined size classes.

---

# 154. ORIENTATION

Artwork itself should remain orientation-neutral.

The portrait display layout belongs to the renderer/UI.

---

# 155. IMAGE QUALITY

The final artwork pipeline should prioritize:

correctness

then:

reasonable visual quality

then:

performance optimization.

---

# 156. PERFORMANCE TARGET

Artwork processing must not cause:

- audio underruns
- UI freezes
- long input latency
- uncontrolled memory growth.

---

# 157. PERFORMANCE MONITORING

The implementation may record:

- artwork decode time
- resize time
- cache hit rate
- cache miss rate
- memory usage
- request queue length.

---

# 158. DEBUGGING

Debug logs may report:

ARTWORK_REQUEST

ARTWORK_CACHE_HIT

ARTWORK_CACHE_MISS

ARTWORK_DECODED

ARTWORK_FAILED

ARTWORK_EVICTED.

---

# 159. LOGGING RESTRICTION

Do not spam logs on every rendered frame.

---

# 160. METRICS

Useful metrics include:

cache hit ratio

average decode time

average resize time

active artwork memory.

---

# 161. ACCEPTANCE TEST: EMBEDDED ARTWORK

Use a FLAC containing a valid front-cover image.

Verify:

artwork is discovered

and:

displayed correctly.

---

# 162. ACCEPTANCE TEST: EXTERNAL ARTWORK

Place:

cover.jpg

beside a track.

Verify:

external artwork is discovered.

---

# 163. ACCEPTANCE TEST: PRIORITY

Provide both embedded and external artwork.

Verify:

the configured priority is respected.

---

# 164. ACCEPTANCE TEST: SHARED ALBUM ART

Give multiple tracks the same artwork source.

Verify:

the artwork is decoded/cached efficiently.

---

# 165. ACCEPTANCE TEST: MISSING ARTWORK

Use a track with no artwork.

Verify:

fallback artwork appears.

---

# 166. ACCEPTANCE TEST: INVALID ARTWORK

Use a corrupt image.

Verify:

fallback appears

and:

playback continues.

---

# 167. ACCEPTANCE TEST: LARGE IMAGE

Use an excessively large source image.

Verify:

memory usage remains bounded.

---

# 168. ACCEPTANCE TEST: CACHE HIT

Load artwork twice.

Verify:

second access uses cache where appropriate.

---

# 169. ACCEPTANCE TEST: CACHE INVALIDATION

Modify external artwork.

Verify:

old artwork is not permanently reused.

---

# 170. ACCEPTANCE TEST: STALE REQUEST

Begin loading artwork for Track A.

Switch to Track B.

Verify:

A cannot replace B's player artwork.

---

# 171. ACCEPTANCE TEST: PLAYBACK INDEPENDENCE

Delay artwork decoding.

Verify:

audio begins and continues normally.

---

# 172. ACCEPTANCE TEST: UI THREAD

Load a large artwork file.

Verify:

UI remains responsive.

---

# 173. ACCEPTANCE TEST: AUDIO THREAD

Run artwork loading during playback.

Verify:

no artwork I/O occurs on the audio thread.

---

# 174. ACCEPTANCE TEST: MEMORY

Scroll through a large artwork-heavy library.

Verify:

decoded artwork memory remains within its configured budget.

---

# 175. ACCEPTANCE TEST: RAPID TRACK CHANGES

Rapidly switch tracks.

Verify:

artwork results resolve to the correct track.

---

# 176. ACCEPTANCE TEST: SHARED CACHE

Open the same album in:

library

album view

player.

Verify:

unnecessary duplicate decoding does not occur.

---

# 177. ACCEPTANCE TEST: CACHE CORRUPTION

Corrupt a cached artwork asset.

Verify:

it is invalidated and regenerated.

---

# 178. ACCEPTANCE TEST: DATABASE FAILURE

Make metadata unavailable.

Verify:

direct artwork loading can still follow a safe source path where supported.

---

# 179. ACCEPTANCE CRITERIA

The artwork architecture is production-ready when:

- embedded artwork is supported
- external artwork is supported
- source priority is deterministic
- artwork identity is stable
- artwork changes invalidate stale cache entries
- artwork is loaded asynchronously
- UI never decodes large artwork synchronously
- audio threads never perform artwork operations
- decoded artwork uses bounded memory
- LRU eviction is supported
- artwork variants are supported
- shared artwork is reused
- missing artwork is safe
- malformed artwork is safe
- oversized images are protected against
- integer overflow is prevented
- cache writes are safe
- cache corruption is recoverable
- current-player artwork has appropriate priority
- stale asynchronous results cannot replace current artwork
- artwork processing never blocks audio playback
- artwork remains independent from lyrics and queue management.

---

# 180. AI CODING AGENT RULES

The coding agent MUST:

1. Treat artwork as presentation data.
2. Keep artwork independent from audio decoding.
3. Keep artwork independent from playback timing.
4. Keep artwork independent from queue management.
5. Support embedded FLAC artwork.
6. Support external artwork.
7. Use deterministic artwork-source priority.
8. Prefer front-cover artwork.
9. Support artwork cache keys.
10. Track source signatures.
11. Detect stale artwork.
12. Decode artwork asynchronously.
13. Resize artwork asynchronously.
14. Keep large image decoding off the UI thread.
15. Keep all artwork operations off the audio thread.
16. Use bounded decoded-image memory.
17. Use LRU-style eviction.
18. Reuse identical artwork across tracks.
19. Support multiple size classes.
20. Avoid unnecessary full-resolution retention.
21. Validate image dimensions before allocation.
22. Protect against excessive decoded memory.
23. Protect against integer overflow.
24. Handle malformed images safely.
25. Provide fallback artwork.
26. Never make artwork required for playback.
27. Never make PLAY wait for artwork.
28. Protect against stale asynchronous results.
29. Keep CPU image data separate from renderer textures.
30. Create renderer-specific resources only from the appropriate rendering context/thread.
31. Keep cache failures non-fatal.
32. Keep artwork processing appropriate for the Pi Zero W.
33. Never allow artwork memory growth to become unbounded.
34. Do not perform per-frame artwork decoding.
35. Never let artwork failures stop audio playback.

---

# 181. FINAL ARTWORK ARCHITECTURE

The intended architecture is:

                         TRACK
                           │
                           ▼
                    METADATA SYSTEM
                           │
                           ▼
                    ARTWORK SOURCE
                           │
              ┌────────────┴────────────┐
              ▼                         ▼
        EMBEDDED COVER             EXTERNAL COVER
              │                         │
              └────────────┬────────────┘
                           ▼
                    ARTWORK LOADER
                           │
                           ▼
                     IMAGE DECODER
                           │
                           ▼
                     IMAGE RESIZER
                           │
                           ▼
                    CPU IMAGE CACHE
                           │
                           ▼
                  RENDERER TEXTURE CACHE
                           │
                           ▼
                           UI


The central invariant is:

ARTWORK MUST NEVER BE REQUIRED FOR AUDIO PLAYBACK.

The second invariant is:

ARTWORK DECODING MUST NEVER BLOCK THE AUDIO THREAD.

The third invariant is:

THE UI MUST NOT SYNCHRONOUSLY DECODE LARGE ARTWORK.

The fourth invariant is:

STALE ARTWORK REQUESTS MUST NEVER REPLACE THE CURRENT TRACK'S ARTWORK.

The fifth invariant is:

DECODED ARTWORK MEMORY MUST REMAIN BOUNDED.

Final policy:

Prefer embedded front-cover artwork.

Support external artwork.

Use deterministic source selection.

Load artwork lazily.

Decode and resize asynchronously.

Cache decoded assets.

Reuse identical artwork.

Use size classes.

Protect against oversized/malformed images.

Use fallback artwork when necessary.

Keep CPU image data separate from renderer textures.

Evict unused assets under memory pressure.

Prioritize current-player artwork.

And always preserve the separation:

METADATA = ARTWORK DESCRIPTION/SOURCE

ARTWORK SYSTEM = IMAGE PROCESSING

CPU CACHE = DECODED IMAGE

RENDERER CACHE = DISPLAY RESOURCE

UI = PRESENTATION

PLAYBACK ENGINE = AUDIO EXECUTION