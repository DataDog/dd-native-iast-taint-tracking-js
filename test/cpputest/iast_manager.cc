#include "iast.h"
#include <CppUTest/UtestMacros.h>
#include <CppUTest/TestHarness.h>
/* #include <CppUTest/MemoryLeakDetectorNewMacros.h> */
/* #include <CppUTest/MemoryLeakDetectorMallocMacros.h> */
//#include <cstdint>



using namespace iast;
using namespace iast::container;

TEST_GROUP(IastManager)
{
    void setup() {
        //Since the object tested is a singleton the framework detects a leak
        //because the object is cleared after detection
        MemoryLeakWarningPlugin::saveAndDisableNewDeleteOverloads();
        IastManager<int>::GetInstance().Clear();
    }
    void teardown() {
        MemoryLeakWarningPlugin::restoreNewDeleteOverloads();
    }
};

TEST(IastManager, initialization)
{
    int elems = 0;
    elems = IastManager<int>::GetInstance().getMaxItems();
    CHECK_EQUAL(2, elems);

    IastManager<int>::GetInstance().setMaxItems(4);

    elems = IastManager<int>::GetInstance().getMaxItems();
    CHECK_EQUAL(4, elems);

    elems = IastManager<int>::GetInstance().Size();
    CHECK_EQUAL(0, elems);
}



TEST(IastManager, new_item)
{
    int* ptr = nullptr;
    size_t elems = 0;
    iast_key_t key;

    key = 1;
    IastManager<int>::GetInstance().New(key);
    ptr = IastManager<int>::GetInstance().Get(key);
    CHECK(ptr != nullptr);

    elems = IastManager<int>::GetInstance().Size();
    CHECK_EQUAL(1, elems);
}

TEST(IastManager, item_reused)
{
    int* ptr = nullptr;
    int* ptr2 = nullptr;
    size_t elems = 0;
    iast_key_t key;

    key = 2;
    IastManager<int>::GetInstance().New(key);
    ptr = IastManager<int>::GetInstance().Get(key);
    CHECK(ptr != nullptr);

    IastManager<int>::GetInstance().Remove(key);
    elems = IastManager<int>::GetInstance().Size();
    CHECK_EQUAL(0, elems);

    key = 3;
    IastManager<int>::GetInstance().New(key);
    ptr2 = IastManager<int>::GetInstance().Get(key);
    POINTERS_EQUAL(ptr2, ptr);
    IastManager<int>::GetInstance().Remove(key);

    elems = IastManager<int>::GetInstance().Size();
    CHECK_EQUAL(0, elems);
}


TEST(IastManager, insert_beyond_limit)
{
    int* ptr = nullptr;
    int* ptr2 = nullptr;
    int* ptr3 = nullptr;
    size_t elems = 0;
    iast_key_t key1, key2, key3;

    elems = IastManager<int>::GetInstance().Size();
    CHECK_EQUAL(0, elems);

    key1 = 1;
    IastManager<int>::GetInstance().New(key1);
    ptr = IastManager<int>::GetInstance().Get(key1);
    CHECK(ptr != nullptr);

    key2 = 2;
    IastManager<int>::GetInstance().New(key2);
    ptr2 = IastManager<int>::GetInstance().Get(key2);
    CHECK(ptr != nullptr);

    key3 = 3;
    IastManager<int>::GetInstance().New(key3);
    ptr3 = IastManager<int>::GetInstance().Get(key3);
    CHECK(ptr3 == nullptr);
}
