#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>
#include <vector>

int g_failures = 0;
int g_checks = 0;

void Check(bool condition, const char* what)
{
    ++g_checks;
    if (!condition)
    {
        ++g_failures;
        std::printf("FAIL: %s\n", what);
    }
}

void RunTest(const char* name, std::function<void()> body)
{
    std::printf("== %s\n", name);
    body();
}

int Finish()
{
    std::printf("== %d/%d checks passed\n", g_checks - g_failures, g_checks);
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
