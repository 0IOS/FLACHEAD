#include "Json.hpp"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cmath>

namespace flachead::metadata
{
JsonValue JsonValue::MakeNull()
{
    return JsonValue{};
}

JsonValue JsonValue::MakeBool(bool value)
{
    JsonValue v;
    v.m_Type = Type::Bool;
    v.m_Bool = value;
    return v;
}

JsonValue JsonValue::MakeNumber(double value)
{
    JsonValue v;
    v.m_Type = Type::Number;
    v.m_Number = value;
    return v;
}

JsonValue JsonValue::MakeString(std::string value)
{
    JsonValue v;
    v.m_Type = Type::String;
    v.m_String = std::move(value);
    return v;
}

JsonValue JsonValue::MakeArray()
{
    JsonValue v;
    v.m_Type = Type::Array;
    return v;
}

JsonValue JsonValue::MakeObject()
{
    JsonValue v;
    v.m_Type = Type::Object;
    return v;
}

const JsonValue* JsonValue::Find(std::string_view key) const
{
    if (m_Type != Type::Object)
    {
        return nullptr;
    }
    for (const auto& member : m_Object)
    {
        if (member.first == key)
        {
            return &member.second;
        }
    }
    return nullptr;
}

std::string JsonValue::GetString(std::string_view key, std::string fallback) const
{
    const JsonValue* value = Find(key);
    if (value && value->m_Type == Type::String)
    {
        return value->m_String;
    }
    return fallback;
}

double JsonValue::GetNumber(std::string_view key, double fallback) const
{
    const JsonValue* value = Find(key);
    if (value && value->m_Type == Type::Number)
    {
        return value->m_Number;
    }
    return fallback;
}

bool JsonValue::GetBool(std::string_view key, bool fallback) const
{
    const JsonValue* value = Find(key);
    if (value && value->m_Type == Type::Bool)
    {
        return value->m_Bool;
    }
    return fallback;
}

namespace
{
class Parser
{
public:
    explicit Parser(std::string_view text)
        : m_Text(text)
    {
    }

    bool Parse(JsonValue& out)
    {
        SkipWhitespace();
        const bool ok = ParseValue(out);
        SkipWhitespace();
        return ok && AtEnd();
    }

private:
    bool AtEnd() const { return m_Pos >= m_Text.size(); }
    char Peek() const { return AtEnd() ? '\0' : m_Text[m_Pos]; }

    void SkipWhitespace()
    {
        while (!AtEnd() && std::isspace(static_cast<unsigned char>(m_Text[m_Pos])))
        {
            ++m_Pos;
        }
    }

    bool Consume(char expected)
    {
        if (Peek() != expected)
        {
            return false;
        }
        ++m_Pos;
        return true;
    }

    bool ParseValue(JsonValue& out)
    {
        SkipWhitespace();
        if (AtEnd())
        {
            return false;
        }

        switch (Peek())
        {
            case '{':
                return ParseObject(out);
            case '[':
                return ParseArray(out);
            case '"':
                return ParseString(out);
            case 't':
                return ParseLiteral("true", JsonValue::MakeBool(true), out);
            case 'f':
                return ParseLiteral("false", JsonValue::MakeBool(false), out);
            case 'n':
                return ParseLiteral("null", JsonValue::MakeNull(), out);
            default:
                return ParseNumber(out);
        }
    }

    bool ParseLiteral(std::string_view literal, JsonValue value, JsonValue& out)
    {
        if (m_Text.compare(m_Pos, literal.size(), literal) != 0)
        {
            return false;
        }
        m_Pos += literal.size();
        out = std::move(value);
        return true;
    }

    bool ParseString(JsonValue& out)
    {
        if (!Consume('"'))
        {
            return false;
        }

        std::string result;
        while (!AtEnd())
        {
            const char c = m_Text[m_Pos++];
            if (c == '"')
            {
                out = JsonValue::MakeString(std::move(result));
                return true;
            }
            if (c == '\\')
            {
                if (AtEnd())
                {
                    return false;
                }
                const char escape = m_Text[m_Pos++];
                switch (escape)
                {
                    case '"': result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    case '/': result.push_back('/'); break;
                    case 'b': result.push_back('\b'); break;
                    case 'f': result.push_back('\f'); break;
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    case 'u':
                    {
                        // Best-effort UTF-16 escape handling: decode BMP code
                        // points, emit raw UTF-8. Surrogate pairs are skipped
                        // (rare in ffprobe metadata).
                        if (m_Pos + 4 > m_Text.size())
                        {
                            return false;
                        }
                        unsigned int code = 0;
                        for (int i = 0; i < 4; ++i)
                        {
                            const char hex = m_Text[m_Pos + i];
                            code <<= 4;
                            if (hex >= '0' && hex <= '9')
                                code |= static_cast<unsigned>(hex - '0');
                            else if (hex >= 'a' && hex <= 'f')
                                code |= static_cast<unsigned>(hex - 'a' + 10);
                            else if (hex >= 'A' && hex <= 'F')
                                code |= static_cast<unsigned>(hex - 'A' + 10);
                            else
                                return false;
                        }
                        m_Pos += 4;
                        if (code < 0x80)
                        {
                            result.push_back(static_cast<char>(code));
                        }
                        else if (code < 0x800)
                        {
                            result.push_back(static_cast<char>(0xC0 | (code >> 6)));
                            result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                        }
                        else
                        {
                            result.push_back(static_cast<char>(0xE0 | (code >> 12)));
                            result.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
                            result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                        }
                        break;
                    }
                    default:
                        return false;
                }
            }
            else
            {
                result.push_back(c);
            }
        }
        return false;
    }

    bool ParseNumber(JsonValue& out)
    {
        const std::size_t start = m_Pos;
        if (Peek() == '-')
        {
            ++m_Pos;
        }
        while (!AtEnd() && (std::isdigit(static_cast<unsigned char>(Peek())) || Peek() == '.' || Peek() == 'e' || Peek() == 'E' || Peek() == '+' || Peek() == '-'))
        {
            ++m_Pos;
        }
        if (m_Pos == start || (m_Pos == start + 1 && m_Text[start] == '-'))
        {
            return false;
        }

        char buffer[64];
        const std::size_t len = m_Pos - start;
        if (len >= sizeof(buffer))
        {
            return false;
        }
        std::memcpy(buffer, m_Text.data() + start, len);
        buffer[len] = '\0';

        char* end = nullptr;
        const double value = std::strtod(buffer, &end);
        if (end != buffer + len)
        {
            return false;
        }
        out = JsonValue::MakeNumber(value);
        return true;
    }

    bool ParseArray(JsonValue& out)
    {
        if (!Consume('['))
        {
            return false;
        }
        JsonValue array = JsonValue::MakeArray();

        SkipWhitespace();
        if (Consume(']'))
        {
            out = std::move(array);
            return true;
        }

        while (true)
        {
            JsonValue element;
            if (!ParseValue(element))
            {
                return false;
            }
            array.AddArrayElement(std::move(element));

            SkipWhitespace();
            if (Consume(']'))
            {
                break;
            }
            if (!Consume(','))
            {
                return false;
            }
        }

        out = std::move(array);
        return true;
    }

    bool ParseObject(JsonValue& out)
    {
        if (!Consume('{'))
        {
            return false;
        }
        JsonValue object = JsonValue::MakeObject();

        SkipWhitespace();
        if (Consume('}'))
        {
            out = std::move(object);
            return true;
        }

        while (true)
        {
            SkipWhitespace();
            JsonValue key;
            if (!ParseString(key))
            {
                return false;
            }
            SkipWhitespace();
            if (!Consume(':'))
            {
                return false;
            }
            JsonValue value;
            if (!ParseValue(value))
            {
                return false;
            }
            object.AddObjectMember(key.AsString(), std::move(value));

            SkipWhitespace();
            if (Consume('}'))
            {
                break;
            }
            if (!Consume(','))
            {
                return false;
            }
        }

        out = std::move(object);
        return true;
    }

    std::string_view m_Text;
    std::size_t m_Pos{0};
};
} // namespace

bool ParseJson(std::string_view text, JsonValue& out)
{
    Parser parser{text};
    return parser.Parse(out);
}
} // namespace flachead::metadata
