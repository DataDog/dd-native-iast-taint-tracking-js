#include "iast_manager.h"
#include <CppUTest/UtestMacros.h>
#include <CppUTest/TestHarness.h>



using namespace iast;
using namespace iast::container;

struct FakeTransaction final {
    iast_key_t _id;
    FakeTransaction() { _id = 0; }
    FakeTransaction(iast_key_t id): _id(id) { };
    FakeTransaction(const FakeTransaction& other) {
        _id = other._id;
    }
    void setId(iast_key_t id) { _id = id; }
    iast_key_t getId() { return _id; }
    void clean(void) { return; }
};

TEST_GROUP(IastManager)
{
    void setup() {
        //Since the object tested is a singleton the framework detects a leak
        //because the object is cleared after leakage detection
        MemoryLeakWarningPlugin::saveAndDisableNewDeleteOverloads();
        IastManager<FakeTransaction>::GetInstance().Clear();
    }
    void teardown() {
        MemoryLeakWarningPlugin::restoreNewDeleteOverloads();
    }
};

TEST(IastManager, initialization)
{
    int elems = 0;
    elems = IastManager<FakeTransaction>::GetInstance().getMaxItems();
    CHECK_EQUAL(2, elems);

    IastManager<FakeTransaction>::GetInstance().setMaxItems(4);

    elems = IastManager<FakeTransaction>::GetInstance().getMaxItems();
    CHECK_EQUAL(4, elems);

    elems = IastManager<FakeTransaction>::GetInstance().Size();
    CHECK_EQUAL(0, elems);
}



TEST(IastManager, new_item)
{
    FakeTransaction* ptr = nullptr;
    size_t elems = 0;
    iast_key_t key;

    key = 1;
    IastManager<FakeTransaction>::GetInstance().New(key);
    ptr = IastManager<FakeTransaction>::GetInstance().Get(key);
    CHECK(ptr != nullptr);

    elems = IastManager<FakeTransaction>::GetInstance().Size();
    CHECK_EQUAL(1, elems);
}

TEST(IastManager, item_reused)
{
    FakeTransaction* ptr = nullptr;
    FakeTransaction* ptr2 = nullptr;
    size_t elems = 0;
    iast_key_t key;

    key = 2;
    IastManager<FakeTransaction>::GetInstance().New(key);
    ptr = IastManager<FakeTransaction>::GetInstance().Get(key);
    CHECK(ptr != nullptr);

    IastManager<FakeTransaction>::GetInstance().Remove(key);
    elems = IastManager<FakeTransaction>::GetInstance().Size();
    CHECK_EQUAL(0, elems);

    key = 3;
    IastManager<FakeTransaction>::GetInstance().New(key);
    ptr2 = IastManager<FakeTransaction>::GetInstance().Get(key);
    POINTERS_EQUAL(ptr2, ptr);
    IastManager<FakeTransaction>::GetInstance().Remove(key);

    elems = IastManager<FakeTransaction>::GetInstance().Size();
    CHECK_EQUAL(0, elems);
}


TEST(IastManager, insert_beyond_limit)
{
    FakeTransaction* ptr = nullptr;
    FakeTransaction* ptr2 = nullptr;
    FakeTransaction* ptr3 = nullptr;
    size_t elems = 0;
    iast_key_t key1, key2, key3;

    elems = IastManager<FakeTransaction>::GetInstance().Size();
    CHECK_EQUAL(0, elems);

    key1 = 1;
    IastManager<FakeTransaction>::GetInstance().New(key1);
    ptr = IastManager<FakeTransaction>::GetInstance().Get(key1);
    CHECK(ptr != nullptr);

    key2 = 2;
    IastManager<FakeTransaction>::GetInstance().New(key2);
    ptr2 = IastManager<FakeTransaction>::GetInstance().Get(key2);
    CHECK(ptr != nullptr);

    key3 = 3;
    IastManager<FakeTransaction>::GetInstance().New(key3);
    ptr3 = IastManager<FakeTransaction>::GetInstance().Get(key3);
    CHECK(ptr3 == nullptr);
}
