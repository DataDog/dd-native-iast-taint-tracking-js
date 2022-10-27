/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include "container/pool.h"

#include <CppUTest/TestFailure.h>
#include <CppUTest/UtestMacros.h>
#include <exception>
#include <iostream>
#include <vector>
#include <CppUTest/TestHarness.h>

#include <string>

#ifndef CHECK_THROWS
#define CHECK_THROWS(expected, expression) \
do { \
    std::string failure_msg("expected to throw "#expected "\nbut threw nothing"); \
    bool caught_expected = false; \
    try { \
        (expression); \
    } catch(const expected &) { \
        caught_expected = true; \
    } catch(...) { \
        failure_msg = "expected to throw " #expected "\nbut threw a different type"; \
    } \
    CHECK_TEXT(caught_expected, failure_msg); \
} while(0)
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

using namespace iast::container;

TEST_GROUP(PoolInitialization)
{
    void setup() {}
    void teardown() {}
};

TEST(PoolInitialization, normal_behavior)
{
    const size_t maxElements = 10;
    auto stringPool = new Pool<std::string, maxElements>();

    std::vector<std::string *> stringVector;
    for (size_t i = 0; i < maxElements; ++i) {
        stringVector.push_back(stringPool->Pop());
    }

    for (auto str : stringVector) {
        stringPool->Push(str);
    }

    delete stringPool;
}

TEST(PoolInitialization, max_elements)
{
    const size_t maxElements = 10;
    Pool<std::string, maxElements> stringPool;
    std::vector<std::string *> stringVector;

    for (size_t i = 0; i < maxElements; ++i) {
        stringVector.push_back(stringPool.Pop());
    }

    CHECK_THROWS(PoolBadAlloc, stringPool.Pop());

    stringPool.Push(stringVector.back());
    auto stringPtr = stringPool.Pop();
    CHECK(stringPtr != nullptr);

    for (auto str : stringVector) {
        stringPool.Push(str);
    }
}

TEST_GROUP(Pool)
{
    static const size_t maxElements = 100;
    Pool<std::string, maxElements>* stringPool;

    void setup() {
        stringPool = new Pool<std::string, maxElements>();
    }
    void teardown() {
        delete stringPool;
    }
};

TEST(Pool, build_parameters)
{
    auto strPtr = stringPool->Pop("foo");
    STRCMP_EQUAL("foo", strPtr->c_str());
    stringPool->Push(strPtr);

    strPtr = stringPool->Pop("bar");
    STRCMP_EQUAL("bar", strPtr->c_str());
    stringPool->Push(strPtr);

    std::string baz("baz");
    strPtr = stringPool->Pop(baz);
    STRCMP_EQUAL("baz", strPtr->c_str());
    stringPool->Push(strPtr);
}

TEST(Pool, iterate_zero_used_elements)
{
    auto elements = 0;
    for (auto it = stringPool->begin(); it != stringPool->end(); ++it) {
        elements++;
    }
    CHECK_EQUAL(0, elements);
}

TEST(Pool, iterate)
{
    const size_t maxElements = 50;
    std::string* ptrs[maxElements] = {};
    for (size_t i = 0; i < maxElements; ++i) {
        ptrs[i] = stringPool->Pop();
    }

    auto nElements = 0;
    for (auto it = stringPool->begin(); it != stringPool->end(); ++it) {
        nElements++;
    }

    for (size_t i = 0; i < maxElements; ++i) {
        stringPool->Push(ptrs[i]);
    }

    CHECK_EQUAL(maxElements, nElements);
}

TEST(Pool, iterate_no_sequential)
{
    auto str1 = stringPool->Pop();
    auto str2 = stringPool->Pop();
    auto str3 = stringPool->Pop();
    auto str4 = stringPool->Pop();
    auto str5 = stringPool->Pop();
    auto str6 = stringPool->Pop();
    auto str7 = stringPool->Pop();
    auto str8 = stringPool->Pop();

    stringPool->Push(str3);
    stringPool->Push(str7);

    auto nElements = 0;
    for (auto it = stringPool->begin(); it != stringPool->end(); ++it) {
        nElements++;
    }

    CHECK_EQUAL(6, nElements);

    stringPool->Push(str1);
    stringPool->Push(str2);
    stringPool->Push(str4);
    stringPool->Push(str5);
    stringPool->Push(str6);
    stringPool->Push(str8);

    nElements = 0;
    for (auto it = stringPool->begin(); it != stringPool->end(); ++it) {
        nElements++;
    }

    CHECK_EQUAL(0, nElements);

}

TEST(Pool, iterate_operators)
{
    auto ptr = stringPool->Pop("foo");
    auto it = stringPool->begin();

    STRCMP_EQUAL("foo", it->c_str());

    stringPool->Push(ptr);
}

TEST(Pool, empty)
{
    auto begin = stringPool->begin();
    auto end = stringPool->end();

    CHECK(begin == end);
}

TEST(Pool, double_clear)
{
    auto ptr = stringPool->Pop("foo");
    auto it = stringPool->begin();

    STRCMP_EQUAL("foo", it->c_str());

    stringPool->Clear();
    it = stringPool->begin();
    POINTERS_EQUAL(nullptr, *it);
    stringPool->Clear();

    ptr = stringPool->Pop("bar");
    it = stringPool->begin();
    STRCMP_EQUAL("bar", it->c_str());
}
