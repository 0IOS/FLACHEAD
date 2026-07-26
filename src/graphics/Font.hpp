#pragma once

#include <string>
#include <string_view>

namespace flachead::graphics
{
class Font
{
public:
    Font() = default;
    explicit Font(std::string_view path);

    bool Load(std::string_view path);
    void Release();

    bool Valid() const;

private:
    std::string m_Path;
    bool m_Valid{false};
};
} // namespace flachead::graphics
