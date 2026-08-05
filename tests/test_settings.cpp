#include "../src/services/SettingsManager.hpp"
#include "../src/database/Database.hpp"
#include "../src/events/EventBus.hpp"
#include "test_util.hpp"

#include <cstdio>
#include <string>
#include <vector>

using flachead::database::Database;
using flachead::events::EventBus;
using flachead::services::SettingsManager;

int main()
{
    std::remove("/tmp/flachead_settings_test.db");

    Database db;
    Check(db.Open("/tmp/flachead_settings_test.db"), "open settings db");

    EventBus bus;
    int changedCount = 0;
    std::string changedKey;
    bus.Subscribe(flachead::events::Type::SettingsChanged,
                  [&](const flachead::events::Event& event) {
                      ++changedCount;
                      changedKey = event.stringValue;
                  });

    RunTest("set and get strings", [&] {
        SettingsManager settings;
        settings.Initialize(db, bus);

        Check(settings.Get("theme", "dark") == "dark", "default used when absent");
        settings.Set("theme", "light");
        Check(settings.Get("theme") == "light", "value round trip");
        Check(changedCount == 1 && changedKey == "theme", "change event published");
    });

    RunTest("typed accessors", [&] {
        SettingsManager settings;
        settings.Initialize(db, bus);

        settings.SetInt("volume", 80);
        Check(settings.GetInt("volume", -1) == 80, "int round trip");
        Check(settings.GetInt("missing", 5) == 5, "int default");

        settings.SetFloat("balance", 0.25f);
        Check(settings.GetFloat("balance", -1.0f) > 0.24f &&
                  settings.GetFloat("balance") < 0.26f,
              "float round trip");

        settings.SetBool("shuffle", true);
        Check(settings.GetBool("shuffle", false), "bool round trip true");
        Check(settings.GetBool("missing", true), "bool default");
    });

    RunTest("persistence across reload", [&] {
        SettingsManager first;
        first.Initialize(db, bus);
        first.Set("theme", "light");
        first.SetInt("volume", 80);

        // A fresh manager on the same database must see the persisted values.
        SettingsManager second;
        second.Initialize(db, bus);
        Check(second.Get("theme") == "light", "string persisted");
        Check(second.GetInt("volume", -1) == 80, "int persisted");
        Check(second.GetFloat("missing", 3.5f) == 3.5f, "missing float default");
    });

    RunTest("delete", [&] {
        SettingsManager settings;
        settings.Initialize(db, bus);
        settings.Set("temporary", "value");
        Check(settings.Has("temporary"), "key present");
        settings.Delete("temporary");
        Check(!settings.Has("temporary"), "key removed from cache");

        SettingsManager reloaded;
        reloaded.Initialize(db, bus);
        Check(!reloaded.Has("temporary"), "key removed from database");
    });

    RunTest("memory-only without database", [&] {
        SettingsManager settings;
        settings.Set("volatile", "yes");
        Check(settings.Get("volatile") == "yes", "works without db");
        Check(settings.Keys().size() == 1, "keys enumerated");
    });

    db.Close();
    return Finish();
}
