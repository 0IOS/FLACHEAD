#include "Font.hpp"

namespace flachead::graphics
{

Font::Font(std::string_view path, float size)
{
    Load(path, size);
}

Font::~Font()
{
    Release();
}

bool Font::Load(std::string_view path, float size)
{
    Release();

    m_Path = path;
    m_Size = size;

    m_Font = TTF_OpenFont(path.data(), size);

    return m_Font != nullptr;
}

void Font::Release()
{
    if(m_Font)
    {
        TTF_CloseFont(m_Font);
        m_Font = nullptr;
    }
}

bool Font::Valid() const
{
    return m_Font != nullptr;
}

TTF_Font* Font::Native() const
{
    return m_Font;
}

}