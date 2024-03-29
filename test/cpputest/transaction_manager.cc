/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include "transaction_manager.h"
#include <CppUTest/UtestMacros.h>
#include <CppUTest/TestHarness.h>
#include <cstdint>

using namespace iast;
using namespace iast::container;
using transaction_key_t = uintptr_t;

struct FakeTransaction final {
    transaction_key_t _id;
    FakeTransaction() { _id = 0; }
    FakeTransaction(transaction_key_t id): _id(id) { };
    FakeTransaction(const FakeTransaction& other) {
        _id = other._id;
    }
    transaction_key_t getId() { return _id; }
    void Clean(void) { return; }
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
    TransactionManager<FakeTransaction, transaction_key_t> iastManager;
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
    TransactionManager<FakeTransaction, transaction_key_t> iastManager;
    FakeTransaction* ptr = nullptr;
    size_t elems = 0;
    transaction_key_t key;

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
    TransactionManager<FakeTransaction, transaction_key_t> iastManager;
    FakeTransaction* ptr = nullptr;
    FakeTransaction* ptr2 = nullptr;
    size_t elems = 0;
    transaction_key_t key;

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
    TransactionManager<FakeTransaction, transaction_key_t> iastManager;
    FakeTransaction* ptr = nullptr;
    FakeTransaction* ptr2 = nullptr;
    FakeTransaction* ptr3 = nullptr;
    size_t elems = 0;
    transaction_key_t key1, key2, key3;

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

TEST(TransactionManager, create_beyond_limit)
{
    int elems = 0;
    TransactionManager<FakeTransaction, transaction_key_t> iastManager;
    elems = iastManager.getMaxItems();
    CHECK_EQUAL(2, elems);

    iastManager.New(1);
    CHECK_EQUAL(1, iastManager.Size());

    iastManager.New(2);
    CHECK_EQUAL(2, iastManager.Size());

    CHECK_EQUAL(static_cast<FakeTransaction*>(nullptr), iastManager.New(3));

    iastManager.setMaxItems(3);
    iastManager.New(3);
    CHECK_EQUAL(3, iastManager.Size());

    iastManager.Clear();
    CHECK_EQUAL(0, iastManager.Size());
}
