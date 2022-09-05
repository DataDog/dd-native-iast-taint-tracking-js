#include <CppUTest/UtestMacros.h>
#include <CppUTest/TestHarness.h>
#include <CppUTest/MemoryLeakDetectorNewMacros.h>
#include <CppUTest/MemoryLeakDetectorMallocMacros.h>
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
    ret = wMap.insert(f->Get(), f);
    CHECK_EQUAL(0, ret);
}

TEST(WeakMap, insert_nullptr)
{
    int ret = 0;
    WeakMap<FakeRef*, 1> wMap{};

    ret = wMap.insert(1, nullptr);
    CHECK_EQUAL(WEAK_MAP_INVALID_ARG, ret);

}

TEST(WeakMap, insert_beyond_limit)
{
    int ret = 0;
    WeakMap<FakeRef*, 1> wMap{};
    
    FakeRef *f = new FakeRef(100, 1);
    ret = wMap.insert(f->Get(), f);
    CHECK_EQUAL(WEAK_MAP_SUCCESS, ret);

    FakeRef *f2 = new FakeRef(101, 1);
    ret = wMap.insert(f->Get(), f2);
    CHECK_EQUAL(WEAK_MAP_MAX_ELEM, ret);
    delete f2;
}

TEST(WeakMap, find)
{
    int ret;
    WeakMap<FakeRef*, 2> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    ret = wMap.insert(f->Get(), f);
    CHECK_EQUAL(0, ret);

    FakeRef* found = wMap.find(1);
    CHECK(found != nullptr);
    CHECK_EQUAL(1, found->_key);
    CHECK_EQUAL(100, found->getId());
}

TEST(WeakMap, find_not_inserted)
{
    int ret;
    WeakMap<FakeRef*, 1> wMap{};

    FakeRef *found = wMap.find(1);
    POINTERS_EQUAL(nullptr, found);

    FakeRef *f = new FakeRef(100, 2);

    ret = wMap.insert(f->Get(), f);
    CHECK_EQUAL(0, ret);
    found = wMap.find(1);
    POINTERS_EQUAL(nullptr, found);

    wMap.del(f->Get());
    found = wMap.find(2);
    POINTERS_EQUAL(nullptr, found);
}

TEST(WeakMap, find_deleted)
{
    int ret;
    WeakMap<FakeRef*, 2> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    ret = wMap.insert(f->Get(), f);

    wMap.del(f->Get());
    FakeRef* found = wMap.find(1);
    POINTERS_EQUAL(nullptr, found);
}

TEST(WeakMap, delete_root)
{
    int ret;
    WeakMap<FakeRef*, 2> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    ret = wMap.insert(f->Get(), f);
    CHECK_EQUAL(0, ret);

    wMap.del(f->Get());
    FakeRef *found = wMap.find(f->Get());
    POINTERS_EQUAL(nullptr, found);
}

TEST(WeakMap, delete_last)
{
    int ret;
    WeakMap<FakeRef*, 2> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    FakeRef *f2 = new FakeRef(101, 2);
    ret = wMap.insert(f->Get(), f);
    ret = wMap.insert(f2->Get(), f2);
    CHECK_EQUAL(0, ret);

    wMap.del(f->Get());
    FakeRef *found = wMap.find(f->Get());
    POINTERS_EQUAL(nullptr, found);

    found = wMap.find(f2->Get());
    CHECK(found != nullptr);

#ifdef IMPROVE_INSERT
    delete f;
#endif
}

TEST(WeakMap, delete_second)
{
    int ret;
    WeakMap<FakeRef*, 10> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    FakeRef *f2 = new FakeRef(101, 2);
    FakeRef *f3 = new FakeRef(102, 3);

    ret = wMap.insert(f->Get(), f);
    CHECK_EQUAL(0, ret);
    ret = wMap.insert(f2->Get(), f2);
    CHECK_EQUAL(0, ret);
    ret = wMap.insert(f3->Get(), f3);
    CHECK_EQUAL(0, ret);

    wMap.del(f2->Get());
    FakeRef *found = wMap.find(f2->Get());
    POINTERS_EQUAL(nullptr, found);

#ifdef IMPROVE_INSERT
    delete f;
    delete f2;
#endif

}

TEST(WeakMap, delete_middle)
{
    int ret;
    WeakMap<FakeRef*, 10> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    FakeRef *f2 = new FakeRef(101, 2);
    FakeRef *f3 = new FakeRef(102, 3);
    FakeRef *f4 = new FakeRef(103, 4);

    ret = wMap.insert(f->Get(), f);
    CHECK_EQUAL(0, ret);
    ret = wMap.insert(f2->Get(), f2);
    CHECK_EQUAL(0, ret);
    ret = wMap.insert(f3->Get(), f3);
    CHECK_EQUAL(0, ret);
    ret = wMap.insert(f4->Get(), f4);
    CHECK_EQUAL(0, ret);

    wMap.del(f2->Get());
    FakeRef *found = wMap.find(f2->Get());
    POINTERS_EQUAL(nullptr, found);

#ifdef IMPROVE_INSERT
    delete f;
    delete f2;
    delete f3;
#endif
}

TEST(WeakMap, rehash_move)
{
    int ret;
    WeakMap<FakeRef*, 10> wMap{};

    FakeRef *f = new FakeRef(100, 1);
    ret = wMap.insert(f->Get(), f);
    CHECK_EQUAL(0, ret);

    CHECK_EQUAL(f->_key, f->Get());

    f->setNewInternal(3);
    CHECK(f->_key != f->Get());

    wMap.rehash();
    CHECK(f->_key == f->Get());

}

TEST(WeakMap, rehash_delete)
{
    int ret;
    WeakMap<FakeRef*, 10> wMap{};
    FakeRef *f = new FakeRef(100, 1);

    ret = wMap.insert(f->Get(), f);
    CHECK_EQUAL(0, ret);

    FakeRef *found = wMap.find(1);
    CHECK(found != nullptr);

    CHECK_EQUAL(f->_key, f->Get());
    f->setNewInternal(0);
    CHECK_EQUAL(true, f->IsEmpty());

    //TODO: there is no other way to check that rehash method is working but to expose
    //getCount method. find returns nullptr for either Empty objects or no object at all.
    ret = wMap.getCount();
    CHECK_EQUAL(1, ret);

    wMap.rehash();
    found = wMap.find(1);
    CHECK(found == nullptr);

    ret = wMap.getCount();
    CHECK_EQUAL(0, ret);

    // Right now rehash only removes the reference from the map so the destruction needs to
    // happen outside.
    delete f;
}
