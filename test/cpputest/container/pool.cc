#include "container/Pool.h"

#include <iostream>
#include <vector>
#include <CppUTest/TestHarness.h>
#include <CppUTest/MemoryLeakDetectorNewMacros.h>
#include <CppUTest/MemoryLeakDetectorMallocMacros.h>

#include <string>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

using namespace container;

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
        stringVector.push_back(stringPool->pop());
    }

    for (auto str : stringVector) {
        stringPool->push(str);
    }

    delete stringPool;
}

TEST(PoolInitialization, max_elements)
{
    const size_t maxElements = 10;
    Pool<std::string, maxElements> stringPool;
    std::vector<std::string *> stringVector;

    for (size_t i = 0; i < maxElements; ++i) {
        stringVector.push_back(stringPool.pop());
    }

    auto stringPtr = stringPool.pop();
    POINTERS_EQUAL(nullptr, stringPtr);

    stringPool.push(stringVector.back());
    stringPtr = stringPool.pop();
    CHECK(stringPtr != nullptr);

    for (auto str : stringVector) {
        stringPool.push(str);
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
    auto strPtr = stringPool->pop("foo");
    STRCMP_EQUAL("foo", strPtr->c_str());
    stringPool->push(strPtr);

    strPtr = stringPool->pop("bar");
    STRCMP_EQUAL("bar", strPtr->c_str());
    stringPool->push(strPtr);

    std::string baz("baz");
    strPtr = stringPool->pop(baz);
    STRCMP_EQUAL("baz", strPtr->c_str());
    stringPool->push(strPtr);
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
        ptrs[i] = stringPool->pop();
    }

    auto nElements = 0;
    for (auto it = stringPool->begin(); it != stringPool->end(); ++it) {
        nElements++;
    }

    for (size_t i = 0; i < maxElements; ++i) {
        stringPool->push(ptrs[i]);
    }

    CHECK_EQUAL(maxElements, nElements);
}

TEST(Pool, iterate_no_sequential)
{
    auto str1 = stringPool->pop();
    auto str2 = stringPool->pop();
    auto str3 = stringPool->pop();
    auto str4 = stringPool->pop();
    auto str5 = stringPool->pop();
    auto str6 = stringPool->pop();
    auto str7 = stringPool->pop();
    auto str8 = stringPool->pop();

    stringPool->push(str3);
    stringPool->push(str7);

    auto nElements = 0;
    for (auto it = stringPool->begin(); it != stringPool->end(); ++it) {
        nElements++;
    }

    CHECK_EQUAL(6, nElements);

    stringPool->push(str1);
    stringPool->push(str2);
    stringPool->push(str4);
    stringPool->push(str5);
    stringPool->push(str6);
    stringPool->push(str8);

    nElements = 0;
    for (auto it = stringPool->begin(); it != stringPool->end(); ++it) {
        nElements++;
    }

    CHECK_EQUAL(0, nElements);

}

TEST(Pool, iterate_operators)
{
    auto ptr = stringPool->pop("foo");
    auto it = stringPool->begin();

    STRCMP_EQUAL("foo", it->c_str());

    stringPool->push(ptr);
}
