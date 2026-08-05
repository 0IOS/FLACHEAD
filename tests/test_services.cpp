#include "../src/services/BackgroundJobManager.hpp"
#include "../src/services/MemoryManager.hpp"
#include "../src/services/NotificationManager.hpp"
#include "test_util.hpp"

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

using flachead::services::BackgroundJobManager;
using flachead::services::MemoryManager;
using flachead::services::NotificationManager;

namespace
{
constexpr auto kStep = std::chrono::milliseconds(2);

void WaitFor(std::atomic<bool>& flag)
{
    for (int i = 0; i < 2000 && !flag.load(); ++i)
    {
        std::this_thread::sleep_for(kStep);
    }
    Check(flag.load(), "flag raised before timeout");
}

void TestBackgroundJobBasic()
{
    BackgroundJobManager manager;
    manager.Start();

    std::atomic<bool> worked{false};
    std::atomic<bool> completed{false};
    const std::int64_t id = manager.Post("basic", [&] { worked.store(true); },
                                         [&] { completed.store(true); });
    Check(id > 0, "post returns a job id");
    manager.WaitForIdle();
    manager.Update();
    Check(worked.load(), "background work ran");
    Check(completed.load(), "completion ran on the calling thread");
    Check(manager.PendingCount() == 0, "queue drains");
    manager.Shutdown();
}

void TestBackgroundJobPriority()
{
    BackgroundJobManager manager;
    manager.Start();

    std::atomic<bool> release{false};
    std::atomic<bool> blockerStarted{false};
    std::vector<std::string> order;

    manager.Post("blocker", [&] {
                              blockerStarted.store(true);
                              while (!release.load())
                              {
                                  std::this_thread::sleep_for(kStep);
                              }
                          },
                 [&] { order.push_back("blocker"); }, 0);

    WaitFor(blockerStarted);

    manager.Post("low", [] {}, [&] { order.push_back("low"); }, 0);
    manager.Post("high", [] {}, [&] { order.push_back("high"); }, 10);

    release.store(true);
    manager.WaitForIdle();
    manager.Update();

    Check(order.size() == 3, "all three completions ran");
    if (order.size() == 3)
    {
        Check(order[0] == "blocker", "blocker finished first");
        Check(order[1] == "high", "high priority ran before low");
        Check(order[2] == "low", "low priority ran last");
    }
    manager.Shutdown();
}

void TestBackgroundJobCancel()
{
    BackgroundJobManager manager;
    manager.Start();

    std::atomic<bool> release{false};
    std::atomic<bool> blockerStarted{false};
    std::atomic<int> cancelledRuns{0};

    manager.Post("blocker", [&] {
                              blockerStarted.store(true);
                              while (!release.load())
                              {
                                  std::this_thread::sleep_for(kStep);
                              }
                          },
                 [] {}, 0);

    WaitFor(blockerStarted);

    manager.Post("cancelme", [&] { cancelledRuns.fetch_add(1); },
                 [&] { cancelledRuns.fetch_add(2); }, 0);
    manager.Cancel("cancelme");

    release.store(true);
    manager.WaitForIdle();
    manager.Update();

    Check(cancelledRuns.load() == 0, "cancelled job never ran");
    manager.Shutdown();
}

void TestBackgroundJobPostAfterShutdown()
{
    BackgroundJobManager manager;
    manager.Start();
    manager.Shutdown();
    const std::int64_t id = manager.Post("late", [] {}, [] {});
    Check(id == 0, "post after shutdown is rejected");
}

void TestNotificationDrain()
{
    NotificationManager manager;
    std::vector<std::string> shown;
    manager.SetToastHook([&](const flachead::services::Notification& n) { shown.push_back(n.title); });

    manager.Push("one", "body");
    manager.Push("two", "body", 1);
    manager.Push("three", "body");
    Check(manager.PendingCount() == 3, "three notifications pending");
    Check(manager.History().size() == 3, "history tracks all three");

    manager.Update();
    Check(shown.size() == 3, "hook fired for every pending notification");
    Check(shown[0] == "one" && shown[1] == "two" && shown[2] == "three", "drain preserves order");
    Check(manager.PendingCount() == 0, "pending queue emptied");
}

void TestNotificationDedupe()
{
    NotificationManager manager;
    std::vector<std::string> shown;
    manager.SetToastHook([&](const flachead::services::Notification& n) { shown.push_back(n.id); });

    manager.Push("first", "", 0, "scan.done");
    manager.Push("second", "", 0, "scan.done");
    Check(manager.PendingCount() == 1, "same id replaces the pending entry");

    manager.Update();
    Check(shown.size() == 1 && shown[0] == "scan.done", "only one toast for deduped id");
    Check(manager.History().size() == 2, "history keeps both versions");
}

void TestNotificationDismiss()
{
    NotificationManager manager;
    manager.Push("a", "", 0, "x");
    manager.Push("b", "", 0, "y");
    manager.Dismiss("x");
    Check(manager.PendingCount() == 1, "dismiss removes one pending");
    manager.DismissAll();
    Check(manager.PendingCount() == 0, "dismiss all empties pending");
}

void TestNotificationNoHook()
{
    NotificationManager manager;
    manager.Push("held", "");
    manager.Update();
    Check(manager.PendingCount() == 1, "without a hook, update keeps notifications pending");
}

void TestMemoryManagerBaseline()
{
    MemoryManager memory;
    memory.Initialize(0, 0);
    memory.Update();
    Check(!memory.OverSoftBudget(), "disabled soft budget never trips");
    Check(!memory.OverHardBudget(), "disabled hard budget never trips");
    Check(memory.CurrentRssKb() > 0, "current RSS sampled on linux");
    Check(memory.PeakRssKb() >= memory.CurrentRssKb(), "peak never below current");
}

void TestMemoryManagerBudgets()
{
    MemoryManager memory;
    int crossings = 0;
    memory.SetThresholdCallback([&](bool, bool) { ++crossings; });

    memory.Initialize(1, 1);
    memory.Update();
    Check(memory.OverSoftBudget(), "tiny soft budget trips");
    Check(memory.OverHardBudget(), "tiny hard budget trips");
    Check(crossings == 1, "callback fires once on the first crossing");

    memory.ForceSample();
    memory.Update();
    Check(crossings == 1, "no callback without a state change");

    memory.Initialize(0, 0);
    memory.ForceSample();
    memory.Update();
    Check(!memory.OverSoftBudget(), "disabling budgets clears the over flag");
    Check(crossings == 2, "callback fires when leaving the threshold");
}

void TestMemoryManagerThresholds()
{
    MemoryManager memory;
    bool sawSoft = false;
    bool sawHard = false;
    memory.SetThresholdCallback([&](bool overSoft, bool overHard) {
        sawSoft = overSoft;
        sawHard = overHard;
    });

    memory.Initialize(0, 1);
    memory.Update();
    Check(!sawSoft && sawHard, "hard-only budget reports soft=false, hard=true");
}
} // namespace

int main()
{
    RunTest("BackgroundJob run + complete", TestBackgroundJobBasic);
    RunTest("BackgroundJob priority ordering", TestBackgroundJobPriority);
    RunTest("BackgroundJob cancel pending", TestBackgroundJobCancel);
    RunTest("BackgroundJob post after shutdown", TestBackgroundJobPostAfterShutdown);
    RunTest("Notification drain + order", TestNotificationDrain);
    RunTest("Notification dedupe by id", TestNotificationDedupe);
    RunTest("Notification dismiss", TestNotificationDismiss);
    RunTest("Notification without hook", TestNotificationNoHook);
    RunTest("Memory baseline sample", TestMemoryManagerBaseline);
    RunTest("Memory budgets + callbacks", TestMemoryManagerBudgets);
    RunTest("Memory threshold reporting", TestMemoryManagerThresholds);
    return Finish();
}
