/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <CppUTest/UtestMacros.h>
#include <CppUTest/TestHarness.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include <string>
#include <iostream>
#include "weakiface.h"

using namespace iast;

struct Point {
    int _x;
    int _y;
};

struct DerivedPersistent : public WeakObjIface<DerivedPersistent>
{
    Point* _target;
    DerivedPersistent() { _target = nullptr; }
    DerivedPersistent(Point* p): _target(p){}
    ~DerivedPersistent() { _target = nullptr; }
    bool IsEmpty() { return _target == nullptr; }
    Point* Get(int val) {
        std::cout << "_val: " << val  << std::endl;
        return _target;
    }

};

struct FalseNull : public WeakObjIface<FalseNull>
{
    bool IsEmpty() { return false; }
    uintptr_t Get() { return 0; }
};

struct TrueNotNull : public WeakObjIface<TrueNotNull>
{
    bool IsEmpty() { return true; }
    uintptr_t Get() { return 1; }
};

TEST_GROUP(WeakObjIface)
{
    void setup() {}
    void teardown() {}

    template<typename T>
    bool iface_is_empty_call(WeakObjIface<T>& obj)
    {
        return obj.IsEmpty();
    }
};

TEST(WeakObjIface, Initialization)
{
    FalseNull falseObj{};
    TrueNotNull trueObj{};

    POINTERS_EQUAL(nullptr, falseObj._next);
    CHECK_EQUAL(false, falseObj.IsEmpty());
    CHECK_EQUAL(0, falseObj.Get());

    POINTERS_EQUAL(nullptr, trueObj._next);
    CHECK_EQUAL(true, trueObj.IsEmpty());
    CHECK_EQUAL(1, trueObj.Get());
}

TEST(WeakObjIface, Next)
{
    FalseNull falseObj{};
    FalseNull falseObj2{};

    POINTERS_EQUAL(nullptr, falseObj._next);

    falseObj._next = &falseObj2;
    CHECK_EQUAL(&falseObj2, falseObj._next);
    CHECK_EQUAL(false, falseObj._next->IsEmpty());
    CHECK_EQUAL(0, falseObj._next->Get());
}

TEST(WeakObjIface, CheckInterface)
{
    FalseNull falseObj{};
    TrueNotNull trueObj{};

    auto ret = iface_is_empty_call(falseObj);
    CHECK_EQUAL(false, ret);

    ret = iface_is_empty_call(trueObj);
    CHECK_EQUAL(true, ret);
}

