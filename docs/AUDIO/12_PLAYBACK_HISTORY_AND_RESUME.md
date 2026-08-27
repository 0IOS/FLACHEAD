# FLACHEAD AUDIO PLAYBACK HISTORY AND RESUME ARCHITECTURE

Document

20_AUDIO_PLAYBACK_HISTORY_AND_RESUME_ARCHITECTURE.md

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

This document defines how FLACHEAD remembers playback history, recently played tracks, playback position, resume state, and related user playback information.

The system covers:

- current playback persistence
- resume positions
- recently played tracks
- play counts
- last-played timestamps
- automatic resume
- manual restart
- track completion
- queue interaction
- crashes
- unexpected power loss
- database persistence
- storage removal
- metadata changes
- track identity
- privacy
- write frequency
- SD-card wear
- recovery

The primary objective is:

ALLOW THE USER TO RETURN TO THEIR MUSIC WITHOUT LOSING IMPORTANT PLAYBACK STATE WHILE MINIMIZING STORAGE WRITES ON THE RASPBERRY PI ZERO W.

---

# 2. CORE PRINCIPLE

Playback history is:

PERSISTENT USER STATE.

Playback position is:

PERSISTENT RESUME STATE.

Neither should be treated as part of the audio decoder's internal state.

---

# 3. AUTHORITATIVE PLAYBACK STATE

The live playback engine remains authoritative while the application is running.

Persistent storage is only a saved snapshot.

Therefore:

LIVE PLAYBACK STATE
    ↓
PERSISTENCE LAYER
    ↓
STORED RESUME/HISTORY STATE

On startup:

STORED STATE
    ↓
PLAYBACK ENGINE
    ↓
LIVE PLAYBACK STATE

---

# 4. TRACK IDENTITY

Resume state must reference a stable:

track_id.

It must not rely solely on:

- filename
- title
- album
- artist.

---

# 5. TRACK ID VS PATH

A path can change when a user:

- renames a file
- moves an album
- reorganizes folders
- swaps storage.

Therefore path alone is insufficient as the logical identity.

---

# 6. TRACK ID PERSISTENCE

If the library can confidently recognize that a moved file is the same logical track:

preserve its track_id.

If identity cannot be established:

do not aggressively merge records.

---

# 7. RESUME RECORD

Conceptual model:

ResumeRecord

    track_id
    position_frames
    duration_frames
    last_played_timestamp
    update_timestamp
    completed
    library_generation

---

# 8. POSITION UNIT

The preferred internal persistence unit is:

audio frames.

This avoids repeated floating-point conversion.

---

# 9. POSITION VALIDATION

Before resuming:

stored position must satisfy:

0 <= position < duration

If the position is invalid:

clamp or discard it according to policy.

---

# 10. RESUME THRESHOLD

Very small positions should not necessarily be saved as meaningful resume points.

Example:

If playback has only reached the first few seconds:

the track may be treated as:

not meaningfully started.

---

# 11. END THRESHOLD

A track near completion should normally be treated as:

completed.

Recommended conceptual behavior:

if remaining time <= configured completion threshold:

mark completed

and reset resume position.

---

# 12. WHY COMPLETION MATTERS

Without a completion policy:

opening a finished track could resume at:

99% complete

instead of starting from the beginning.

---

# 13. RECOMMENDED COMPLETION BEHAVIOR

When a track is considered completed:

- mark completed
- reset saved resume position
- update last-played state
- allow next playback to begin from the start.

---

# 14. MANUAL RESTART

The user must be able to explicitly restart a track.

Restart should:

set position to zero

and clear the meaningful resume point.

---

# 15. MANUAL RESUME

If a track has a valid saved position:

the UI may provide:

RESUME

or:

START FROM BEGINNING.

---

# 16. AUTO-RESUME

Automatic resume behavior should be configurable.

Recommended default:

resume the previously active track if it has a meaningful saved position.

---

# 17. STARTUP STATE

On application startup:

load persistent playback state.

Do not immediately begin playback unless the user has explicitly enabled:

AUTO RESUME ON STARTUP.

---

# 18. AUTO-RESUME SAFETY

Startup auto-resume must not accidentally start music because of:

- a stale crash record
- a corrupted state
- a missing file
- an unavailable storage root.

---

# 19. CRASH RECOVERY

If FLACHEAD crashes during playback:

the last persisted resume checkpoint should be used.

Do not attempt to reconstruct an exact sample position after the crash unless such information was safely persisted.

---

# 20. EXPECTED RESUME ERROR

A small discrepancy between:

last audible position

and:

last saved position

is acceptable.

The persistence system should prioritize:

reliability

over:

constant storage writes.

---

# 21. CHECKPOINTING

The system should periodically save playback progress.

It should NOT write to persistent storage for every playback position update.

---

# 22. CHECKPOINT INTERVAL

A practical initial policy is:

save approximately every 5–15 seconds during active playback.

The exact interval should be benchmarked.

---

# 23. CHECKPOINT EVENTS

A position checkpoint should also occur on important events:

- pause
- manual seek
- track change
- application shutdown
- playback stop
- track completion.

---

# 24. SEEK CHECKPOINT

After a user seeks:

save the new position after the seek has been accepted by the playback engine.

Do not persist a seek request that failed.

---

# 25. PAUSE CHECKPOINT

When the user pauses:

persist the current authoritative playback position.

---

# 26. STOP CHECKPOINT

When playback is explicitly stopped:

persist according to the configured stop semantics.

---

# 27. SHUTDOWN CHECKPOINT

On graceful shutdown:

persist the current playback state before terminating.

---

# 28. POWER LOSS

Unexpected power loss cannot guarantee a final write.

Therefore periodic checkpoints are required.

---

# 29. SD-CARD WEAR

The Raspberry Pi Zero W may use SD-card storage.

Excessive position writes should therefore be avoided.

---

# 30. WRITE COALESCING

If several state updates occur rapidly:

combine them into one persistence operation.

---

# 31. DIRTY STATE

Maintain an in-memory:

resume_dirty

flag.

Only persist when necessary.

---

# 32. PERSISTENCE WORKER

A dedicated persistence worker may receive:

SAVE_RESUME_STATE

commands.

It should not block the audio thread.

---

# 33. AUDIO THREAD RULE

The audio output thread MUST NOT perform:

database writes

filesystem writes

or blocking persistence operations.

---

# 34. UI THREAD RULE

The UI thread should not synchronously write playback history during normal rendering.

---

# 35. DATABASE

SQLite is an appropriate persistence mechanism for:

- resume records
- history
- play counts
- timestamps.

The implementation must remain consistent with the project's overall database architecture.

---

# 36. HISTORY MODEL

Conceptual history record:

HistoryRecord

    track_id
    played_at
    duration_played_frames
    completed

---

# 37. RECENTLY PLAYED

The UI may expose a:

RECENTLY PLAYED

section.

It should query persisted history rather than reconstructing it from the filesystem.

---

# 38. HISTORY ORDER

History should be sorted by:

played_at DESC.

---

# 39. HISTORY DUPLICATES

Playing the same track multiple times should create distinct play events if play history is intended to represent actual listening sessions.

---

# 40. HISTORY RETENTION

History should be bounded.

Recommended initial policy:

retain a configurable number of recent play events.

---

# 41. RETENTION EXAMPLE

Possible default:

500 recent play events.

This is an implementation parameter, not a hard architectural requirement.

---

# 42. PLAY COUNT

The library may maintain:

play_count.

A play should only count after the track has been meaningfully started.

---

# 43. PLAY COUNT THRESHOLD

Starting a track and immediately skipping it should not necessarily count as a play.

Use a minimum playback threshold.

---

# 44. PLAY COUNT COMPLETION

The system may count a play when:

- a configured percentage is reached
- or a configured minimum duration is reached.

The exact policy should be centralized.

---

# 45. PLAY COUNT CONSISTENCY

Play count updates should be idempotent where practical.

Avoid double-counting because of duplicate lifecycle events.

---

# 46. LAST PLAYED

Each track may store:

last_played_at.

This is useful for:

- recent tracks
- sorting
- history
- recommendations.

---

# 47. TIMESTAMP SOURCE

Persist timestamps using a consistent wall-clock source.

Playback position itself must still use the audio clock.

---

# 48. CLOCK CHANGE

System wall-clock changes must not corrupt playback positions.

Resume positions are frame-based.

---

# 49. HISTORY PRIVACY

Playback history is local user data.

It should not be transmitted anywhere by default.

---

# 50. NETWORK

The history/resume system must not require internet access.

---

# 51. DATA LOCATION

Persistent playback state should live in FLACHEAD's application data directory/database.

Do not modify audio files merely to store playback position.

---

# 52. AUDIO FILE MODIFICATION

FLACHEAD must not write resume metadata into FLAC files during normal operation.

---

# 53. METADATA SEPARATION

Playback state is application state.

FLAC metadata is media metadata.

Keep them separate.

---

# 54. QUEUE

Resume state for a track does not imply that the previous queue must be restored automatically.

These are separate policies.

---

# 55. QUEUE RESUME

If queue restoration is implemented:

store queue state separately from track resume state.

---

# 56. CURRENT TRACK

The persistent playback snapshot may contain:

current_track_id.

---

# 57. CURRENT QUEUE POSITION

If queue persistence is enabled:

store:

queue_id

or:

queue snapshot identity

and:

current queue index.

---

# 58. INITIAL IMPLEMENTATION

Do not make complete queue persistence mandatory unless required by the product behavior.

Track resume should work independently.

---

# 59. TRACK CHANGE

When moving to the next track:

1. checkpoint the previous track
2. determine whether it completed
3. update history
4. establish the new current track
5. reset the new track's live position
6. begin checkpointing the new track.

---

# 60. PREVIOUS TRACK

A track that was skipped early should retain its previous meaningful resume position if policy allows.

---

# 61. SKIP POLICY

The system must distinguish:

USER SKIP

TRACK COMPLETION

NATURAL STOP

APPLICATION EXIT

These events may have different persistence behavior.

---

# 62. USER SKIP

On manual skip:

save the current position before changing tracks.

---

# 63. NATURAL COMPLETION

On natural completion:

mark the track completed.

Reset its resume position.

---

# 64. APPLICATION EXIT

On graceful exit:

save current track and position.

---

# 65. PAUSE AND EXIT

If the player is paused and the application exits:

the paused position should be persisted.

---

# 66. MISSING TRACK

If the stored current track no longer exists:

do not crash.

---

# 67. MISSING TRACK STARTUP

On startup:

if current_track_id cannot be resolved:

load the library normally.

Clear invalid current playback state.

---

# 68. UNAVAILABLE STORAGE

If the music root is unavailable:

retain the persistent current track identity.

Do not destroy it merely because storage is temporarily absent.

---

# 69. STORAGE RETURN

When storage returns:

attempt to resolve the stored track identity again.

---

# 70. TRACK ID RESOLUTION

Resolution priority may be:

1. stable track_id
2. strong file identity
3. canonical path
4. conservative metadata fallback.

Do not perform aggressive fuzzy matching automatically.

---

# 71. METADATA CHANGE

Changing:

title

artist

album

should not automatically destroy resume state if the underlying track identity remains the same.

---

# 72. FILE REPLACEMENT

If a file is replaced with a completely different audio file at the same path:

path alone should not guarantee identity.

File identity information should be re-evaluated.

---

# 73. DURATION CHANGE

If a track's duration changes:

the stored resume position must be validated against the new duration.

---

# 74. RESUME POSITION TOO LARGE

If:

stored_position >= new_duration

discard the saved position.

---

# 75. RESUME POSITION NEAR END

If the stored position is very close to the new end:

treat it according to completion policy.

---

# 76. CORRUPTED RECORD

If a resume record is malformed:

ignore that record.

Do not crash the application.

---

# 77. DATABASE FAILURE

If the persistence database cannot be opened:

audio playback should remain functional.

---

# 78. PERSISTENCE FAILURE

A failed history write must not stop playback.

Log the error and continue with in-memory state.

---

# 79. RETRY

Persistence failures may be retried with bounded backoff.

Do not retry indefinitely in a tight loop.

---

# 80. DEGRADED MODE

If persistent storage becomes unavailable:

FLACHEAD may continue using:

in-memory resume/history state.

---

# 81. RECOVERY FROM DEGRADED MODE

When persistence becomes available again:

flush the latest valid state.

Older intermediate states do not need to be replayed.

---

# 82. LATEST STATE WINS

For resume position:

the newest valid checkpoint is authoritative.

Do not replay obsolete position writes.

---

# 83. HISTORY VS RESUME

History is append-like.

Resume state is latest-state-wins.

They should not share identical persistence semantics.

---

# 84. HISTORY WRITE BATCHING

Multiple history changes may be batched where appropriate.

However:

a completed play event should not be lost unnecessarily.

---

# 85. RESUME CHECKPOINT COALESCING

Example:

10 s

15 s

20 s

25 s

If the persistence worker has not yet committed:

only the latest necessary position needs to be written.

---

# 86. POSITION MONOTONICITY

During ordinary playback:

position should increase.

During seek:

position may jump.

The persistence system must support both.

---

# 87. BACKWARD SEEK

After backward seek:

the saved position should reflect the new location.

Do not retain the old later position.

---

# 88. FORWARD SEEK

After forward seek:

save the new position after the seek is confirmed.

---

# 89. RAPID SEEK

Rapid seeks should be coalesced.

Persist the final accepted position rather than every intermediate drag position.

---

# 90. SCRUBBING

While the user is continuously dragging a seek control:

do not write every intermediate position.

Persist after:

seek commit

or:

interaction release.

---

# 91. HISTORY EVENT TIMING

A track should not be added to history merely because:

the user selected it in the UI.

It should be recorded when playback meaningfully starts.

---

# 92. PLAYBACK START

At actual playback start:

record the start event in memory.

---

# 93. PLAYBACK INTERRUPTION

If playback stops very quickly:

apply the minimum-play threshold before counting it as a play.

---

# 94. REPEATED TRACK

If repeat-one is enabled:

each meaningful completed playback can count as another play event according to policy.

---

# 95. SHUFFLE

Shuffle order does not change track identity.

History remains track-based.

---

# 96. RECENTLY PLAYED

Recently played should reflect actual playback events.

Selecting a track without playing it should not move it to the top.

---

# 97. HISTORY SCREEN

The UI should query history using bounded results.

Do not load the complete history database into RAM.

---

# 98. HISTORY PAGINATION

If the user scrolls beyond the first history page:

load additional records on demand.

---

# 99. HISTORY CLEAR

The user may optionally clear playback history.

Clearing history should not necessarily clear resume positions unless explicitly requested.

---

# 100. CLEAR HISTORY VS CLEAR RESUME

These are separate operations:

CLEAR PLAY HISTORY

CLEAR RESUME DATA

Do not combine them accidentally.

---

# 101. RESET PLAYER STATE

A factory/reset operation may clear:

- history
- resume positions
- queue state
- preferences

according to explicit reset policy.

---

# 102. DATABASE TRANSACTIONS

Related updates should use transactions.

Example:

track completion may update:

- play count
- last played
- history event
- resume state.

These should be persisted consistently.

---

# 103. TRANSACTION FAILURE

If a transaction fails:

do not leave the application in a crash loop.

Retain the desired state in memory and retry safely.

---

# 104. ATOMICITY

The system should avoid states where:

history says a track completed

while:

resume state still incorrectly indicates an old active position.

A transaction can enforce consistency.

---

# 105. STARTUP LOAD ORDER

Recommended startup order:

1. initialize database
2. load persistent playback snapshot
3. initialize library
4. resolve current track
5. initialize audio system
6. wait for explicit or configured auto-resume
7. begin playback.

---

# 106. NO AUTOMATIC AUDIO THREAD START

Persistence loading must not directly start the audio output thread.

Playback commands remain under playback-engine control.

---

# 107. UI STARTUP

The UI should be able to display:

last track

last position

or:

resume available

before audio begins.

---

# 108. RESUME CONFIRMATION

If automatic resume is disabled:

the UI may show:

RESUME LAST TRACK

instead of automatically starting.

---

# 109. USER CONTROL

The user should be able to choose:

resume

or:

start over.

---

# 110. RESUME DISPLAY

The UI may show:

RESUME 42:17

based on stored position.

---

# 111. POSITION DISPLAY

Stored resume time should use the same formatting rules as live playback time.

---

# 112. LONG TRACKS

Resume must support tracks longer than:

32-bit signed integer seconds.

Use 64-bit frame/time representation where appropriate.

---

# 113. HIGH SAMPLE RATE

Resume calculations must remain correct for:

44.1 kHz

48 kHz

88.2 kHz

96 kHz

and other supported rates.

---

# 114. SAMPLE RATE CHANGES

A saved position should represent a logical time/frame location associated with the track.

If internal representation depends on sample rate:

store enough information to validate/recalculate safely.

---

# 115. DURATION SNAPSHOT

Store duration information with the resume record when useful for validation.

---

# 116. POSITION ACCURACY

Resume accuracy should be sufficient to return the listener close to the previously saved location.

Exact sample-perfect restoration is not required.

---

# 117. AUDIO BUFFER DIFFERENCE

The saved position should represent the playback timeline rather than the amount of PCM already decoded.

---

# 118. RESUME AND LYRICS

When resuming:

lyrics must synchronize to the same restored playback position.

---

# 119. RESUME AND PROGRESS UI

The progress bar, elapsed time, and lyrics should all start from the same restored position.

---

# 120. RESUME AND ARTWORK

Artwork loading is independent.

It must not delay playback unnecessarily.

---

# 121. RESUME AND METADATA

If metadata loads slightly later:

resume should still use track identity and playback position.

---

# 122. RESUME AND LIBRARY SCAN

The player should be able to resolve the saved track using the existing library cache while a background scan is still running.

---

# 123. SCAN GENERATION

If the library is being reconciled:

the resume system must not accept stale library records over newer ones.

---

# 124. CONCURRENCY

Persistence, library scanning, playback, and UI operations may run concurrently.

Shared state must have clear ownership.

---

# 125. OWNERSHIP

Playback engine owns:

live playback position.

Persistence layer owns:

stored resume/history state.

Library service owns:

track identity and availability.

UI owns:

presentation and user interaction.

---

# 126. NO DIRECT DATABASE ACCESS FROM UI

The UI should call a service/API.

It should not directly manipulate persistence tables.

---

# 127. NO DIRECT DATABASE ACCESS FROM AUDIO OUTPUT

The audio output thread must not access the database.

---

# 128. EVENT MODEL

Useful events include:

PLAYBACK_STARTED

PLAYBACK_PAUSED

PLAYBACK_SEEKED

PLAYBACK_STOPPED

TRACK_CHANGED

TRACK_COMPLETED

PLAYBACK_ERROR

APPLICATION_SHUTDOWN.

---

# 129. EVENT HANDLING

The persistence layer subscribes to relevant playback lifecycle events.

It does not control the playback engine.

---

# 130. STALE EVENTS

Playback events should carry:

track_id

and:

playback generation.

This prevents persistence from applying an old event to a new track.

---

# 131. GENERATION EXAMPLE

Track A starts:

generation 41.

Track B starts:

generation 42.

A delayed Track A event with generation 41 must not overwrite Track B state.

---

# 132. ACCEPTANCE TEST: NORMAL RESUME

Play a track.

Pause at a known position.

Exit FLACHEAD.

Restart.

Verify:

the track and position are restored correctly.

---

# 133. ACCEPTANCE TEST: CRASH

Play a track long enough to create checkpoints.

Force an abnormal application termination.

Restart.

Verify:

the latest persisted checkpoint is used.

---

# 134. ACCEPTANCE TEST: SEEK

Seek backward.

Exit.

Restart.

Verify:

the backward seek position is restored rather than the previous later position.

---

# 135. ACCEPTANCE TEST: COMPLETION

Play a track to completion.

Restart it.

Verify:

it does not incorrectly resume at the end.

---

# 136. ACCEPTANCE TEST: SKIP

Start a track and skip it quickly.

Verify:

play count follows the configured minimum-play policy.

---

# 137. ACCEPTANCE TEST: HISTORY

Play several tracks.

Open Recently Played.

Verify:

tracks are ordered by actual playback events.

---

# 138. ACCEPTANCE TEST: REPEAT

Play a track repeatedly.

Verify:

history/play count behavior matches the configured policy.

---

# 139. ACCEPTANCE TEST: STORAGE REMOVAL

Remove/unmount the music storage.

Restart FLACHEAD.

Verify:

stored state remains safe.

---

# 140. ACCEPTANCE TEST: STORAGE RETURN

Restore storage.

Verify:

the previously stored track can be resolved when identity is available.

---

# 141. ACCEPTANCE TEST: MISSING TRACK

Delete the saved track.

Restart.

Verify:

FLACHEAD does not crash or attempt invalid playback.

---

# 142. ACCEPTANCE TEST: DATABASE FAILURE

Make persistence unavailable.

Verify:

audio playback continues.

---

# 143. ACCEPTANCE TEST: RAPID SEEK

Perform multiple rapid seeks.

Verify:

only the final valid position is persisted.

---

# 144. ACCEPTANCE TEST: LARGE HISTORY

Generate a large history dataset.

Verify:

memory usage remains bounded.

---

# 145. ACCEPTANCE TEST: POWER LOSS

Terminate power after a checkpoint interval.

Restart.

Verify:

the player resumes from the latest persisted checkpoint rather than requiring a database repair.

---

# 146. ACCEPTANCE CRITERIA

The persistence architecture is production-ready when:

- resume state survives normal shutdown
- periodic checkpoints protect against crashes
- storage writes are bounded
- audio playback never waits on persistence
- seek positions are persisted correctly
- completed tracks do not resume at the end
- missing tracks do not crash startup
- removable storage is handled safely
- history records actual playback
- play counts follow a deterministic threshold
- history and resume data are independently manageable
- database failures do not stop playback
- corrupted records are ignored safely
- stale playback events cannot overwrite newer state
- high sample-rate tracks remain supported
- lyrics and progress UI can restore from the same position
- all persistent behavior remains offline.

---

# 147. AI CODING AGENT RULES

The coding agent MUST:

1. Treat live playback state as authoritative.
2. Treat persistent state as a recoverable snapshot.
3. Store track identity using track_id.
4. Never use title alone as a resume identity.
5. Store playback position using a precise integer representation.
6. Prefer audio-frame-based timing internally.
7. Validate saved positions before restoring them.
8. Implement a meaningful resume threshold.
9. Implement an explicit completion threshold.
10. Reset resume state after completed playback.
11. Save periodically rather than on every position update.
12. Save immediately on important lifecycle events.
13. Coalesce rapid position updates.
14. Never write to the database from the audio output thread.
15. Never write to the database from the UI render loop.
16. Keep persistence failures from stopping audio.
17. Support degraded in-memory operation.
18. Keep resume state separate from history.
19. Keep history state separate from queue state.
20. Keep library identity separate from persistence identity handling.
21. Use transactions for logically related database updates.
22. Make startup recovery safe.
23. Make unexpected power loss survivable.
24. Reject stale playback events using track_id/generation.
25. Never automatically start playback unless the configured resume policy allows it.
26. Never require network access for resume/history.
27. Never modify FLAC files to store playback state.
28. Bound history size.
29. Keep history queries paginated/bounded.
30. Keep all persistence operations lightweight enough for the Pi Zero W.

---

# 148. FINAL PLAYBACK PERSISTENCE ARCHITECTURE

The intended runtime architecture is:

                         PLAYBACK ENGINE
                               │
             ┌─────────────────┼─────────────────┐
             │                 │                 │
             ▼                 ▼                 ▼
        POSITION           PLAYBACK           TRACK
        UPDATES             EVENTS            EVENTS
             │                 │                 │
             └─────────────────┼─────────────────┘
                               ▼
                    +----------------------+
                    | PERSISTENCE SERVICE  |
                    |                      |
                    | Resume State         |
                    | History              |
                    | Play Counts          |
                    | Last Played          |
                    +----------------------+
                               │
                               ▼
                         SQLite / DATA
                               │
             ┌─────────────────┼─────────────────┐
             ▼                 ▼                 ▼
        RESUME DATA       HISTORY DATA       STATS


Startup:

                    PERSISTENT DATA
                           │
                           ▼
                    RESUME SERVICE
                           │
                           ▼
                    TRACK RESOLUTION
                           │
                           ▼
                    PLAYBACK ENGINE
                           │
                           ▼
                  OPTIONAL AUTO-RESUME


The central invariant is:

LIVE PLAYBACK STATE IS AUTHORITATIVE; PERSISTED STATE IS ONLY A RECOVERY SNAPSHOT.

The second invariant is:

PERSISTENCE MUST NEVER BLOCK AUDIO PLAYBACK.

The third invariant is:

RESUME POSITION AND PLAY HISTORY ARE DIFFERENT TYPES OF STATE AND MUST BE STORED/UPDATED DIFFERENTLY.

The fourth invariant is:

COMPLETED TRACKS MUST NOT ACCIDENTALLY RESUME AT THEIR END.

The fifth invariant is:

A MISSING TRACK OR BROKEN DATABASE MUST NEVER CRASH FLACHEAD OR PREVENT NORMAL AUDIO OPERATION.

Final policy:

Checkpoint periodically.

Checkpoint on important playback events.

Coalesce writes.

Keep SD-card writes bounded.

Use stable track identity.

Keep history local and offline.

Keep resume separate from queue state.

Make persistence failure non-fatal.

Recover safely after crashes and power loss.

And always prioritize:

AUDIO PLAYBACK > UI RESPONSIVENESS > PERSISTENCE BACKGROUND WORK.