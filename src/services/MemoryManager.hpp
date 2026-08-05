#pragma once

#include <chrono>
#include <cstdint>
#include <cstddef>
#include <functional>

namespace flachead::services
{
// Monitors process RSS against optional soft/hard budgets. Current and peak
// RSS are sampled from /proc/self/status on Linux; sampling is throttled to
// once per second. ExceededSoftBudget/ExceededHardBudget latch until the
// reading drops back under the threshold. ThresholdCallback fires on every
// crossing (into or out of a budget).
class MemoryManager
{
public:
    using ThresholdCallback = std::function<void(bool overSoft, bool overHard)>;

    // Budgets in KiB. 0 disables the corresponding limit.
    void Initialize(std::size_t softBudgetKb, std::size_t hardBudgetKb);
    void Update();

    std::size_t CurrentRssKb() const { return m_CurrentRssKb; }
    std::size_t PeakRssKb() const { return m_PeakRssKb; }
    std::size_t SoftBudgetKb() const { return m_SoftBudgetKb; }
    std::size_t HardBudgetKb() const { return m_HardBudgetKb; }

    bool OverSoftBudget() const { return m_OverSoftBudget; }
    bool OverHardBudget() const { return m_OverHardBudget; }
    bool ExceededSoftBudget() const { return m_ExceededSoft; }
    bool ExceededHardBudget() const { return m_ExceededHard; }

    void SetThresholdCallback(ThresholdCallback callback) { m_ThresholdCallback = std::move(callback); }

    // Forces a sample on the next Update regardless of the throttle.
    void ForceSample() { m_NextSampleAt = std::chrono::steady_clock::time_point{}; }

private:
    void Sample();

    std::size_t m_CurrentRssKb{0};
    std::size_t m_PeakRssKb{0};
    std::size_t m_SoftBudgetKb{0};
    std::size_t m_HardBudgetKb{0};
    bool m_OverSoftBudget{false};
    bool m_OverHardBudget{false};
    bool m_ExceededSoft{false};
    bool m_ExceededHard{false};
    ThresholdCallback m_ThresholdCallback;
    std::chrono::steady_clock::time_point m_NextSampleAt{};
    bool m_Initialized{false};
};
} // namespace flachead::services
