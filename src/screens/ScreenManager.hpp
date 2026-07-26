#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace flachead::screens
{
class Screen
{
public:
    virtual ~Screen() = default;
    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void OnUpdate(float) {}
};

class ScreenManager
{
public:
    ScreenManager() = default;
    ~ScreenManager() = default;

    void Register(std::string name, std::unique_ptr<Screen> screen);
    void Push(std::string_view name);
    void Pop();
    void Clear();
    Screen* Current() const;

private:
    std::unordered_map<std::string, std::unique_ptr<Screen>> m_Screens;
    std::vector<std::string> m_Stack;
};
} // namespace flachead::screens
