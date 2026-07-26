#include "Renderer.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace flachead::core
{
Renderer::Renderer()
    : m_Renderer(nullptr)
{
}

Renderer::~Renderer()
{
    Destroy();
}

bool Renderer::Create(SDL_Window* window)
{
    m_Renderer = SDL_CreateRenderer(window, nullptr);
    if (m_Renderer)
    {
        SDL_SetRenderDrawBlendMode(m_Renderer, SDL_BLENDMODE_BLEND);
    }
    return m_Renderer != nullptr;
}

void Renderer::Destroy()
{
    if (m_Renderer)
    {
        SDL_DestroyRenderer(m_Renderer);
        m_Renderer = nullptr;
    }
}

void Renderer::BeginFrame()
{
    SDL_SetRenderDrawColor(m_Renderer, 5, 7, 12, 255);
    SDL_RenderClear(m_Renderer);
}

void Renderer::EndFrame()
{
    SDL_RenderPresent(m_Renderer);
}

void Renderer::SetColor(const Color& color)
{
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
}

void Renderer::DrawRect(const Rect& rect)
{
    SDL_FRect sdlRect{rect.position.x, rect.position.y, rect.size.x, rect.size.y};
    SDL_RenderRect(m_Renderer, &sdlRect);
}

void Renderer::FillRect(const Rect& rect)
{
    SDL_FRect sdlRect{rect.position.x, rect.position.y, rect.size.x, rect.size.y};
    SDL_RenderFillRect(m_Renderer, &sdlRect);
}

void Renderer::DrawLine(float x1, float y1, float x2, float y2)
{
    SDL_RenderLine(m_Renderer, x1, y1, x2, y2);
}

void Renderer::FillRoundedRect(const Rect& rect, float radius)
{
    if (!m_Renderer)
        return;

    const float x = rect.position.x;
    const float y = rect.position.y;
    const float w = rect.size.x;
    const float h = rect.size.y;
    const float r = std::min(radius, std::min(w, h) * 0.5f);

    // Fill center column
    SDL_FRect center{x, y + r, w, h - 2.0f * r};
    SDL_RenderFillRect(m_Renderer, &center);

    // Fill top row (without corners)
    SDL_FRect top{x + r, y, w - 2.0f * r, r};
    SDL_RenderFillRect(m_Renderer, &top);

    // Fill bottom row (without corners)
    SDL_FRect bottom{x + r, y + h - r, w - 2.0f * r, r};
    SDL_RenderFillRect(m_Renderer, &bottom);

    // Fill corners using circles
    const int ir = static_cast<int>(r);
    const int cx0 = static_cast<int>(x + r);
    const int cy0 = static_cast<int>(y + r);
    const int cx1 = static_cast<int>(x + w - r);
    const int cy1 = static_cast<int>(y + h - r);

    for (int dy = 0; dy <= ir; ++dy)
    {
        const int dx = static_cast<int>(std::sqrt(static_cast<float>(ir * ir - dy * dy)));
        SDL_FRect row0{static_cast<float>(cx0 - dx), static_cast<float>(cy0 - dy),
                       static_cast<float>(2 * dx + 1), 1.0f};
        SDL_RenderFillRect(m_Renderer, &row0);

        SDL_FRect row1{static_cast<float>(cx0 - dx), static_cast<float>(cy0 - dy),
                       static_cast<float>(dx + 1), 1.0f};
        (void)row1;

        // Top-left corner
        SDL_FRect tl{static_cast<float>(cx0 - dx), static_cast<float>(cy0 - dy),
                     static_cast<float>(dx + 1), 1.0f};
        SDL_RenderFillRect(m_Renderer, &tl);

        // Top-right corner
        SDL_FRect tr{static_cast<float>(cx1), static_cast<float>(cy0 - dy),
                     static_cast<float>(dx + 1), 1.0f};
        SDL_RenderFillRect(m_Renderer, &tr);

        // Bottom-left corner
        SDL_FRect bl{static_cast<float>(cx0 - dx), static_cast<float>(cy1 + dy),
                     static_cast<float>(dx + 1), 1.0f};
        SDL_RenderFillRect(m_Renderer, &bl);

        // Bottom-right corner
        SDL_FRect br{static_cast<float>(cx1), static_cast<float>(cy1 + dy),
                     static_cast<float>(dx + 1), 1.0f};
        SDL_RenderFillRect(m_Renderer, &br);
    }
}

void Renderer::DrawRoundedRect(const Rect& rect, float radius)
{
    if (!m_Renderer)
        return;

    const float x = rect.position.x;
    const float y = rect.position.y;
    const float w = rect.size.x;
    const float h = rect.size.y;
    const float r = std::min(radius, std::min(w, h) * 0.5f);

    // Four edges
    SDL_RenderLine(m_Renderer, x + r, y, x + w - r, y);
    SDL_RenderLine(m_Renderer, x + r, y + h, x + w - r, y + h);
    SDL_RenderLine(m_Renderer, x, y + r, x, y + h - r);
    SDL_RenderLine(m_Renderer, x + w, y + r, x + w, y + h - r);

    // Corner arcs (approximate with lines)
    const int steps = std::max(6, static_cast<int>(r * 0.5f));
    for (int i = 0; i < steps; ++i)
    {
        const float a0 = static_cast<float>(i) / static_cast<float>(steps) * (3.14159f / 2.0f);
        const float a1 = static_cast<float>(i + 1) / static_cast<float>(steps) * (3.14159f / 2.0f);

        // Top-left
        SDL_RenderLine(m_Renderer,
                       x + r - std::cos(a0) * r, y + r - std::sin(a0) * r,
                       x + r - std::cos(a1) * r, y + r - std::sin(a1) * r);
        // Top-right
        SDL_RenderLine(m_Renderer,
                       x + w - r + std::sin(a0) * r, y + r - std::cos(a0) * r,
                       x + w - r + std::sin(a1) * r, y + r - std::cos(a1) * r);
        // Bottom-right
        SDL_RenderLine(m_Renderer,
                       x + w - r + std::cos(a0) * r, y + h - r + std::sin(a0) * r,
                       x + w - r + std::cos(a1) * r, y + h - r + std::sin(a1) * r);
        // Bottom-left
        SDL_RenderLine(m_Renderer,
                       x + r - std::sin(a0) * r, y + h - r + std::cos(a0) * r,
                       x + r - std::sin(a1) * r, y + h - r + std::cos(a1) * r);
    }
}

void Renderer::FillCircle(float cx, float cy, float radius)
{
    if (!m_Renderer)
        return;

    const int r = static_cast<int>(radius);
    for (int dy = -r; dy <= r; ++dy)
    {
        const int dx = static_cast<int>(std::sqrt(static_cast<float>(r * r - dy * dy)));
        SDL_FRect row{cx - static_cast<float>(dx), cy + static_cast<float>(dy),
                      static_cast<float>(2 * dx + 1), 1.0f};
        SDL_RenderFillRect(m_Renderer, &row);
    }
}

void Renderer::DrawCircle(float cx, float cy, float radius)
{
    if (!m_Renderer)
        return;

    const int steps = std::max(32, static_cast<int>(radius * 2.0f));
    for (int i = 0; i < steps; ++i)
    {
        const float a0 = static_cast<float>(i) / static_cast<float>(steps) * 6.28318f;
        const float a1 = static_cast<float>(i + 1) / static_cast<float>(steps) * 6.28318f;
        SDL_RenderLine(m_Renderer,
                       cx + std::cos(a0) * radius, cy + std::sin(a0) * radius,
                       cx + std::cos(a1) * radius, cy + std::sin(a1) * radius);
    }
}

void Renderer::DrawText(const Rect& rect, std::string_view text, const flachead::graphics::Font& font, const Color& color)
{
    if (!m_Renderer || !font.Valid() || text.empty())
    {
        return;
    }

    const std::string textString{text};
    SDL_Color sdlColor{color.r, color.g, color.b, color.a};
    SDL_Surface* surface = TTF_RenderText_Blended(font.Native(), textString.c_str(), textString.size(), sdlColor);
    if (!surface)
    {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
    if (texture)
    {
        SDL_FRect dstRect{rect.position.x, rect.position.y, rect.size.x, rect.size.y};
        SDL_RenderTexture(m_Renderer, texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);
    }

    SDL_DestroySurface(surface);
}
} // namespace flachead::core