#include "QueueManager.hpp"

#include <algorithm>
#include <numeric>
#include <utility>

namespace flachead::playback
{
using flachead::models::SongModel;

void QueueManager::SetTracks(std::vector<SongModel> tracks, int startIndex)
{
    m_Tracks = std::move(tracks);
    m_CurrentIndex = -1;
    m_ShufflePosition = -1;

    if (m_ShuffleEnabled)
    {
        RebuildShuffle();
        PlayAt(startIndex);
    }
    else
    {
        PlayAt(startIndex);
    }
}

void QueueManager::Clear()
{
    m_Tracks.clear();
    m_ShuffleOrder.clear();
    m_CurrentIndex = -1;
    m_ShufflePosition = -1;
}

SongModel QueueManager::CurrentTrack() const
{
    if (m_CurrentIndex < 0 || m_CurrentIndex >= static_cast<int>(m_Tracks.size()))
    {
        return {};
    }
    return m_Tracks[static_cast<std::size_t>(m_CurrentIndex)];
}

void QueueManager::PlayAt(int index)
{
    if (m_Tracks.empty())
    {
        m_CurrentIndex = -1;
        m_ShufflePosition = -1;
        return;
    }

    m_CurrentIndex = std::clamp(index, 0, static_cast<int>(m_Tracks.size()) - 1);

    if (m_ShuffleEnabled)
    {
        AlignShuffleToCurrent();
    }
}

void QueueManager::RebuildShuffle()
{
    m_ShuffleOrder.resize(m_Tracks.size());
    std::iota(m_ShuffleOrder.begin(), m_ShuffleOrder.end(), 0);
    std::shuffle(m_ShuffleOrder.begin(), m_ShuffleOrder.end(), m_Rng);
}

// Rotates the shuffled order so the current track is first. This keeps the
// invariant "shuffle position == 0" and guarantees every track is visited
// once when stepping forward through the queue.
void QueueManager::AlignShuffleToCurrent()
{
    if (m_CurrentIndex < 0 || m_ShuffleOrder.empty())
    {
        m_ShufflePosition = -1;
        return;
    }

    const auto it = std::find(m_ShuffleOrder.begin(), m_ShuffleOrder.end(), m_CurrentIndex);
    const auto pos = it != m_ShuffleOrder.end() ? it - m_ShuffleOrder.begin() : 0;
    std::rotate(m_ShuffleOrder.begin(), m_ShuffleOrder.begin() + pos, m_ShuffleOrder.end());
    m_ShufflePosition = 0;
}

void QueueManager::SetShuffle(bool enabled)
{
    if (m_ShuffleEnabled == enabled)
    {
        return;
    }

    m_ShuffleEnabled = enabled;
    if (!m_ShuffleEnabled)
    {
        m_ShufflePosition = -1;
        return;
    }

    RebuildShuffle();
    if (m_CurrentIndex >= 0)
    {
        AlignShuffleToCurrent();
    }
    else
    {
        m_ShufflePosition = -1;
    }
}

int QueueManager::PhysicalIndexOfShufflePosition() const
{
    if (m_ShufflePosition < 0 || m_ShufflePosition >= static_cast<int>(m_ShuffleOrder.size()))
    {
        return -1;
    }
    return m_ShuffleOrder[static_cast<std::size_t>(m_ShufflePosition)];
}

bool QueueManager::HasNext() const
{
    if (m_Tracks.empty())
    {
        return false;
    }
    if (m_Repeat == RepeatMode::One)
    {
        return true;
    }
    return Advance() >= 0;
}

bool QueueManager::HasPrevious() const
{
    if (m_Tracks.empty())
    {
        return false;
    }
    return Retreat() >= 0;
}

int QueueManager::Advance() const
{
    if (m_ShuffleEnabled && !m_ShuffleOrder.empty())
    {
        const int next = m_ShufflePosition + 1;
        if (next < static_cast<int>(m_ShuffleOrder.size()))
        {
            return m_ShuffleOrder[static_cast<std::size_t>(next)];
        }
        return m_Repeat == RepeatMode::All ? m_ShuffleOrder.front() : -1;
    }

    const int next = m_CurrentIndex + 1;
    if (next < static_cast<int>(m_Tracks.size()))
    {
        return next;
    }
    return m_Repeat == RepeatMode::All ? 0 : -1;
}

int QueueManager::Retreat() const
{
    if (m_ShuffleEnabled && !m_ShuffleOrder.empty())
    {
        const int prev = m_ShufflePosition - 1;
        if (prev >= 0)
        {
            return m_ShuffleOrder[static_cast<std::size_t>(prev)];
        }
        return m_Repeat == RepeatMode::All ? m_ShuffleOrder.back() : -1;
    }

    const int prev = m_CurrentIndex - 1;
    if (prev >= 0)
    {
        return prev;
    }
    return m_Repeat == RepeatMode::All ? static_cast<int>(m_Tracks.size()) - 1 : -1;
}

int QueueManager::PeekNext() const
{
    if (m_Tracks.empty())
    {
        return -1;
    }
    return Advance();
}

int QueueManager::PeekPrevious() const
{
    if (m_Tracks.empty())
    {
        return -1;
    }
    return Retreat();
}

int QueueManager::Next()
{
    if (m_Tracks.empty())
    {
        return -1;
    }
    if (m_Repeat == RepeatMode::One)
    {
        return m_CurrentIndex;
    }

    const int next = Advance();
    if (next < 0)
    {
        return -1;
    }
    PlayAt(next);
    return m_CurrentIndex;
}

int QueueManager::Previous()
{
    if (m_Tracks.empty())
    {
        return -1;
    }
    if (m_Repeat == RepeatMode::One)
    {
        return m_CurrentIndex;
    }

    const int prev = Retreat();
    if (prev < 0)
    {
        return -1;
    }
    PlayAt(prev);
    return m_CurrentIndex;
}

void QueueManager::RemoveAt(int index)
{
    if (index < 0 || index >= static_cast<int>(m_Tracks.size()))
    {
        return;
    }

    m_Tracks.erase(m_Tracks.begin() + index);

    if (m_ShuffleEnabled)
    {
        for (auto& entry : m_ShuffleOrder)
        {
            if (entry > index)
            {
                --entry;
            }
        }
        m_ShuffleOrder.erase(
            std::remove(m_ShuffleOrder.begin(), m_ShuffleOrder.end(), index),
            m_ShuffleOrder.end());
        if (m_CurrentIndex == index)
        {
            m_CurrentIndex = -1;
            m_ShufflePosition = -1;
        }
        else
        {
            if (m_CurrentIndex > index)
            {
                --m_CurrentIndex;
            }
            AlignShuffleToCurrent();
        }
        return;
    }

    if (m_CurrentIndex == index)
    {
        m_CurrentIndex = std::min(index, static_cast<int>(m_Tracks.size()) - 1);
    }
    else if (m_CurrentIndex > index)
    {
        --m_CurrentIndex;
    }
}

void QueueManager::InsertAfterCurrent(const SongModel& track)
{
    const int insertAt = m_CurrentIndex + 1;
    m_Tracks.insert(m_Tracks.begin() + insertAt, track);

    if (m_ShuffleEnabled)
    {
        for (auto& entry : m_ShuffleOrder)
        {
            if (entry > m_CurrentIndex)
            {
                ++entry;
            }
        }
        m_ShuffleOrder.insert(m_ShuffleOrder.begin() + m_ShufflePosition + 1, insertAt);
    }
}

void QueueManager::Move(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= static_cast<int>(m_Tracks.size()) ||
        toIndex < 0 || toIndex >= static_cast<int>(m_Tracks.size()) ||
        fromIndex == toIndex)
    {
        return;
    }

    SongModel track = m_Tracks[static_cast<std::size_t>(fromIndex)];
    m_Tracks.erase(m_Tracks.begin() + fromIndex);
    m_Tracks.insert(m_Tracks.begin() + toIndex, std::move(track));

    // Shuffle order is not maintained after a manual reorder; rebuild it.
    if (m_ShuffleEnabled)
    {
        RebuildShuffle();
        if (m_CurrentIndex >= 0)
        {
            AlignShuffleToCurrent();
        }
        else
        {
            m_ShufflePosition = -1;
        }
    }
    else if (fromIndex < toIndex)
    {
        if (m_CurrentIndex == fromIndex)
        {
            m_CurrentIndex = toIndex;
        }
        else if (m_CurrentIndex > fromIndex && m_CurrentIndex <= toIndex)
        {
            --m_CurrentIndex;
        }
    }
    else
    {
        if (m_CurrentIndex == fromIndex)
        {
            m_CurrentIndex = toIndex;
        }
        else if (m_CurrentIndex >= toIndex && m_CurrentIndex < fromIndex)
        {
            ++m_CurrentIndex;
        }
    }
}
} // namespace flachead::playback
