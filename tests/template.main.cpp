#include <stdio.h>
#include <string.h>
#include <mintpack/timewaster.h>
#include <mintsystem/timer.h>


typedef bool TestEntryFunc(int numThreads);

struct TestInfo
{
    const char* description;
    TestEntryFunc* func;
    int numThreads;
    bool allowFailure;
};

${TEST_ENTRY_FUNCS}

TestInfo g_testInfos[] =
{
${TEST_INFOS}
};

struct TestGroup
{
    int numTests;
    int numPassed;

    TestGroup()
    {
        numTests = 0;
        numPassed = 0;
    }
    void addResult(bool success)
    {
        if (success)
            numPassed++;
        numTests++;
    }
};

/* Base 10 number of digits */
int digits_in(int n)
{
    int c = 0;
    do {
        c += 1;
        n /= 10;
    } while (n);
    return c;
}

int main()
{
    mint_timer_initialize();
    TimeWaster::Initialize();

    TestGroup required, optional;
    int numTests = sizeof(g_testInfos) / sizeof(g_testInfos[0]);
    int digits = digits_in(numTests),
        desclen = 0;

    for (int i = 0; i < numTests; i++)
    {
        int length = strlen(g_testInfos[i].description);
        if (length > desclen)
            desclen = length;
    }

    for (int i = 0; i < numTests; i++)
    {
        TestInfo& info = g_testInfos[i];
        printf("[%*d/%*d] %-*s",
            digits, i + 1,
            digits, numTests,
            desclen, info.description);

        mint_timer_tick_t start = mint_timer_get();
        bool success = info.func(info.numThreads);
        mint_timer_tick_t end = mint_timer_get();

        const char* status = success ? "ok" : info.allowFailure ? "xfail" : "fail";
        printf(" %5s, %8.2f ms\n", status, (end - start) * mint_timer_ticksToSeconds * 1000);

        TestGroup& group = info.allowFailure ? optional : required;
        group.addResult(success);
    }

    printf("\n----- Summary: -----\n");
    printf("Out of the tests required to pass, %d/%d passed.\n", required.numPassed, required.numTests);
    printf("Out of the tests allowed to fail, %d/%d passed.\n", optional.numPassed, optional.numTests);

    return (required.numPassed == required.numTests) ? 0 : 1;
}
