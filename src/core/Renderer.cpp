#include "Renderer.hpp"

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
    SDL_SetRenderDrawColor(m_Renderer, 20, 20, 20, 255);
    SDL_RenderClear(m_Renderer);
}

void Renderer::EndFrame()
{
    SDL_RenderPresent(m_Renderer);
}

void Renderer::SetColor(const Color& color)
{
    SDL_SetRenderDrawColor(
        m_Renderer,
        color.r,
        color.g,
        color.b,
        color.a
    );
}

void Renderer::DrawRect(const Rect& rect)
{
    SDL_FRect sdlRect{
        rect.position.x,
        rect.position.y,
        rect.size.x,
        rect.size.y
    };

    SDL_RenderRect(m_Renderer, &sdlRect);
}

void Renderer::FillRect(const Rect& rect)
{
    SDL_FRect sdlRect{
        rect.position.x,
        rect.position.y,
        rect.size.x,
        rect.size.y
    };

    SDL_RenderFillRect(m_Renderer, &sdlRect);
}