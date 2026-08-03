#include "../src/database/Database.hpp"
#include "../src/core/PathUtils.hpp"
#include "test_util.hpp"

#include <sqlite3.h>

#include <cstdio>
#include <filesystem>

using flachead::database::Database;
using flachead::database::Statement;

int main()
{
    const std::string dbPath = "/tmp/flachead_test.db";
    std::filesystem::remove(dbPath);
    std::filesystem::remove(dbPath + "-wal");
    std::filesystem::remove(dbPath + "-shm");

    RunTest("database open + migrate", [&] {
        Database db;
        Check(db.Open(dbPath), "open creates schema");
        Check(db.Execute("INSERT INTO songs (path, title, artist, album) VALUES ('/a/1.flac','One','Artist','Album')"),
              "insert song");
        Check(db.Execute("INSERT INTO artists (name) VALUES ('Artist')"), "insert artist");
        Check(db.Execute("INSERT INTO settings (key, value) VALUES ('volume','0.7')"), "insert setting");
    });

    RunTest("database query + types", [&] {
        Database db;
        Check(db.Open(dbPath), "reopen same file");

        Statement stmt = db.Prepare("SELECT id, path, title FROM songs WHERE artist = ?;");
        stmt.Bind(1, "Artist");
        Check(stmt.Step() == SQLITE_ROW, "row available");
        Check(stmt.ColumnInt64(0) > 0, "auto id assigned");
        Check(stmt.ColumnText(1) == "/a/1.flac", "path roundtrip");
        Check(stmt.ColumnText(2) == "One", "title roundtrip");
        Check(stmt.Step() == SQLITE_DONE, "no more rows");
    });

    RunTest("database transaction rollback", [&] {
        Database db;
        Check(db.Open(dbPath), "open again");

        {
            auto txn = flachead::database::Transaction{db.Handle()};
            db.Execute("INSERT INTO songs (path, title) VALUES ('/a/2.flac','Two')");
            // no commit -> rollback on scope exit
        }

        Statement count = db.Prepare("SELECT COUNT(*) FROM songs WHERE title='Two';");
        count.Step();
        Check(count.ColumnInt(0) == 0, "rollback discards uncommitted rows");
    });

    RunTest("path utils", [&] {
        Check(flachead::core::path::Extension("/x/y.flac") == ".flac", "extension lowercased");
        Check(flachead::core::path::BaseName("/x/y/Album.flac") == "Album.flac", "base name");
        Check(flachead::core::path::Stem("/x/y/Album.flac") == "Album", "stem");
        Check(flachead::core::path::ParentDirectory("/x/y/Album.flac") == "/x/y", "parent dir");
        Check(flachead::core::path::IsAudioFile("song.MP3"), "uppercase ext");
        Check(!flachead::core::path::IsAudioFile("song.txt"), "not audio");
        Check(flachead::core::path::IsImageFile("cover.JPG"), "image ext");
        Check(flachead::core::path::SanitizeSegment("a/b:c") == "a_b_c", "sanitize");
    });

    return Finish();
}
