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

void Renderer::SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(m_Renderer, r, g, b, a);
}

void Renderer::DrawRect(float x, float y, float w, float h)
{
    SDL_FRect rect{ x, y, w, h };
    SDL_RenderRect(m_Renderer, &rect);
}

void Renderer::FillRect(float x, float y, float w, float h)
{
    SDL_FRect rect{ x, y, w, h };
    SDL_RenderFillRect(m_Renderer, &rect);
}