#include "Renderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

namespace flachead::core
{
namespace
{
constexpr int   kTextCacheCap      = 256;
constexpr int   kShapeCacheCap     = 32;
constexpr int   kMaskMaxRadius     = 96;

SDL_Surface* CreateMaskSurface(int size)
{
    return SDL_CreateSurface(size, size, SDL_PIXELFORMAT_RGBA8888);
}

void WriteMaskPixel(Uint8* base, Uint8 alpha)
{
    base[0] = 255;
    base[1] = 255;
    base[2] = 255;
    base[3] = alpha;
}
} // namespace

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
        m_DisplayBackend = std::make_unique<flachead::system::SdlDisplayBackend>(m_Renderer);
    }
    return m_Renderer != nullptr;
}

void Renderer::Destroy()
{
    if (!m_Renderer)
    {
        return;
    }

    for (const auto& [key, glyph] : m_TextCache)
    {
        (void)key;
        SDL_DestroyTexture(glyph.texture);
    }
    m_TextCache.clear();
    m_TextCacheOrder.clear();

    for (const auto& [key, texture] : m_Corners)
    {
        (void)key;
        SDL_DestroyTexture(texture);
    }
    m_Corners.clear();

    for (const auto& [key, texture] : m_CornerOutlines)
    {
        (void)key;
        SDL_DestroyTexture(texture);
    }
    m_CornerOutlines.clear();

    for (const auto& [key, texture] : m_Discs)
    {
        (void)key;
        SDL_DestroyTexture(texture);
    }
    m_Discs.clear();

    for (const auto& [key, texture] : m_Rings)
    {
        (void)key;
        SDL_DestroyTexture(texture);
    }
    m_Rings.clear();

    SDL_DestroyRenderer(m_Renderer);
    m_Renderer = nullptr;
}

void Renderer::BeginFrame()
{
    SDL_SetRenderDrawColor(m_Renderer, 5, 7, 12, 255);
    SDL_RenderClear(m_Renderer);
}

void Renderer::EndFrame()
{
    if (m_DisplayBackend)
    {
        m_DisplayBackend->Present();
    }
    else
    {
        SDL_RenderPresent(m_Renderer);
    }
}

void Renderer::SetDisplayBackend(std::unique_ptr<flachead::system::DisplayBackend> backend)
{
    m_DisplayBackend = std::move(backend);
}

Renderer::Stats Renderer::GetStats() const
{
    Stats stats;
    stats.textureCount += static_cast<int>(m_TextCache.size());
    for (const auto& [key, glyph] : m_TextCache)
    {
        (void)key;
        stats.textureBytes += static_cast<size_t>(glyph.width) * static_cast<size_t>(glyph.height) * 4u;
    }

    auto countShapeCache = [&stats](const std::unordered_map<int, SDL_Texture*>& cache, bool corners) {
        stats.textureCount += static_cast<int>(cache.size());
        for (const auto& [radius, texture] : cache)
        {
            (void)texture;
            const int size = corners ? radius + 1 : 2 * radius + 1;
            stats.textureBytes += static_cast<size_t>(size) * static_cast<size_t>(size) * 4u;
        }
    };

    countShapeCache(m_Corners, true);
    countShapeCache(m_CornerOutlines, true);
    countShapeCache(m_Discs, false);
    countShapeCache(m_Rings, false);
    return stats;
}

void Renderer::SetColor(const Color& color)
{
    m_CurrentColor = color;
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

void Renderer::PrepareShape(SDL_Texture* texture)
{
    SDL_SetTextureColorMod(texture, m_CurrentColor.r, m_CurrentColor.g, m_CurrentColor.b);
    SDL_SetTextureAlphaMod(texture, m_CurrentColor.a);
    SDL_SetTextureBlendMode(texture, m_CurrentColor.a == 255 ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
}

void Renderer::BlitShape(SDL_Texture* texture, float x, float y, float w, float h)
{
    PrepareShape(texture);

    SDL_FRect dst{x, y, w, h};
    SDL_RenderTexture(m_Renderer, texture, nullptr, &dst);
}

SDL_Texture* Renderer::CornerTexture(int radius, bool outline)
{
    auto& cache = outline ? m_CornerOutlines : m_Corners;

    auto it = cache.find(radius);
    if (it != cache.end())
    {
        return it->second;
    }

    if (cache.size() >= kShapeCacheCap)
    {
        auto victim = cache.begin();
        SDL_DestroyTexture(victim->second);
        cache.erase(victim);
    }

    const int size = radius + 1;
    SDL_Surface* surface = CreateMaskSurface(size);
    const int bytesPerPixel = SDL_GetPixelFormatDetails(surface->format)->bytes_per_pixel;
    Uint8* pixels = static_cast<Uint8*>(surface->pixels);
    const int rr = radius * radius;

    for (int j = 0; j < size; ++j)
    {
        for (int i = 0; i < size; ++i)
        {
            const int dx = radius - i;
            const int dy = radius - j;
            const int dist2 = dx * dx + dy * dy;
            Uint8 alpha = 0;
            if (!outline)
            {
                alpha = dist2 <= rr ? 255 : 0;
            }
            else
            {
                const int dOuter = (radius + 1) * (radius + 1);
                alpha = (dist2 <= dOuter && dist2 >= rr - 2 * radius) ? 255 : 0;
            }
            WriteMaskPixel(pixels + (j * surface->pitch + i * bytesPerPixel), alpha);
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
    SDL_DestroySurface(surface);
    cache.emplace(radius, texture);
    return texture;
}

SDL_Texture* Renderer::DiscTexture(int radius, bool outline)
{
    auto& cache = outline ? m_Rings : m_Discs;

    auto it = cache.find(radius);
    if (it != cache.end())
    {
        return it->second;
    }

    if (cache.size() >= kShapeCacheCap)
    {
        auto victim = cache.begin();
        SDL_DestroyTexture(victim->second);
        cache.erase(victim);
    }

    const int size = 2 * radius + 1;
    SDL_Surface* surface = CreateMaskSurface(size);
    const int bytesPerPixel = SDL_GetPixelFormatDetails(surface->format)->bytes_per_pixel;
    Uint8* pixels = static_cast<Uint8*>(surface->pixels);
    const int rr = radius * radius;

    for (int j = 0; j < size; ++j)
    {
        for (int i = 0; i < size; ++i)
        {
            const int dx = i - radius;
            const int dy = j - radius;
            const int dist2 = dx * dx + dy * dy;
            Uint8 alpha = 0;
            if (!outline)
            {
                alpha = dist2 <= rr ? 255 : 0;
            }
            else
            {
                const int dOuter = (radius + 1) * (radius + 1);
                alpha = (dist2 <= dOuter && dist2 >= rr - 2 * radius) ? 255 : 0;
            }
            WriteMaskPixel(pixels + (j * surface->pitch + i * bytesPerPixel), alpha);
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
    SDL_DestroySurface(surface);
    cache.emplace(radius, texture);
    return texture;
}

void Renderer::FillRoundedRect(const Rect& rect, float radius)
{
    if (!m_Renderer)
        return;

    const float w = rect.size.x;
    const float h = rect.size.y;
    const float clamped = std::min(radius, std::min(w, h) * 0.5f);

    if (clamped < 1.0f)
    {
        FillRect(rect);
        return;
    }

    const int ri = static_cast<int>(clamped + 0.5f);
    SDL_Texture* corner = CornerTexture(ri, false);

    const int xi = static_cast<int>(rect.position.x + 0.5f);
    const int yi = static_cast<int>(rect.position.y + 0.5f);
    const int wi = std::max(2 * ri, static_cast<int>(w + 0.5f));
    const int hi = std::max(2 * ri, static_cast<int>(h + 0.5f));

    SDL_FRect center{static_cast<float>(xi + ri), static_cast<float>(yi + ri),
                     static_cast<float>(wi - 2 * ri), static_cast<float>(hi - 2 * ri)};
    SDL_RenderFillRect(m_Renderer, &center);

    SDL_FRect top{static_cast<float>(xi + ri), static_cast<float>(yi),
                  static_cast<float>(wi - 2 * ri), static_cast<float>(ri)};
    SDL_RenderFillRect(m_Renderer, &top);

    SDL_FRect bottom{static_cast<float>(xi + ri), static_cast<float>(yi + hi - ri),
                     static_cast<float>(wi - 2 * ri), static_cast<float>(ri)};
    SDL_RenderFillRect(m_Renderer, &bottom);

    SDL_FRect left{static_cast<float>(xi), static_cast<float>(yi + ri),
                   static_cast<float>(ri), static_cast<float>(hi - 2 * ri)};
    SDL_RenderFillRect(m_Renderer, &left);

    SDL_FRect right{static_cast<float>(xi + wi - ri), static_cast<float>(yi + ri),
                    static_cast<float>(ri), static_cast<float>(hi - 2 * ri)};
    SDL_RenderFillRect(m_Renderer, &right);

    const float cs = static_cast<float>(ri);
    SDL_FRect tl{static_cast<float>(xi), static_cast<float>(yi), cs, cs};
    PrepareShape(corner);
    SDL_RenderTexture(m_Renderer, corner, nullptr, &tl);

    SDL_FRect tr{static_cast<float>(xi + wi - ri), static_cast<float>(yi), cs, cs};
    SDL_RenderTextureRotated(m_Renderer, corner, nullptr, &tr, 0.0, nullptr, SDL_FLIP_HORIZONTAL);

    SDL_FRect bl{static_cast<float>(xi), static_cast<float>(yi + hi - ri), cs, cs};
    SDL_RenderTextureRotated(m_Renderer, corner, nullptr, &bl, 0.0, nullptr, SDL_FLIP_VERTICAL);

    SDL_FRect br{static_cast<float>(xi + wi - ri), static_cast<float>(yi + hi - ri), cs, cs};
    SDL_RenderTextureRotated(m_Renderer, corner, nullptr, &br, 0.0, nullptr,
                             static_cast<SDL_FlipMode>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL));
}

void Renderer::DrawRoundedRect(const Rect& rect, float radius)
{
    if (!m_Renderer)
        return;

    const float w = rect.size.x;
    const float h = rect.size.y;
    const float clamped = std::min(radius, std::min(w, h) * 0.5f);

    if (clamped < 1.0f)
    {
        DrawRect(rect);
        return;
    }

    const int ri = static_cast<int>(clamped + 0.5f);
    SDL_Texture* corner = CornerTexture(ri, true);

    const int xi = static_cast<int>(rect.position.x + 0.5f);
    const int yi = static_cast<int>(rect.position.y + 0.5f);
    const int wi = std::max(2 * ri, static_cast<int>(w + 0.5f));
    const int hi = std::max(2 * ri, static_cast<int>(h + 0.5f));

    SDL_RenderLine(m_Renderer, static_cast<float>(xi + ri), static_cast<float>(yi),
                   static_cast<float>(xi + wi - ri), static_cast<float>(yi));
    SDL_RenderLine(m_Renderer, static_cast<float>(xi + ri), static_cast<float>(yi + hi),
                   static_cast<float>(xi + wi - ri), static_cast<float>(yi + hi));
    SDL_RenderLine(m_Renderer, static_cast<float>(xi), static_cast<float>(yi + ri),
                   static_cast<float>(xi), static_cast<float>(yi + hi - ri));
    SDL_RenderLine(m_Renderer, static_cast<float>(xi + wi), static_cast<float>(yi + ri),
                   static_cast<float>(xi + wi), static_cast<float>(yi + hi - ri));

    const float cs = static_cast<float>(ri);
    SDL_FRect tl{static_cast<float>(xi), static_cast<float>(yi), cs, cs};
    PrepareShape(corner);
    SDL_RenderTexture(m_Renderer, corner, nullptr, &tl);

    SDL_FRect tr{static_cast<float>(xi + wi - ri), static_cast<float>(yi), cs, cs};
    SDL_RenderTextureRotated(m_Renderer, corner, nullptr, &tr, 0.0, nullptr, SDL_FLIP_HORIZONTAL);

    SDL_FRect bl{static_cast<float>(xi), static_cast<float>(yi + hi - ri), cs, cs};
    SDL_RenderTextureRotated(m_Renderer, corner, nullptr, &bl, 0.0, nullptr, SDL_FLIP_VERTICAL);

    SDL_FRect br{static_cast<float>(xi + wi - ri), static_cast<float>(yi + hi - ri), cs, cs};
    SDL_RenderTextureRotated(m_Renderer, corner, nullptr, &br, 0.0, nullptr,
                             static_cast<SDL_FlipMode>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL));
}

void Renderer::FillCircle(float cx, float cy, float radius)
{
    if (!m_Renderer)
        return;

    const int ri = static_cast<int>(radius + 0.5f);
    if (ri < 1 || ri > kMaskMaxRadius)
    {
        for (int dy = -ri; dy <= ri; ++dy)
        {
            const int dx = static_cast<int>(std::sqrt(static_cast<float>(ri * ri - dy * dy)));
            SDL_FRect row{cx - static_cast<float>(dx), cy + static_cast<float>(dy),
                          static_cast<float>(2 * dx + 1), 1.0f};
            SDL_RenderFillRect(m_Renderer, &row);
        }
        return;
    }

    SDL_Texture* disc = DiscTexture(ri, false);
    BlitShape(disc, cx - static_cast<float>(ri), cy - static_cast<float>(ri),
              static_cast<float>(2 * ri + 1), static_cast<float>(2 * ri + 1));
}

void Renderer::DrawCircle(float cx, float cy, float radius)
{
    if (!m_Renderer)
        return;

    const int ri = static_cast<int>(radius + 0.5f);
    if (ri < 1 || ri > kMaskMaxRadius)
    {
        const int steps = std::max(32, ri * 2);
        for (int i = 0; i < steps; ++i)
        {
            const float a0 = static_cast<float>(i) / static_cast<float>(steps) * 6.28318f;
            const float a1 = static_cast<float>(i + 1) / static_cast<float>(steps) * 6.28318f;
            SDL_RenderLine(m_Renderer,
                           cx + std::cos(a0) * radius, cy + std::sin(a0) * radius,
                           cx + std::cos(a1) * radius, cy + std::sin(a1) * radius);
        }
        return;
    }

    SDL_Texture* ring = DiscTexture(ri, true);
    BlitShape(ring, cx - static_cast<float>(ri), cy - static_cast<float>(ri),
              static_cast<float>(2 * ri + 1), static_cast<float>(2 * ri + 1));
}

uint64_t Renderer::TextHash(std::string_view text, const flachead::graphics::Font& font, const Color& color)
{
    const uint64_t fnv = 1099511628211ULL;
    uint64_t hash = 1469598103934665603ULL;
    const uint64_t seed = reinterpret_cast<uintptr_t>(font.Native());

    for (int i = 0; i < 8; ++i)
    {
        hash ^= (seed >> (i * 8)) & 0xFF;
        hash *= fnv;
    }
    hash ^= static_cast<uint64_t>(color.r);
    hash *= fnv;
    hash ^= static_cast<uint64_t>(color.g);
    hash *= fnv;
    hash ^= static_cast<uint64_t>(color.b);
    hash *= fnv;
    hash ^= static_cast<uint64_t>(color.a);
    hash *= fnv;
    hash ^= static_cast<uint64_t>(text.size());
    hash *= fnv;

    for (char byte : text)
    {
        hash ^= static_cast<uint8_t>(byte);
        hash *= fnv;
    }
    return hash;
}

void Renderer::DrawText(const Rect& rect, std::string_view text, const flachead::graphics::Font& font, const Color& color)
{
    if (!m_Renderer || !font.Valid() || text.empty())
    {
        return;
    }

    const uint64_t key = TextHash(text, font, color);
    TextGlyph glyph;

    auto it = m_TextCache.find(key);
    if (it != m_TextCache.end())
    {
        glyph = it->second;
    }
    else
    {
        SDL_Color sdlColor{color.r, color.g, color.b, color.a};
        SDL_Surface* surface = TTF_RenderText_Blended(font.Native(), text.data(), text.size(), sdlColor);
        if (!surface)
        {
            return;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
        if (texture)
        {
            glyph = {texture, surface->w, surface->h};
            m_TextCache.emplace(key, glyph);
            m_TextCacheOrder.push_back(key);

            if (m_TextCache.size() > kTextCacheCap)
            {
                const uint64_t oldKey = m_TextCacheOrder.front();
                m_TextCacheOrder.erase(m_TextCacheOrder.begin());
                auto oldIt = m_TextCache.find(oldKey);
                if (oldIt != m_TextCache.end())
                {
                    SDL_DestroyTexture(oldIt->second.texture);
                    m_TextCache.erase(oldIt);
                }
            }
        }

        SDL_DestroySurface(surface);
    }

    float w = static_cast<float>(glyph.width);
    float h = static_cast<float>(glyph.height);

    if (rect.size.y > 0.0f)
    {
        float scale = rect.size.y / h;
        if (rect.size.x > 0.0f && w * scale > rect.size.x)
        {
            scale = rect.size.x / w;
        }
        w *= scale;
        h *= scale;
    }

    SDL_FRect dstRect{rect.position.x, rect.position.y, w, h};
    SDL_RenderTexture(m_Renderer, glyph.texture, nullptr, &dstRect);
}
} // namespace flachead::core
