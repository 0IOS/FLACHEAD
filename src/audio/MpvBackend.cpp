#include "MpvBackend.hpp"

#include "../core/Logger.hpp"

#include <mpv/client.h>

#include <dlfcn.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>

namespace flachead::audio
{
namespace
{
constexpr const char* kLibraryNames[] = {
    "libmpv.so.2",
    "libmpv.so",
    "libmpv.2.dylib",
};

constexpr double kEventTimeoutSeconds = 0.1;
} // namespace

// ---------------------------------------------------------------------------
// MpvLoader — resolves the mpv client API at runtime via dlopen.
// ---------------------------------------------------------------------------
bool MpvBackend::MpvLoader::Load()
{
    for (const char* name : kLibraryNames)
    {
        library = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
        if (library)
        {
            break;
        }
    }

    if (!library)
    {
        return false;
    }

#define LOAD_SYMBOL(member, symbol)                                 \
    member = reinterpret_cast<decltype(member)>(dlsym(library, #symbol)); \
    if (!member)                                                    \
    {                                                               \
        dlclose(library);                                           \
        library = nullptr;                                          \
        return false;                                               \
    }

    LOAD_SYMBOL(ClientApiVersion, mpv_client_api_version);
    LOAD_SYMBOL(ErrorString, mpv_error_string);
    LOAD_SYMBOL(Free, mpv_free);
    LOAD_SYMBOL(Create, mpv_create);
    LOAD_SYMBOL(Initialize, mpv_initialize);
    LOAD_SYMBOL(Destroy, mpv_destroy);
    LOAD_SYMBOL(TerminateDestroy, mpv_terminate_destroy);
    LOAD_SYMBOL(FreeNodeContents, mpv_free_node_contents);
    LOAD_SYMBOL(Command, mpv_command);
    LOAD_SYMBOL(SetProperty, mpv_set_property);
    LOAD_SYMBOL(SetPropertyString, mpv_set_property_string);
    LOAD_SYMBOL(GetProperty, mpv_get_property);
    LOAD_SYMBOL(GetPropertyString, mpv_get_property_string);
    LOAD_SYMBOL(RequestEvent, mpv_request_event);
    LOAD_SYMBOL(RequestLogMessages, mpv_request_log_messages);
    LOAD_SYMBOL(WaitEvent, mpv_wait_event);
    LOAD_SYMBOL(Wakeup, mpv_wakeup);

#undef LOAD_SYMBOL
    return true;
}

void MpvBackend::MpvLoader::Unload()
{
    if (library)
    {
        dlclose(library);
        library = nullptr;
    }
}

bool MpvBackend::Available()
{
    MpvLoader loader;
    return loader.Load();
}

MpvBackend::~MpvBackend()
{
    Shutdown();
}

bool MpvBackend::Initialize()
{
    if (m_Handle)
    {
        return true;
    }

    m_Lib = std::make_unique<MpvLoader>();
    if (!m_Lib->Load())
    {
        flachead::core::Logger::Warning("libmpv not available, using fallback audio backend");
        m_Lib.reset();
        return false;
    }

    m_Handle = m_Lib->Create();
    if (!m_Handle)
    {
        flachead::core::Logger::Error("mpv_create failed");
        m_Lib->Unload();
        m_Lib.reset();
        return false;
    }

    ApplyConfiguration();

    if (m_Lib->Initialize(m_Handle) < 0)
    {
        flachead::core::Logger::Error("mpv_initialize failed");
        m_Lib->Destroy(m_Handle);
        m_Handle = nullptr;
        m_Lib->Unload();
        m_Lib.reset();
        return false;
    }

    // Start paused so OpenMedia() never auto-plays.
    SetPropertyString("pause", "yes");
    SetPropertyString("volume", "100");

    m_Lib->RequestEvent(m_Handle, MPV_EVENT_START_FILE, 1);
    m_Lib->RequestEvent(m_Handle, MPV_EVENT_FILE_LOADED, 1);
    m_Lib->RequestEvent(m_Handle, MPV_EVENT_END_FILE, 1);
    m_Lib->RequestEvent(m_Handle, MPV_EVENT_IDLE, 1);

    m_Running.store(true);
    m_EventThread = std::thread(&MpvBackend::RunEventLoop, this);

    flachead::core::Logger::Info("Audio backend: libmpv");
    return true;
}

void MpvBackend::ApplyConfiguration()
{
    const char* options[][2] = {
        {"force-window", "no"},
        {"vid", "no"},
        {"keep-open", "no"},
        {"gapless-audio", "yes"},
        {"audio-display", "no"},
        {"terminal", "no"},
        {"input-default-bindings", "no"},
        {"input-vo-keyboard", "no"},
        {"cache", "yes"},
        {"demuxer-readahead-secs", "10"},
    };

    for (const auto& option : options)
    {
        // Set via property API before mpv_initialize where possible; failures
        // are non-fatal.
        m_Lib->SetPropertyString(m_Handle, option[0], option[1]);
    }
}

void MpvBackend::Shutdown()
{
    if (!m_Handle)
    {
        return;
    }

    m_Running.store(false);
    if (m_Lib && m_Handle)
    {
        m_Lib->Wakeup(m_Handle);
    }
    if (m_EventThread.joinable())
    {
        m_EventThread.join();
    }

    m_Lib->TerminateDestroy(m_Handle);
    m_Handle = nullptr;
    m_Lib->Unload();
    m_Lib.reset();

    std::lock_guard<std::mutex> lock(m_EventMutex);
    m_Events.clear();
}

bool MpvBackend::SetPropertyString(const char* name, const char* value)
{
    if (!m_Handle)
    {
        return false;
    }
    return m_Lib->SetPropertyString(m_Handle, name, value) >= 0;
}

bool MpvBackend::GetDoubleProperty(const char* name, double& out) const
{
    if (!m_Handle)
    {
        return false;
    }
    double value = 0.0;
    const int rc = m_Lib->GetProperty(m_Handle, name, MPV_FORMAT_DOUBLE, &value);
    if (rc >= 0)
    {
        out = value;
        return true;
    }
    return false;
}

bool MpvBackend::GetFlagProperty(const char* name) const
{
    if (!m_Handle)
    {
        return false;
    }
    int value = 0;
    const int rc = m_Lib->GetProperty(m_Handle, name, MPV_FORMAT_FLAG, &value);
    return rc >= 0 && value != 0;
}

bool MpvBackend::OpenMedia(std::string_view uri)
{
    if (!m_Handle)
    {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_StateMutex);
        m_CurrentUri = std::string{uri};
        m_QueuedCount = 0;
    }

    const char* command[] = {"loadfile", std::string{uri}.c_str(), "replace", nullptr};
    const int rc = m_Lib->Command(m_Handle, command);
    return rc >= 0;
}

bool MpvBackend::QueueNext(std::string_view uri)
{
    if (!m_Handle)
    {
        return false;
    }

    const char* command[] = {"loadfile", std::string{uri}.c_str(), "append-play", nullptr};
    const int rc = m_Lib->Command(m_Handle, command);
    if (rc >= 0)
    {
        std::lock_guard<std::mutex> lock(m_StateMutex);
        ++m_QueuedCount;
    }
    return rc >= 0;
}

void MpvBackend::ClearQueue()
{
    if (!m_Handle)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_StateMutex);
    while (m_QueuedCount > 0)
    {
        // Remove the last entry; queued tracks are always at the end.
        int64_t count = 0;
        if (m_Lib->GetProperty(m_Handle, "playlist-count", MPV_FORMAT_INT64, &count) < 0 || count <= 0)
        {
            break;
        }
        const std::string indexStr = std::to_string(count - 1);
        const char* command[] = {"playlist-remove", indexStr.c_str(), nullptr};
        m_Lib->Command(m_Handle, command);
        --m_QueuedCount;
    }
}

void MpvBackend::Play()
{
    if (m_Handle)
    {
        SetPropertyString("pause", "no");
    }
}

void MpvBackend::Pause()
{
    if (m_Handle)
    {
        SetPropertyString("pause", "yes");
    }
}

void MpvBackend::Stop()
{
    if (!m_Handle)
    {
        return;
    }
    const char* command[] = {"stop", nullptr};
    m_Lib->Command(m_Handle, command);
}

void MpvBackend::Seek(double seconds)
{
    if (!m_Handle)
    {
        return;
    }
    const std::string position = std::to_string(seconds);
    const char* command[] = {"seek", position.c_str(), "absolute", nullptr};
    m_Lib->Command(m_Handle, command);
}

void MpvBackend::SetVolume(float linearVolume)
{
    m_Volume.store(linearVolume);
    if (!m_Handle)
    {
        return;
    }
    const int percent = std::clamp(static_cast<int>(std::lround(linearVolume * 100.0f)), 0, 100);
    SetPropertyString("volume", std::to_string(percent).c_str());
    if (percent == 0)
    {
        SetPropertyString("mute", "yes");
    }
    else
    {
        SetPropertyString("mute", "no");
    }
}

double MpvBackend::GetPositionSeconds() const
{
    if (!m_Handle)
    {
        return 0.0;
    }
    double value = 0.0;
    return GetDoubleProperty("time-pos", value) ? value : 0.0;
}

double MpvBackend::GetDurationSeconds() const
{
    if (!m_Handle)
    {
        return 0.0;
    }
    double value = 0.0;
    return GetDoubleProperty("duration", value) ? value : 0.0;
}

bool MpvBackend::IsPlaying() const
{
    return m_Handle && !GetFlagProperty("pause");
}

bool MpvBackend::IsPaused() const
{
    return m_Handle && GetFlagProperty("pause");
}

void MpvBackend::PushEvent(BackendEvent event)
{
    std::lock_guard<std::mutex> lock(m_EventMutex);
    m_Events.push_back(std::move(event));
}

bool MpvBackend::PollEvents(BackendEvent& out)
{
    std::lock_guard<std::mutex> lock(m_EventMutex);
    if (m_Events.empty())
    {
        return false;
    }
    out = std::move(m_Events.front());
    m_Events.pop_front();
    return true;
}

void MpvBackend::RunEventLoop()
{
    while (m_Running.load() && m_Handle)
    {
        mpv_event* event = static_cast<mpv_event*>(m_Lib->WaitEvent(m_Handle, kEventTimeoutSeconds));
        if (event == nullptr || event->event_id == MPV_EVENT_NONE)
        {
            continue;
        }
        if (event->event_id == MPV_EVENT_SHUTDOWN)
        {
            break;
        }

        switch (event->event_id)
        {
            case MPV_EVENT_START_FILE:
            {
                std::lock_guard<std::mutex> lock(m_StateMutex);
                PushEvent(BackendEvent{BackendEvent::Kind::StartOfFile, m_CurrentUri, 0, {}});
                break;
            }
            case MPV_EVENT_FILE_LOADED:
            {
                std::lock_guard<std::mutex> lock(m_StateMutex);
                PushEvent(BackendEvent{BackendEvent::Kind::FileLoaded, m_CurrentUri, 0, {}});
                break;
            }
            case MPV_EVENT_END_FILE:
            {
                const mpv_event_end_file* endFile =
                    static_cast<const mpv_event_end_file*>(event->data);
                const int reason = endFile ? endFile->reason : 0;
                const std::string error =
                    endFile && endFile->error ? m_Lib->ErrorString(endFile->error) : "";
                std::string uri;
                {
                    std::lock_guard<std::mutex> lock(m_StateMutex);
                    uri = m_CurrentUri;
                }
                PushEvent(BackendEvent{BackendEvent::Kind::EndOfFile, uri, reason, error});
                break;
            }
            case MPV_EVENT_IDLE:
            {
                PushEvent(BackendEvent{BackendEvent::Kind::Idle, {}, 0, {}});
                break;
            }
            default:
                break;
        }
    }
}
} // namespace flachead::audio
