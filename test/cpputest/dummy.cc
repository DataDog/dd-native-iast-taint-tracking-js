#include <CppUTest/TestHarness.h>
#include <CppUTest/MemoryLeakDetectorNewMacros.h>
#include <CppUTest/MemoryLeakDetectorMallocMacros.h>

#include <string>

TEST_GROUP(Dummy)
{
    void setup() {}
    void teardown() {}
};

TEST(Dummy, dummy_test_ok)
{
    CHECK_EQUAL(1, 1);
}

TEST(Dummy, dummy_test_ok2)
{
    CHECK_EQUAL(0, 0);
}
