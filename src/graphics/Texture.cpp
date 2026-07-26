#include "Texture.hpp"

namespace flachead::graphics
{
Texture::Texture(std::string_view path)
{
    Load(path);
}

bool Texture::Load(std::string_view path)
{
    m_Path = path;
    m_Valid = !m_Path.empty();
    return m_Valid;
}

void Texture::Release()
{
    m_Path.clear();
    m_Valid = false;
}

bool Texture::Valid() const
{
    return m_Valid;
}

std::string_view Texture::Path() const
{
    return m_Path;
}
} // namespace flachead::graphics
