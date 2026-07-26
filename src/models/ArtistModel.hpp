#pragma once

#include <string>
#include <string_view>

namespace flachead::models
{
class ArtistModel
{
public:
    ArtistModel() = default;
    ArtistModel(std::string_view name, std::string_view portraitPath);

    void SetName(std::string_view name);
    void SetPortraitPath(std::string_view portraitPath);

    std::string_view Name() const;
    std::string_view PortraitPath() const;

private:
    std::string m_Name;
    std::string m_PortraitPath;
};
} // namespace flachead::models
