#include "UniversalSearchScreen.hpp"

#include "../ui/Button.hpp"
#include "../ui/Label.hpp"
#include "../ui/TextField.hpp"

#include <cctype>
#include <string>

namespace flachead::shell
{
namespace
{
constexpr const char* kKeyboardRows[] = {"abcdefg", "hijklmn", "opqrstu", "vwxyz "};
}

UniversalSearchScreen::UniversalSearchScreen(const ShellServices& services)
    : ShellScreen(services)
{
}

void UniversalSearchScreen::BuildShell()
{
    auto& root = Root();
    root.SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Vertical));

    m_Field = new flachead::ui::TextField();
    m_Field->SetId("field");
    m_Field->SetPlaceholder("Search songs, artists, albums");
    m_Field->SetFocusable(true);
    m_Field->SetChangeHandler([this] { RefreshResults(); });
    auto field = std::unique_ptr<flachead::ui::TextField>(m_Field);
    root.AddChild(std::move(field));

    m_Status = new flachead::ui::Label();
    m_Status->SetText("");
    m_Status->SetFontSize(11.0f);
    m_Status->SetAlign(flachead::ui::Label::Align::Left);
    auto status = std::unique_ptr<flachead::ui::Label>(m_Status);
    root.AddChild(std::move(status));

    m_Results = new flachead::ui::Container();
    m_Results->SetId("results");
    m_Results->SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Vertical));
    auto results = std::unique_ptr<flachead::ui::Container>(static_cast<flachead::ui::Container*>(m_Results));
    root.AddChild(std::move(results));

    auto keyboard = std::make_unique<flachead::ui::Container>();
    keyboard->SetLayout(flachead::layout::MakeGrid(7));
    flachead::ui::Widget* kb = keyboard.get();
    root.AddChild(std::move(keyboard));

    for (const char* row : kKeyboardRows)
    {
        for (const char* cursor = row; *cursor; ++cursor)
        {
            const char key = *cursor;
            auto tile = std::make_unique<flachead::ui::Button>();
            tile->SetId(std::string{"key_"} + key);
            tile->SetText(std::string(1, key == ' ' ? 'S' : key));
            tile->SetFocusable(true);
            if (key == ' ')
            {
                tile->SetClickHandler([this] { AppendChar(' '); });
            }
            else
            {
                tile->SetClickHandler([this, key] { AppendChar(key); });
            }
            kb->AddChild(std::move(tile));
        }
    }
    auto backspace = std::make_unique<flachead::ui::Button>();
    backspace->SetId("key_back");
    backspace->SetText("Del");
    backspace->SetFocusable(true);
    backspace->SetClickHandler([this] { Backspace(); });
    kb->AddChild(std::move(backspace));

    RefreshResults();
}

void UniversalSearchScreen::AppendChar(char ch)
{
    if (m_Field)
    {
        m_Field->SetQuery(std::string{m_Field->Query()} + ch);
        RefreshResults();
    }
}

void UniversalSearchScreen::Backspace()
{
    if (m_Field)
    {
        const std::string_view query = m_Field->Query();
        m_Field->SetQuery(query.empty() ? "" : query.substr(0, query.size() - 1));
        RefreshResults();
    }
}

void UniversalSearchScreen::RefreshResults()
{
    if (!Ctx().library)
    {
        return;
    }
    const std::string query{m_Field ? m_Field->Query() : std::string{}};
    if (query.empty())
    {
        m_Songs = Ctx().library->Recent(6);
    }
    else
    {
        m_Songs = Ctx().library->Search(query);
        if (m_Songs.size() > 6)
        {
            m_Songs.resize(6);
        }
    }
    RebuildResults(m_Songs);
}

void UniversalSearchScreen::RebuildResults(const std::vector<flachead::models::SongModel>& songs)
{
    if (m_Status)
    {
        if (songs.empty())
        {
            m_Status->SetText("No matches");
        }
        else
        {
            m_Status->SetText("Recent picks");
        }
    }
    if (!m_Results)
    {
        return;
    }
    m_Results->RemoveAllChildren();
    for (const auto& song : songs)
    {
        auto row = std::make_unique<flachead::ui::Button>();
        row->SetId("song_" + std::to_string(song.id));
        row->SetText(song.DisplayTitle() + " - " + song.DisplayArtist());
        row->SetFocusable(true);
        row->SetClickHandler([this, song] { PlayTrack(song); });
        m_Results->AddChild(std::move(row));
    }
    MarkDirty();
}

void UniversalSearchScreen::PlayTrack(const flachead::models::SongModel& song)
{
    if (!Ctx().playback)
    {
        return;
    }
    Ctx().playback->PlayTrack(song);
    Ctx().goBack();
    Ctx().goBack();
}

void UniversalSearchScreen::OnShellUpdate(float deltaSeconds)
{
    (void)deltaSeconds;
}

bool UniversalSearchScreen::OnShellCommand(flachead::commands::Command command)
{
    switch (command)
    {
        case flachead::commands::Command::Back:
            Ctx().goBack();
            return true;
        case flachead::commands::Command::Select:
        case flachead::commands::Command::PlayPause:
            return false;
        default:
            return false;
    }
}
} // namespace flachead::shell
