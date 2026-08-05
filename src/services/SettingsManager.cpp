#include "SettingsManager.hpp"

#include "../events/EventBus.hpp"

#include <sqlite3.h>

#include <cstdlib>

namespace flachead::services
{
using flachead::database::Statement;
using flachead::events::EventBus;

void SettingsManager::Initialize(flachead::database::Database& db, EventBus& eventBus)
{
    m_Db = &db;
    m_EventBus = &eventBus;
    m_Settings.clear();

    Statement stmt = db.Prepare("SELECT key, value FROM settings;");
    while (stmt.Step() == SQLITE_ROW)
    {
        m_Settings[stmt.ColumnText(0)] = stmt.ColumnText(1);
    }
}

void SettingsManager::Persist(std::string_view key, std::string_view value)
{
    if (!m_Db)
    {
        return;
    }
    Statement stmt = m_Db->Prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?);");
    stmt.Bind(1, key);
    stmt.Bind(2, value);
    stmt.Step();

    if (m_EventBus)
    {
        flachead::events::Event event;
        event.type = flachead::events::Type::SettingsChanged;
        event.stringValue = std::string{key};
        m_EventBus->Publish(event);
    }
}

void SettingsManager::Set(std::string_view key, std::string_view value)
{
    m_Settings[std::string{key}] = std::string{value};
    Persist(key, value);
}

std::string SettingsManager::Get(std::string_view key, std::string_view defaultValue) const
{
    auto it = m_Settings.find(key);
    if (it != m_Settings.end())
    {
        return it->second;
    }
    return std::string{defaultValue};
}

bool SettingsManager::Has(std::string_view key) const
{
    return m_Settings.find(key) != m_Settings.end();
}

void SettingsManager::Delete(std::string_view key)
{
    m_Settings.erase(std::string{key});
    if (!m_Db)
    {
        return;
    }
    Statement stmt = m_Db->Prepare("DELETE FROM settings WHERE key = ?;");
    stmt.Bind(1, key);
    stmt.Step();
}

int SettingsManager::GetInt(std::string_view key, int defaultValue) const
{
    const std::string value = Get(key);
    if (value.empty())
    {
        return defaultValue;
    }
    return std::atoi(value.c_str());
}

void SettingsManager::SetInt(std::string_view key, int value)
{
    Set(key, std::to_string(value));
}

float SettingsManager::GetFloat(std::string_view key, float defaultValue) const
{
    const std::string value = Get(key);
    if (value.empty())
    {
        return defaultValue;
    }
    return std::strtof(value.c_str(), nullptr);
}

void SettingsManager::SetFloat(std::string_view key, float value)
{
    Set(key, std::to_string(value));
}

bool SettingsManager::GetBool(std::string_view key, bool defaultValue) const
{
    const std::string value = Get(key);
    if (value.empty())
    {
        return defaultValue;
    }
    return value == "1" || value == "true";
}

void SettingsManager::SetBool(std::string_view key, bool value)
{
    Set(key, value ? "1" : "0");
}

std::vector<std::string> SettingsManager::Keys() const
{
    std::vector<std::string> keys;
    keys.reserve(m_Settings.size());
    for (const auto& [key, value] : m_Settings)
    {
        keys.push_back(key);
    }
    return keys;
}
} // namespace flachead::services
