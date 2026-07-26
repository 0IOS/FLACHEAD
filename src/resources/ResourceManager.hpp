#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace flachead::resources
{
template <typename T>
class ResourceManager
{
public:
    std::shared_ptr<T> Acquire(std::string_view key);

private:
    std::unordered_map<std::string, std::shared_ptr<T>> m_Resources;
};

template <typename T>
std::shared_ptr<T> ResourceManager<T>::Acquire(std::string_view key)
{
    auto it = m_Resources.find(std::string{key});
    if (it != m_Resources.end())
    {
        return it->second;
    }

    auto resource = std::make_shared<T>();
    m_Resources.emplace(std::string{key}, resource);
    return resource;
}
} // namespace flachead::resources
