#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace flachead::graphics
{
class Texture
{
public:
    Texture() = default;
    explicit Texture(std::string_view path);

    bool Load(std::string_view path);
    void Release();

    bool Valid() const;
    std::string_view Path() const;

private:
    std::string m_Path;
    bool m_Valid{false};
};
} // namespace flachead::graphics
