#include <CppUTest/UtestMacros.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <CppUTest/TestHarness.h>
#include <CppUTest/MemoryLeakDetectorNewMacros.h>
#include <CppUTest/MemoryLeakDetectorMallocMacros.h>

#include "container/queued_pool.h"

using namespace iast::container;

TEST_GROUP(QueuedPool)
{
    void setup() {}
    void teardown() {}
};

TEST(QueuedPool, pop)
{
    QueuedPool<std::string> strPool;

    std::string* ptr = strPool.pop();
    CHECK(ptr != nullptr);
    CHECK_EQUAL(1, strPool.size());

    std::string* ref = ptr;
    strPool.push(ptr);
    ptr = strPool.pop();
    POINTERS_EQUAL(ref, ptr);

    delete ptr;
}

TEST(QueuedPool, destruction)
{
    QueuedPool<std::string> strPool;

    std::vector<std::string*> v;
    for (int i = 0; i < 10; ++i) {
        std::string* ptr = strPool.pop();
        CHECK(ptr != nullptr);
        v.push_back(ptr);
    }

    for (std::string* i : v) {
        strPool.push(i);
    }

    CHECK_EQUAL(10, strPool.available());
    // At this point no leak should be triggered.
}

TEST(QueuedPool, clear)
{
    int max_items = 10;
    QueuedPool<std::string> strPool;

    CHECK_EQUAL(0, strPool.size());
    CHECK_EQUAL(0, strPool.available());

    std::vector<std::string*> v;
    for (int i = 0; i < max_items; ++i) {
        std::string* ptr = strPool.pop();
        CHECK(ptr != nullptr);
        v.push_back(ptr);
    }

    CHECK_EQUAL(10, strPool.size());
    CHECK_EQUAL(0, strPool.available());

    for (std::string* i : v) {
        strPool.push(i);
    }

    CHECK_EQUAL(10, strPool.size());
    CHECK_EQUAL(10, strPool.available());

    std::string* ptr = strPool.pop();
    CHECK_EQUAL(10, strPool.size());
    CHECK_EQUAL(9, strPool.available());

    strPool.push(ptr);
    strPool.clear();
    CHECK_EQUAL(0, strPool.available());
}
