#pragma once

#include "../graphics/Font.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"
#include <SDL3/SDL.h>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace flachead::core
{
class Renderer
{
public:
    Renderer();
    ~Renderer();

    bool Create(SDL_Window* window);
    void Destroy();

    void BeginFrame();
    void EndFrame();

    void SetColor(const Color& color);

    void DrawRect(const Rect& rect);
    void FillRect(const Rect& rect);
    void FillRoundedRect(const Rect& rect, float radius);
    void DrawRoundedRect(const Rect& rect, float radius);
    void DrawLine(float x1, float y1, float x2, float y2);
    void FillCircle(float cx, float cy, float radius);
    void DrawCircle(float cx, float cy, float radius);
    void DrawText(const Rect& rect, std::string_view text, const flachead::graphics::Font& font, const Color& color);

    SDL_Renderer* GetSDLRenderer() const { return m_Renderer; }

private:
    struct TextGlyph
    {
        SDL_Texture* texture{nullptr};
        int width{0};
        int height{0};
    };

    static uint64_t TextHash(std::string_view text, const flachead::graphics::Font& font, const Color& color);

    SDL_Texture* CornerTexture(int radius, bool outline);
    SDL_Texture* DiscTexture(int radius, bool outline);
    void PrepareShape(SDL_Texture* texture);
    void BlitShape(SDL_Texture* texture, float x, float y, float w, float h);

    SDL_Renderer* m_Renderer{nullptr};
    Color m_CurrentColor{255, 255, 255, 255};

    std::unordered_map<uint64_t, TextGlyph> m_TextCache;
    std::vector<uint64_t> m_TextCacheOrder;
    std::unordered_map<int, SDL_Texture*> m_Corners;
    std::unordered_map<int, SDL_Texture*> m_CornerOutlines;
    std::unordered_map<int, SDL_Texture*> m_Discs;
    std::unordered_map<int, SDL_Texture*> m_Rings;
};
} // namespace flachead::core
