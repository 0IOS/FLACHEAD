#pragma once

#include "AudioBackend.hpp"

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace flachead::audio
{
// Production playback backend built on libmpv. libmpv is loaded at runtime
// via dlopen, so the app builds and runs without it (falling back to the
// NullBackend) and uses it transparently where libmpv.so.2 is installed.
//
// Gapless playback: QueueNext() appends the following track to mpv's own
// playlist; with --gapless-audio the engine blends the transition and the
// controller observes START_FILE for the new track.
class MpvBackend final : public AudioBackend
{
public:
    // Resolves the mpv client API from a shared library at runtime. All
    // handles are passed as void* so the header needs no <mpv/client.h>.
    struct MpvLoader
    {
        void* library{nullptr};

        unsigned long (*ClientApiVersion)(void){nullptr};
        const char* (*ErrorString)(int){nullptr};
        void (*Free)(void*){nullptr};
        void* (*Create)(void){nullptr};
        int (*Initialize)(void*){nullptr};
        void (*Destroy)(void*){nullptr};
        void (*TerminateDestroy)(void*){nullptr};
        void (*FreeNodeContents)(void*){nullptr};
        int (*Command)(void*, const char**){nullptr};
        int (*SetProperty)(void*, const char*, int, void*){nullptr};
        int (*SetPropertyString)(void*, const char*, const char*){nullptr};
        int (*GetProperty)(void*, const char*, int, void*){nullptr};
        char* (*GetPropertyString)(void*, const char*){nullptr};
        int (*RequestEvent)(void*, int, int){nullptr};
        int (*RequestLogMessages)(void*, const char*){nullptr};
        void* (*WaitEvent)(void*, double){nullptr};
        void (*Wakeup)(void*){nullptr};

        // Loads libmpv and resolves every symbol. Returns false on failure.
        bool Load();

        // Pre-allocated command arguments; kept out of hot paths.
        void Unload();
    };

    MpvBackend() = default;
    ~MpvBackend() override;

    bool Initialize() override;
    void Shutdown() override;

    bool OpenMedia(std::string_view uri) override;
    bool QueueNext(std::string_view uri) override;
    void ClearQueue() override;
    void Play() override;
    void Pause() override;
    void Stop() override;
    void Seek(double seconds) override;
    void SetVolume(float linearVolume) override;

    double GetPositionSeconds() const override;
    double GetDurationSeconds() const override;
    bool   IsPlaying() const override;
    bool   IsPaused() const override;

    bool PollEvents(BackendEvent& out) override;

    std::string_view Name() const override { return "libmpv"; }

    // True when the runtime libmpv shared object could be loaded.
    static bool Available();

private:
    bool SetPropertyString(const char* name, const char* value);
    bool GetDoubleProperty(const char* name, double& out) const;
    bool GetFlagProperty(const char* name) const;
    void RunEventLoop();
    void PushEvent(BackendEvent event);
    void ApplyConfiguration();

    std::unique_ptr<MpvLoader> m_Lib;
    void* m_Handle{nullptr};
    std::thread m_EventThread;
    std::atomic<bool> m_Running{false};

    mutable std::mutex m_EventMutex;
    std::deque<BackendEvent> m_Events;

    std::mutex m_StateMutex;
    std::string m_CurrentUri;
    int m_QueuedCount{0};

    std::atomic<float> m_Volume{1.0f};
};
} // namespace flachead::audio
