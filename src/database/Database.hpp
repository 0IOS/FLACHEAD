#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

struct sqlite3;
struct sqlite3_stmt;

namespace flachead::database
{
// RAII wrapper around a single prepared SQLite statement.
class Statement
{
public:
    Statement() = default;
    explicit Statement(sqlite3* db, std::string_view sql);
    ~Statement();

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&& other) noexcept;

    bool Valid() const { return m_Stmt != nullptr; }

    bool Bind(int index, int value);
    bool Bind(int index, std::int64_t value);
    bool Bind(int index, double value);
    bool Bind(int index, std::string_view value);

    // Returns SQLITE_ROW (data available) or SQLITE_DONE (finished).
    int Step();

    // Column accessors. Index is 0-based.
    int                 ColumnInt(int index) const;
    std::int64_t        ColumnInt64(int index) const;
    double              ColumnDouble(int index) const;
    std::string         ColumnText(int index) const;
    bool                IsNull(int index) const;

    void Reset();
    void ClearBindings();

private:
    friend class Database;
    void Release();

    sqlite3* m_Db{nullptr};
    ::sqlite3_stmt* m_Stmt{nullptr};
};

// RAII transaction guard. Rolls back unless Commit() is called.
class Transaction
{
public:
    explicit Transaction(sqlite3* db);
    ~Transaction();

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    bool Commit();

private:
    sqlite3* m_Db{nullptr};
    bool m_Active{false};
};

class Database
{
public:
    Database() = default;
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Opens (or creates) the database file and runs pending migrations.
    bool Open(std::string_view path);
    void Close();

    bool IsOpen() const { return m_Db != nullptr; }

    // Applies all pending schema migrations. Returns false on failure.
    bool Migrate();

    // Runs a batch of SQL statements (separated by ';') with no parameters.
    bool Execute(std::string_view sql);

    // Creates a prepared statement on the current connection.
    Statement Prepare(std::string_view sql);

    std::int64_t LastInsertRowId() const;

    sqlite3* Handle() const { return m_Db; }

    std::string Path() const { return m_Path; }

private:
    bool ApplyMigration(int version, std::string_view sql);

    sqlite3* m_Db{nullptr};
    std::string m_Path;
};
} // namespace flachead::database
