#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace flachead::metadata
{
// Minimal JSON value tree used to parse ffprobe output. Supports objects,
// arrays, strings, numbers, booleans and null. Numbers are stored as double.
class JsonValue
{
public:
    enum class Type
    {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object,
    };

    using Member = std::pair<std::string, JsonValue>;

    JsonValue() = default;

    static JsonValue MakeNull();
    static JsonValue MakeBool(bool value);
    static JsonValue MakeNumber(double value);
    static JsonValue MakeString(std::string value);
    static JsonValue MakeArray();
    static JsonValue MakeObject();

    Type GetType() const { return m_Type; }
    bool IsNull() const { return m_Type == Type::Null; }

    bool AsBool() const { return m_Bool; }
    double AsNumber() const { return m_Number; }
    const std::string& AsString() const { return m_String; }

    const std::vector<JsonValue>& AsArray() const { return m_Array; }
    const std::vector<Member>& AsObject() const { return m_Object; }

    // Returns nullptr when key is missing or the value is not an object.
    const JsonValue* Find(std::string_view key) const;
    // Like Find but returns a default when missing.
    std::string GetString(std::string_view key, std::string fallback = {}) const;
    double GetNumber(std::string_view key, double fallback = 0.0) const;
    bool GetBool(std::string_view key, bool fallback = false) const;

    // Mutators used by the parser to build values in place.
    void SetBool(bool value) { m_Type = Type::Bool; m_Bool = value; }
    void SetNumber(double value) { m_Type = Type::Number; m_Number = value; }
    void SetString(std::string value) { m_Type = Type::String; m_String = std::move(value); }
    void SetArray() { m_Type = Type::Array; }
    void SetObject() { m_Type = Type::Object; }
    void AddArrayElement(JsonValue value) { m_Array.push_back(std::move(value)); }
    void AddObjectMember(std::string key, JsonValue value)
    {
        m_Object.emplace_back(std::move(key), std::move(value));
    }

private:
    Type m_Type{Type::Null};
    bool m_Bool{false};
    double m_Number{0.0};
    std::string m_String;
    std::vector<JsonValue> m_Array;
    std::vector<Member> m_Object;
};

// Parses a complete JSON document. Returns false on syntax error.
bool ParseJson(std::string_view text, JsonValue& out);
} // namespace flachead::metadata
