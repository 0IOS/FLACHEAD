#include "Database.hpp"

#include "../core/Logger.hpp"

#include <sqlite3.h>

#include <vector>

namespace flachead::database
{
// ---------------------------------------------------------------------------
// Statement
// ---------------------------------------------------------------------------

Statement::Statement(sqlite3* db, std::string_view sql)
    : m_Db(db)
{
    if (!m_Db)
    {
        return;
    }
    const int rc = sqlite3_prepare_v2(m_Db, sql.data(), static_cast<int>(sql.size()), &m_Stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        flachead::core::Logger::Error("SQL prepare failed: " + std::string{sqlite3_errmsg(m_Db)});
        m_Stmt = nullptr;
    }
}

Statement::~Statement()
{
    Release();
}

Statement::Statement(Statement&& other) noexcept
    : m_Db(other.m_Db),
      m_Stmt(other.m_Stmt)
{
    other.m_Db = nullptr;
    other.m_Stmt = nullptr;
}

Statement& Statement::operator=(Statement&& other) noexcept
{
    if (this != &other)
    {
        Release();
        m_Db = other.m_Db;
        m_Stmt = other.m_Stmt;
        other.m_Db = nullptr;
        other.m_Stmt = nullptr;
    }
    return *this;
}

void Statement::Release()
{
    if (m_Stmt)
    {
        sqlite3_finalize(m_Stmt);
        m_Stmt = nullptr;
    }
}

bool Statement::Bind(int index, int value)
{
    return sqlite3_bind_int(m_Stmt, index, value) == SQLITE_OK;
}

bool Statement::Bind(int index, std::int64_t value)
{
    return sqlite3_bind_int64(m_Stmt, index, value) == SQLITE_OK;
}

bool Statement::Bind(int index, double value)
{
    return sqlite3_bind_double(m_Stmt, index, value) == SQLITE_OK;
}

bool Statement::Bind(int index, std::string_view value)
{
    return sqlite3_bind_text(m_Stmt, index, value.data(), static_cast<int>(value.size()),
                             SQLITE_TRANSIENT) == SQLITE_OK;
}

int Statement::Step()
{
    return sqlite3_step(m_Stmt);
}

int Statement::ColumnInt(int index) const
{
    return sqlite3_column_int(m_Stmt, index);
}

std::int64_t Statement::ColumnInt64(int index) const
{
    return sqlite3_column_int64(m_Stmt, index);
}

double Statement::ColumnDouble(int index) const
{
    return sqlite3_column_double(m_Stmt, index);
}

std::string Statement::ColumnText(int index) const
{
    const unsigned char* text = sqlite3_column_text(m_Stmt, index);
    if (!text)
    {
        return {};
    }
    const int bytes = sqlite3_column_bytes(m_Stmt, index);
    return std::string{reinterpret_cast<const char*>(text), static_cast<std::size_t>(bytes)};
}

bool Statement::IsNull(int index) const
{
    return sqlite3_column_type(m_Stmt, index) == SQLITE_NULL;
}

void Statement::Reset()
{
    if (m_Stmt)
    {
        sqlite3_reset(m_Stmt);
    }
}

void Statement::ClearBindings()
{
    if (m_Stmt)
    {
        sqlite3_clear_bindings(m_Stmt);
    }
}

// ---------------------------------------------------------------------------
// Transaction
// ---------------------------------------------------------------------------

Transaction::Transaction(sqlite3* db)
    : m_Db(db)
{
    if (m_Db)
    {
        sqlite3_exec(m_Db, "BEGIN;", nullptr, nullptr, nullptr);
        m_Active = true;
    }
}

Transaction::~Transaction()
{
    if (m_Active && m_Db)
    {
        sqlite3_exec(m_Db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
}

bool Transaction::Commit()
{
    if (!m_Active || !m_Db)
    {
        return false;
    }
    const int rc = sqlite3_exec(m_Db, "COMMIT;", nullptr, nullptr, nullptr);
    m_Active = false;
    return rc == SQLITE_OK;
}

// ---------------------------------------------------------------------------
// Database
// ---------------------------------------------------------------------------

Database::~Database()
{
    Close();
}

bool Database::Open(std::string_view path)
{
    Close();
    m_Path = path;

    if (sqlite3_open_v2(m_Path.c_str(), &m_Db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
    {
        flachead::core::Logger::Error("Failed to open database: " + std::string{sqlite3_errmsg(m_Db)});
        Close();
        return false;
    }

    sqlite3_exec(m_Db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_Db, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_Db, "PRAGMA foreign_keys=ON;", nullptr, nullptr, nullptr);

    return Migrate();
}

void Database::Close()
{
    if (m_Db)
    {
        sqlite3_close(m_Db);
        m_Db = nullptr;
    }
}

bool Database::Migrate()
{
    if (!m_Db)
    {
        return false;
    }

    struct Migration
    {
        int version;
        std::string_view sql;
    };

    // Version 1: initial schema.
    static constexpr std::string_view kSchemaV1 = R"SQL(
CREATE TABLE IF NOT EXISTS songs (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    path         TEXT NOT NULL UNIQUE,
    title        TEXT NOT NULL DEFAULT '',
    artist       TEXT NOT NULL DEFAULT '',
    album        TEXT NOT NULL DEFAULT '',
    album_artist TEXT NOT NULL DEFAULT '',
    genre        TEXT NOT NULL DEFAULT '',
    folder       TEXT NOT NULL DEFAULT '',
    codec        TEXT NOT NULL DEFAULT '',
    track_no     INTEGER NOT NULL DEFAULT 0,
    disc_no      INTEGER NOT NULL DEFAULT 0,
    year         INTEGER NOT NULL DEFAULT 0,
    channels     INTEGER NOT NULL DEFAULT 0,
    sample_rate  INTEGER NOT NULL DEFAULT 0,
    bitrate      INTEGER NOT NULL DEFAULT 0,
    duration     REAL    NOT NULL DEFAULT 0,
    file_size    INTEGER NOT NULL DEFAULT 0,
    file_mtime   INTEGER NOT NULL DEFAULT 0,
    date_added   INTEGER NOT NULL DEFAULT 0,
    date_played  INTEGER NOT NULL DEFAULT 0,
    play_count   INTEGER NOT NULL DEFAULT 0,
    favorite     INTEGER NOT NULL DEFAULT 0,
    has_art      INTEGER NOT NULL DEFAULT 0,
    art_path     TEXT    NOT NULL DEFAULT ''
);
CREATE INDEX IF NOT EXISTS idx_songs_artist     ON songs(artist);
CREATE INDEX IF NOT EXISTS idx_songs_album      ON songs(album);
CREATE INDEX IF NOT EXISTS idx_songs_genre      ON songs(genre);
CREATE INDEX IF NOT EXISTS idx_songs_folder     ON songs(folder);
CREATE INDEX IF NOT EXISTS idx_songs_favorite   ON songs(favorite);
CREATE INDEX IF NOT EXISTS idx_songs_played     ON songs(date_played);
CREATE INDEX IF NOT EXISTS idx_songs_added      ON songs(date_added);

CREATE TABLE IF NOT EXISTS albums (
    id       INTEGER PRIMARY KEY AUTOINCREMENT,
    name     TEXT NOT NULL DEFAULT '',
    artist   TEXT NOT NULL DEFAULT '',
    art_path TEXT NOT NULL DEFAULT '',
    year     INTEGER NOT NULL DEFAULT 0,
    UNIQUE(name, artist)
);
CREATE INDEX IF NOT EXISTS idx_albums_artist ON albums(artist);

CREATE TABLE IF NOT EXISTS artists (
    id   INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS playlists (
    id      INTEGER PRIMARY KEY AUTOINCREMENT,
    name    TEXT NOT NULL UNIQUE,
    created INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS playlist_tracks (
    playlist_id INTEGER NOT NULL,
    position    INTEGER NOT NULL,
    song_id     INTEGER NOT NULL,
    PRIMARY KEY (playlist_id, position),
    FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE,
    FOREIGN KEY (song_id)     REFERENCES songs(id)     ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS settings (
    key   TEXT PRIMARY KEY,
    value TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS play_history (
    song_id   INTEGER NOT NULL,
    played_at INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_history_played ON play_history(played_at);
)SQL";

    static const std::vector<Migration> kMigrations = {
        {1, kSchemaV1},
    };

    std::int64_t version = 0;
    {
        Statement stmt = Prepare("PRAGMA user_version;");
        if (stmt.Step() == SQLITE_ROW)
        {
            version = stmt.ColumnInt64(0);
        }
    }

    for (const auto& migration : kMigrations)
    {
        if (migration.version <= version)
        {
            continue;
        }
        flachead::core::Logger::Info("Applying database migration v" + std::to_string(migration.version));
        if (!Execute(migration.sql))
        {
            return false;
        }
        // PRAGMA does not support bound parameters; build the statement.
        Execute("PRAGMA user_version = " + std::to_string(migration.version) + ";");
    }

    return true;
}

bool Database::Execute(std::string_view sql)
{
    if (!m_Db)
    {
        return false;
    }

    char* error = nullptr;
    const int rc = sqlite3_exec(m_Db, std::string{sql}.c_str(), nullptr, nullptr, &error);
    if (rc != SQLITE_OK)
    {
        flachead::core::Logger::Error("SQL exec failed: " + std::string{error ? error : "unknown"});
        sqlite3_free(error);
        return false;
    }
    return true;
}

Statement Database::Prepare(std::string_view sql)
{
    return Statement{m_Db, sql};
}

std::int64_t Database::LastInsertRowId() const
{
    return m_Db ? sqlite3_last_insert_rowid(m_Db) : 0;
}
} // namespace flachead::database
