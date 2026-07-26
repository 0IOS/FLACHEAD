#pragma once

#include <string>
#include <string_view>

namespace flachead::app
{
class AppManager
{
public:
    AppManager();
    ~AppManager() = default;

    bool Initialize();
    void Shutdown();
    void Tick(float deltaSeconds);

    std::string_view ActiveApplication() const;
    void SetActiveApplication(std::string_view name);

private:
    std::string m_ActiveApplication;
    bool m_Initialized{false};
};
} // namespace flachead::app
