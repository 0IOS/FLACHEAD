#pragma once

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace flachead::services
{
// A named unit of background work. `work` runs on the manager's worker
// thread; `complete` is re-queued and invoked by Update() on the main
// thread so callbacks can touch UI services safely.
struct BackgroundJob
{
    std::string name;
    std::function<void()> work;
    std::function<void()> complete;
    int priority{0};
    std::int64_t id{0};
    bool cancelled{false};
};

// Runs posted jobs on a single worker thread, highest priority first, and
// hands finished jobs back to the main thread through Update(). Cancel only
// removes jobs that have not started; running jobs run to completion.
class BackgroundJobManager
{
public:
    BackgroundJobManager() = default;
    ~BackgroundJobManager();

    BackgroundJobManager(const BackgroundJobManager&) = delete;
    BackgroundJobManager& operator=(const BackgroundJobManager&) = delete;

    void Start();
    void Shutdown();

    // Posts a job. Returns the job id, or 0 if the manager is stopped.
    // `complete` is invoked from Update() once `work` finishes.
    std::int64_t Post(std::string name, std::function<void()> work,
                      std::function<void()> complete = {}, int priority = 0);

    void Cancel(std::string_view name);
    void CancelAll();

    // Runs completed jobs' callbacks on the calling (main) thread.
    void Update();

    std::size_t PendingCount() const;
    std::size_t ActiveCount() const;
    std::size_t CompletedCount() const;

    // Blocks until the pending queue drains. Mainly for tests.
    void WaitForIdle();

private:
    void WorkerLoop();

    mutable std::mutex m_Mutex;
    std::condition_variable m_Cv;
    std::thread m_Worker;
    std::vector<BackgroundJob> m_Pending;
    std::vector<BackgroundJob> m_Completed;
    std::int64_t m_NextId{1};
    bool m_Active{false};
    bool m_Running{false};
};
} // namespace flachead::services
