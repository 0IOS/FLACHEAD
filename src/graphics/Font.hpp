#pragma once

#include <SDL3_ttf/SDL_ttf.h>

#include <string>
#include <string_view>

namespace flachead::graphics
{

class Font
{
public:
    Font() = default;
    Font(std::string_view path, float size);
    ~Font();

    bool Load(std::string_view path, float size);

    void Release();

    bool Valid() const;

    TTF_Font* Native() const;

private:
    std::string m_Path;

    float m_Size{18.f};

    TTF_Font* m_Font{nullptr};
};

}