#include "Font.hpp"

namespace flachead::graphics
{
Font::Font(std::string_view path)
{
    Load(path);
}

bool Font::Load(std::string_view path)
{
    m_Path = path;
    m_Valid = !m_Path.empty();
    return m_Valid;
}

void Font::Release()
{
    m_Path.clear();
    m_Valid = false;
}

bool Font::Valid() const
{
    return m_Valid;
}
} // namespace flachead::graphics
