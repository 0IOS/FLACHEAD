#pragma once

#include "../database/Database.hpp"
#include "../events/EventBus.hpp"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace flachead::services
{
// Typed key/value settings. Values are cached in memory for zero-cost reads
// and written through to the `settings` table whenever Initialize() has bound
// a database. Publishes SettingsChanged (key in stringValue) on every change.
class SettingsManager
{
public:
    // Bind a database + event bus and load all persisted settings. Calling
    // this again reloads. Without it, Set()/Get() are memory-only.
    void Initialize(flachead::database::Database& db, flachead::events::EventBus& eventBus);

    void Set(std::string_view key, std::string_view value);
    std::string Get(std::string_view key, std::string_view defaultValue = {}) const;
    bool Has(std::string_view key) const;
    void Delete(std::string_view key);

    int   GetInt(std::string_view key, int defaultValue = 0) const;
    void  SetInt(std::string_view key, int value);
    float GetFloat(std::string_view key, float defaultValue = 0.0f) const;
    void  SetFloat(std::string_view key, float value);
    bool  GetBool(std::string_view key, bool defaultValue = false) const;
    void  SetBool(std::string_view key, bool value);

    // All currently cached keys, for diagnostics.
    std::vector<std::string> Keys() const;

private:
    void Persist(std::string_view key, std::string_view value);

    flachead::database::Database* m_Db{nullptr};
    flachead::events::EventBus* m_EventBus{nullptr};
    std::map<std::string, std::string, std::less<>> m_Settings;
};
} // namespace flachead::services
