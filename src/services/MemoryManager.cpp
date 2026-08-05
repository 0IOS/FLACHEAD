#include "MemoryManager.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

namespace flachead::services
{
namespace
{
// Parses a "Field: value kB" line out of /proc/self/status.
std::size_t ReadProcField(const char* field)
{
#if defined(__linux__)
    FILE* status = std::fopen("/proc/self/status", "r");
    if (!status)
    {
        return 0;
    }

    std::size_t value = 0;
    char line[256];
    const std::size_t fieldLength = std::strlen(field);
    while (std::fgets(line, sizeof(line), status))
    {
        if (std::strncmp(line, field, fieldLength) != 0)
        {
            continue;
        }
        if (std::sscanf(line + fieldLength, ": %zu kB", &value) == 1)
        {
            break;
        }
    }
    std::fclose(status);
    return value;
#else
    (void)field;
    return 0;
#endif
}
} // namespace

void MemoryManager::Initialize(std::size_t softBudgetKb, std::size_t hardBudgetKb)
{
    m_SoftBudgetKb = softBudgetKb;
    m_HardBudgetKb = hardBudgetKb;
    m_Initialized = true;
    m_NextSampleAt = std::chrono::steady_clock::time_point{};
}

void MemoryManager::Update()
{
    const auto now = std::chrono::steady_clock::now();
    if (!m_Initialized || now < m_NextSampleAt)
    {
        return;
    }
    m_NextSampleAt = now + std::chrono::seconds(1);
    Sample();

    const bool overSoft = m_SoftBudgetKb > 0 && m_CurrentRssKb > m_SoftBudgetKb;
    const bool overHard = m_HardBudgetKb > 0 && m_CurrentRssKb > m_HardBudgetKb;

    m_ExceededSoft = overSoft;
    m_ExceededHard = overHard;

    if (m_ThresholdCallback && (overSoft != m_OverSoftBudget || overHard != m_OverHardBudget))
    {
        m_ThresholdCallback(overSoft, overHard);
    }
    m_OverSoftBudget = overSoft;
    m_OverHardBudget = overHard;
}

void MemoryManager::Sample()
{
#if defined(__linux__)
    const std::size_t current = ReadProcField("VmRSS");
    const std::size_t peak = ReadProcField("VmHWM");
#else
    const std::size_t current = 0;
    const std::size_t peak = 0;
#endif
    m_CurrentRssKb = current;
    if (peak > m_PeakRssKb)
    {
        m_PeakRssKb = peak;
    }
    if (current > m_PeakRssKb)
    {
        m_PeakRssKb = current;
    }
}
} // namespace flachead::services
