#include "transaction_manager.h"
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

TEST_GROUP(TransactionManager)
{
    void setup() {
    }

    void teardown() {
    }
};

TEST(TransactionManager, initialization)
{
    int elems = 0;
    TransactionManager<FakeTransaction> iastManager;
    elems = iastManager.getMaxItems();
    CHECK_EQUAL(2, elems);

    iastManager.setMaxItems(4);

    elems = iastManager.getMaxItems();
    CHECK_EQUAL(4, elems);

    elems = iastManager.Size();
    CHECK_EQUAL(0, elems);
}

TEST(TransactionManager, new_item)
{
    TransactionManager<FakeTransaction> iastManager;
    FakeTransaction* ptr = nullptr;
    size_t elems = 0;
    iast_key_t key;

    key = 1;
    iastManager.New(key);
    ptr = iastManager.Get(key);
    CHECK(ptr != nullptr);

    elems = iastManager.Size();
    CHECK_EQUAL(1, elems);

    iastManager.Clear();
}

TEST(TransactionManager, item_reused)
{
    TransactionManager<FakeTransaction> iastManager;
    FakeTransaction* ptr = nullptr;
    FakeTransaction* ptr2 = nullptr;
    size_t elems = 0;
    iast_key_t key;

    key = 2;
    iastManager.New(key);
    ptr = iastManager.Get(key);
    CHECK(ptr != nullptr);

    iastManager.Remove(key);
    elems = iastManager.Size();
    CHECK_EQUAL(0, elems);

    key = 3;
    iastManager.New(key);
    ptr2 = iastManager.Get(key);
    POINTERS_EQUAL(ptr2, ptr);
    iastManager.Remove(key);

    elems = iastManager.Size();
    CHECK_EQUAL(0, elems);
}

TEST(TransactionManager, insert_beyond_limit)
{
    TransactionManager<FakeTransaction> iastManager;
    FakeTransaction* ptr = nullptr;
    FakeTransaction* ptr2 = nullptr;
    FakeTransaction* ptr3 = nullptr;
    size_t elems = 0;
    iast_key_t key1, key2, key3;

    elems = iastManager.Size();
    CHECK_EQUAL(0, elems);

    key1 = 1;
    iastManager.New(key1);
    ptr = iastManager.Get(key1);
    CHECK(ptr != nullptr);

    key2 = 2;
    iastManager.New(key2);
    ptr2 = iastManager.Get(key2);
    CHECK(ptr != nullptr);

    key3 = 3;
    iastManager.New(key3);
    ptr3 = iastManager.Get(key3);
    CHECK(ptr3 == nullptr);

    iastManager.Remove(key1);
    iastManager.Remove(key2);
}
