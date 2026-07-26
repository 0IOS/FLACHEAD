#pragma once

#include <string>
#include <string_view>

namespace flachead::ui
{
class Widget
{
public:
    virtual ~Widget() = default;

    virtual void SetId(std::string_view id);
    virtual std::string_view Id() const;

    virtual void SetEnabled(bool enabled);
    virtual bool Enabled() const;

protected:
    std::string m_Id;
    bool m_Enabled{true};
};
} // namespace flachead::ui
