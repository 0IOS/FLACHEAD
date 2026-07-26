#include "ArtistModel.hpp"

namespace flachead::models
{
ArtistModel::ArtistModel(std::string_view name, std::string_view portraitPath)
    : m_Name(name),
      m_PortraitPath(portraitPath)
{
}

void ArtistModel::SetName(std::string_view name)
{
    m_Name = name;
}

void ArtistModel::SetPortraitPath(std::string_view portraitPath)
{
    m_PortraitPath = portraitPath;
}

std::string_view ArtistModel::Name() const
{
    return m_Name;
}

std::string_view ArtistModel::PortraitPath() const
{
    return m_PortraitPath;
}
} // namespace flachead::models
