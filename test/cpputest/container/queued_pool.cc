/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <CppUTest/UtestMacros.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <CppUTest/TestHarness.h>

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

    std::string* ptr = strPool.Pop();
    CHECK(ptr != nullptr);
    CHECK_EQUAL(1, strPool.Size());

    std::string* ref = ptr;
    strPool.Push(ptr);
    ptr = strPool.Pop();
    POINTERS_EQUAL(ref, ptr);

    delete ptr;
}

TEST(QueuedPool, destruction)
{
    QueuedPool<std::string> strPool;

    std::vector<std::string*> v;
    for (int i = 0; i < 10; ++i) {
        std::string* ptr = strPool.Pop();
        CHECK(ptr != nullptr);
        v.push_back(ptr);
    }

    for (std::string* i : v) {
        strPool.Push(i);
    }

    CHECK_EQUAL(10, strPool.Available());
    // At this point no leak should be triggered.
}

TEST(QueuedPool, clear)
{
    int max_items = 10;
    QueuedPool<std::string> strPool;

    CHECK_EQUAL(0, strPool.Size());
    CHECK_EQUAL(0, strPool.Available());

    std::vector<std::string*> v;
    for (int i = 0; i < max_items; ++i) {
        std::string* ptr = strPool.Pop();
        CHECK(ptr != nullptr);
        v.push_back(ptr);
    }

    CHECK_EQUAL(10, strPool.Size());
    CHECK_EQUAL(0, strPool.Available());

    for (std::string* i : v) {
        strPool.Push(i);
    }

    CHECK_EQUAL(10, strPool.Size());
    CHECK_EQUAL(10, strPool.Available());

    std::string* ptr = strPool.Pop();
    CHECK_EQUAL(10, strPool.Size());
    CHECK_EQUAL(9, strPool.Available());

    strPool.Push(ptr);
    strPool.Clear();
    CHECK_EQUAL(0, strPool.Available());
}
