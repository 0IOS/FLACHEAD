#include "BackgroundJobManager.hpp"

#include <algorithm>
#include <utility>

namespace flachead::services
{
BackgroundJobManager::~BackgroundJobManager()
{
    Shutdown();
}

void BackgroundJobManager::Start()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_Running)
    {
        return;
    }
    m_Running = true;
    m_Worker = std::thread([this] { WorkerLoop(); });
}

void BackgroundJobManager::Shutdown()
{
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (!m_Running)
        {
            return;
        }
        m_Running = false;
        m_Pending.clear();
        m_Cv.notify_all();
    }
    if (m_Worker.joinable())
    {
        m_Worker.join();
    }
}

std::int64_t BackgroundJobManager::Post(std::string name, std::function<void()> work,
                                        std::function<void()> complete, int priority)
{
    if (!work)
    {
        return 0;
    }

    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Running)
    {
        return 0;
    }

    BackgroundJob job;
    job.name = std::move(name);
    job.work = std::move(work);
    job.complete = std::move(complete);
    job.priority = priority;
    job.id = m_NextId++;
    m_Pending.push_back(std::move(job));
    m_Cv.notify_all();
    return job.id;
}

void BackgroundJobManager::Cancel(std::string_view name)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Pending.erase(std::remove_if(m_Pending.begin(), m_Pending.end(),
                                   [name](const BackgroundJob& job) {
                                       return job.name == name;
                                   }),
                    m_Pending.end());
}

void BackgroundJobManager::CancelAll()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Pending.clear();
}

void BackgroundJobManager::Update()
{
    std::vector<BackgroundJob> finished;
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        finished.swap(m_Completed);
    }

    for (auto& job : finished)
    {
        if (!job.cancelled && job.complete)
        {
            job.complete();
        }
    }
}

std::size_t BackgroundJobManager::PendingCount() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Pending.size();
}

std::size_t BackgroundJobManager::ActiveCount() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Active ? 1 : 0;
}

std::size_t BackgroundJobManager::CompletedCount() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Completed.size();
}

void BackgroundJobManager::WaitForIdle()
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Cv.wait(lock, [this] { return m_Pending.empty() && !m_Active; });
}

void BackgroundJobManager::WorkerLoop()
{
    for (;;)
    {
        BackgroundJob job;
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Cv.wait(lock, [this] { return !m_Running || !m_Pending.empty(); });
            if (!m_Running)
            {
                return;
            }

            auto best = m_Pending.begin();
            for (auto it = m_Pending.begin(); it != m_Pending.end(); ++it)
            {
                if (it->priority > best->priority)
                {
                    best = it;
                }
            }
            job = std::move(*best);
            m_Pending.erase(best);
            m_Active = true;
        }

        job.work();

        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Active = false;
            m_Completed.push_back(std::move(job));
            m_Cv.notify_all();
        }
    }
}
} // namespace flachead::services
