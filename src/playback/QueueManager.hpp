#pragma once

#include "../models/SongModel.hpp"

#include <cstdint>
#include <random>
#include <vector>

namespace flachead::playback
{
enum class RepeatMode
{
    Off = 0,
    All,
    One,
};

// Owns the ordered list of tracks available to the player plus the current
// position, repeat and shuffle state. Pure logic: no I/O, no events, easily
// unit tested. Shuffle uses a Fisher-Yates order over indices; repeat applies
// at Next()/Prev() boundaries.
class QueueManager
{
public:
    void SetTracks(std::vector<flachead::models::SongModel> tracks, int startIndex = 0);
    void Clear();

    const std::vector<flachead::models::SongModel>& Tracks() const { return m_Tracks; }
    int Size() const { return static_cast<int>(m_Tracks.size()); }
    bool IsEmpty() const { return m_Tracks.empty(); }

    // Index of the currently playing track, or -1 when nothing is loaded.
    int CurrentIndex() const { return m_CurrentIndex; }

    // The track at the current index (empty when nothing loaded).
    flachead::models::SongModel CurrentTrack() const;

    // True when advancing past the current track is possible with the active
    // repeat mode.
    bool HasNext() const;
    bool HasPrevious() const;

    // Physical index of the next track without moving the playhead, or -1.
    int PeekNext() const;
    int PeekPrevious() const;

    // Advances or rewinds, honouring repeat and shuffle. Returns the new index
    // or -1 when there is nothing to play next.
    int Next();
    int Previous();

    // Moves the playhead to a specific track.
    void PlayAt(int index);

    // Queue editing (used by the queue screen / play next).
    void RemoveAt(int index);
    void InsertAfterCurrent(const flachead::models::SongModel& track);
    void Move(int fromIndex, int toIndex);

    // Repeat / shuffle controls. Toggling shuffle reshuffles from the current
    // track.
    void SetRepeat(RepeatMode mode) { m_Repeat = mode; }
    RepeatMode Repeat() const { return m_Repeat; }

    void SetShuffle(bool enabled);
    bool Shuffle() const { return m_ShuffleEnabled; }

    // The deterministic shuffled index order. Rebuilt by SetShuffle(true) and
    // SetTracks().
    const std::vector<int>& ShuffleOrder() const { return m_ShuffleOrder; }

private:
    int PhysicalIndexOfShufflePosition() const;
    void RebuildShuffle();
    void AlignShuffleToCurrent();
    int Advance() const;
    int Retreat() const;

    std::vector<flachead::models::SongModel> m_Tracks;
    std::vector<int> m_ShuffleOrder;
    int m_CurrentIndex{-1};
    int m_ShufflePosition{-1};
    RepeatMode m_Repeat{RepeatMode::Off};
    bool m_ShuffleEnabled{false};
    std::mt19937 m_Rng{std::random_device{}()};
};
} // namespace flachead::playback
