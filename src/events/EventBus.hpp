#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace flachead::events
{
enum class Type
{
    None = 0,
    PlaybackStarted,
    PlaybackPaused,
    PlaybackResumed,
    PlaybackStopped,
    PlaybackFinished,
    TrackChanged,
    TrackLoaded,
    TrackProgress,
    PlaybackError,
    VolumeChanged,
    QueueChanged,
    ShuffleChanged,
    RepeatChanged,
    LibraryScanStarted,
    LibraryScanProgress,
    LibraryScanFinished,
    LibraryUpdated,
    PlaylistChanged,
    SettingsChanged,
    ThemeChanged,
    DisplaySleep,
    DisplayWake,
    ShutdownRequested,
};

// Generic event payload. Every subsystem communicates through EventBus so the
// UI never talks to playback/library internals directly.
struct Event
{
    Type type{Type::None};

    int64_t intValue{0};
    double  doubleValue{0.0};
    std::string stringValue;
};

class EventBus
{
public:
    using Handler = std::function<void(const Event&)>;

    EventBus() = default;
    ~EventBus() = default;

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    // Returns a subscription id used to unsubscribe. Thread-safe.
    int Subscribe(Type type, Handler handler);
    void Unsubscribe(int subscriptionId);

    // Dispatches synchronously to all subscribers on the calling thread.
    void Publish(const Event& event);

    // Convenience helpers.
    void Publish(Type type);
    void Publish(Type type, int64_t intValue);

private:
    struct Subscription
    {
        int id;
        Handler handler;
    };

    std::mutex m_Mutex;
    int m_NextId{1};
    std::unordered_map<Type, std::vector<Subscription>> m_Subscribers;
};
} // namespace flachead::events
