#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace flachead::models
{
class PlaylistModel
{
public:
    PlaylistModel() = default;
    PlaylistModel(std::string_view name);

    void SetName(std::string_view name);
    void AddTrack(std::string_view path);

    std::string_view Name() const;
    const std::vector<std::string>& Tracks() const;

private:
    std::string m_Name;
    std::vector<std::string> m_Tracks;
};
} // namespace flachead::models
