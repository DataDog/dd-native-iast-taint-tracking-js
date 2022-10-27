/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <CppUTest/UtestMacros.h>
#include <CppUTest/TestHarness.h>
#include <cstdint>

#include "weakiface.h"
#include "container/weakmap.h"


using namespace iast;
using namespace iast::container;

class FakeRef : public WeakObjIface<FakeRef*> {
    public:
        FakeRef() { }
        FakeRef(int id, uintptr_t internal): _id(id) { _key = internal; _target = reinterpret_cast<void*>(internal);}
        int getId() { return _id;}
        bool IsEmpty() { return _target == nullptr; }
        weak_key_t Get() { return reinterpret_cast<uintptr_t>(_target); }
        void setNewInternal(uintptr_t target) { _target = reinterpret_cast<void*>(target); }
    private:
        int _id;
        void* _target;
};

TEST_GROUP(WeakMap)
{
    void setup() {};
    void teardown() {};
};

TEST(WeakMap, insert)
{
    int ret = 0;
    WeakMap<FakeRef*, 1> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    ret = wMap.Insert(f->Get(), f);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);
}

TEST(WeakMap, insert_nullptr)
{
    int ret = WEAK_MAP_SUCCESS;
    WeakMap<FakeRef*, 1> wMap{};

    ret = wMap.Insert(1, nullptr);
    CHECK_EQUAL(WEAK_MAP_INVALID_ARG, ret);

}

TEST(WeakMap, insert_beyond_limit)
{
    int ret = WEAK_MAP_SUCCESS;
    WeakMap<FakeRef*, 1> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    ret = wMap.Insert(f->Get(), f);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    FakeRef *f2 = new FakeRef(101, 1);
    ret = wMap.Insert(f->Get(), f2);
    CHECK_EQUAL(WEAK_MAP_MAX_ELEM, ret);
    delete f2;
}

TEST(WeakMap, find)
{
    int ret;
    WeakMap<FakeRef*, 2> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    ret = wMap.Insert(f->Get(), f);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    FakeRef* found = wMap.Find(1);
    CHECK(found != nullptr);
    CHECK_EQUAL(1, found->_key);
    CHECK_EQUAL(100, found->getId());
}

TEST(WeakMap, find_not_inserted)
{
    int ret;
    WeakMap<FakeRef*, 1> wMap{};

    FakeRef *found = wMap.Find(1);
    POINTERS_EQUAL(nullptr, found);

    FakeRef *f = new FakeRef(100, 2);

    ret = wMap.Insert(f->Get(), f);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);
    found = wMap.Find(1);
    POINTERS_EQUAL(nullptr, found);

    wMap.Del(f->Get());
    found = wMap.Find(2);
    POINTERS_EQUAL(nullptr, found);
}

TEST(WeakMap, find_deleted)
{
    int ret;
    WeakMap<FakeRef*, 2> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    ret = wMap.Insert(f->Get(), f);

    wMap.Del(f->Get());
    FakeRef* found = wMap.Find(1);
    POINTERS_EQUAL(nullptr, found);
}

TEST(WeakMap, delete_root)
{
    int ret;
    WeakMap<FakeRef*, 2> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    weak_key_t f_addr = f->Get();
    ret = wMap.Insert(f_addr, f);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    wMap.Del(f_addr);
    FakeRef *found = wMap.Find(f_addr);
    POINTERS_EQUAL(nullptr, found);
}

TEST(WeakMap, delete_last)
{
    int ret;
    WeakMap<FakeRef*, 2> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    FakeRef *f2 = new FakeRef(101, 2);

    weak_key_t f_addr = f->Get();
    weak_key_t f2_addr = f2->Get();

    ret = wMap.Insert(f_addr, f);
    ret = wMap.Insert(f2_addr, f2);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    wMap.Del(f_addr);
    FakeRef *found = wMap.Find(f_addr);
    POINTERS_EQUAL(nullptr, found);

    found = wMap.Find(f2_addr);
    CHECK(found != nullptr);
}

TEST(WeakMap, delete_second)
{
    int ret;
    WeakMap<FakeRef*, 10> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    FakeRef *f2 = new FakeRef(101, 2);
    FakeRef *f3 = new FakeRef(102, 3);

    weak_key_t f_addr = f->Get();
    weak_key_t f2_addr = f2->Get();
    weak_key_t f3_addr = f3->Get();

    ret = wMap.Insert(f_addr, f);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);
    ret = wMap.Insert(f2_addr, f2);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);
    ret = wMap.Insert(f3_addr, f3);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    wMap.Del(f2_addr);
    FakeRef *found = wMap.Find(f2_addr);
    POINTERS_EQUAL(nullptr, found);

}

TEST(WeakMap, delete_middle)
{
    int ret;
    WeakMap<FakeRef*, 10> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    FakeRef *f2 = new FakeRef(101, 2);
    FakeRef *f3 = new FakeRef(102, 3);
    FakeRef *f4 = new FakeRef(103, 4);

    weak_key_t f_addr = f->Get();
    weak_key_t f2_addr = f2->Get();
    weak_key_t f3_addr = f3->Get();
    weak_key_t f4_addr = f4->Get();


    ret = wMap.Insert(f_addr, f);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);
    ret = wMap.Insert(f2_addr, f2);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);
    ret = wMap.Insert(f3_addr, f3);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);
    ret = wMap.Insert(f4_addr, f4);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    wMap.Del(f2_addr);
    FakeRef *found = wMap.Find(f2_addr);
    POINTERS_EQUAL(nullptr, found);

}

TEST(WeakMap, rehash_move)
{
    int ret;
    WeakMap<FakeRef*, 10> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    ret = wMap.Insert(f->Get(), f);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    CHECK_EQUAL(f->_key, f->Get());

    f->setNewInternal(3);
    CHECK(f->_key != f->Get());

    wMap.Rehash();
    CHECK(f->_key == f->Get());

}

TEST(WeakMap, rehash_delete)
{
    int ret;
    WeakMap<FakeRef*, 10> wMap{};
    FakeRef *f = new FakeRef(100, 1);

    ret = wMap.Insert(f->Get(), f);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    FakeRef *found = wMap.Find(1);
    CHECK(found != nullptr);

    CHECK_EQUAL(f->_key, f->Get());
    f->setNewInternal(0);
    CHECK_EQUAL(true, f->IsEmpty());

    //TODO: there is no other way to check that rehash method is working but to expose
    //getCount method. find returns nullptr for either Empty objects or no object at all.
    ret = wMap.GetCount();
    CHECK_EQUAL(1, ret);

    wMap.Rehash();
    found = wMap.Find(1);
    CHECK(found == nullptr);

    ret = wMap.GetCount();
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    // Right now rehash only removes the reference from the map so the destruction needs to
    // happen outside.
    delete f;
}

TEST(WeakMap, clear_empty)
{
    int ret;
    WeakMap<FakeRef*, 1> wMap{};

    ret = wMap.GetCount();
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    wMap.Clean();

    ret = wMap.GetCount();
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);
}

TEST(WeakMap, clear)
{
    int ret;
    WeakMap<FakeRef*, 3> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    FakeRef *f2 = new FakeRef(101, 2);
    FakeRef *f3 = new FakeRef(102, 3);

    ret = wMap.Insert(f->Get(), f);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    ret = wMap.Insert(f2->Get(), f2);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    ret = wMap.Insert(f3->Get(), f3);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    ret = wMap.GetCount();
    CHECK_EQUAL(3, ret);

    wMap.Clean();

    ret = wMap.GetCount();
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    // Right now clean only removes the reference from the map so the destruction needs to
    // happen outside.
    delete f;
    delete f2;
    delete f3;
}
