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

// Mock V8
namespace v8 {
    template<class T> class Local {
    public:
        Local() : ptr(nullptr) {}
        explicit Local(T* p) : ptr(p) {}
        T* operator->() const { return ptr; }
        T& operator*() const { return *ptr; }
        bool IsEmpty() const { return ptr == nullptr; }
    private:
        T* ptr;
    };
    
    template<class T> class Persistent {
    public:
        Persistent() : ptr(nullptr) {}
        template<class S> Persistent(void* isolate, Local<S> other) : ptr(reinterpret_cast<T*>(0x1234)) {}
        void Reset() { ptr = nullptr; }
        template<class S> void Reset(void* isolate, Local<S> other) { ptr = reinterpret_cast<T*>(0x1234); }
        bool IsEmpty() const { return ptr == nullptr; }
    private:
        T* ptr;
    };
    
    class Value {
    public:
        Value() {}
    };
    
    class Isolate {
    public:
        static Isolate* GetCurrent() { return reinterpret_cast<Isolate*>(0x5678); }
    };
}

struct FakeTransaction final {
    transaction_key_t _id;
    v8::Persistent<v8::Value> _jsObjectRef;
    
    FakeTransaction() : _id(0) {}
    FakeTransaction(transaction_key_t id) : _id(id) {}
    FakeTransaction(transaction_key_t id, v8::Local<v8::Value> jsObject) 
        : _id(id), _jsObjectRef(v8::Isolate::GetCurrent(), jsObject) {}
    FakeTransaction(const FakeTransaction& other) {
        _id = other._id;
    }
    
    transaction_key_t getId() { return _id; }
    void Clean(void) { 
        if (!_jsObjectRef.IsEmpty()) {
            _jsObjectRef.Reset();
        }
    }
    
    void UpdateJsObjectReference(v8::Local<v8::Value> jsObject) {
        if (!_jsObjectRef.IsEmpty()) {
            _jsObjectRef.Reset();
        }
        _jsObjectRef.Reset(v8::Isolate::GetCurrent(), jsObject);
    }
    
    void Reinitialize(transaction_key_t id, v8::Local<v8::Value> jsObject) {
        Clean();
        _id = id;
        UpdateJsObjectReference(jsObject);
    }
    
    void RehashMap() {}
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
    v8::Local<v8::Value> mockJsObject;

    key = 1;
    iastManager.New(key, mockJsObject);
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
    v8::Local<v8::Value> mockJsObject;

    key = 2;
    iastManager.New(key, mockJsObject);
    ptr = iastManager.Get(key);
    CHECK(ptr != nullptr);

    iastManager.Remove(key);
    elems = iastManager.Size();
    CHECK_EQUAL(0, elems);

    key = 3;
    iastManager.New(key, mockJsObject);
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
    v8::Local<v8::Value> mockJsObject;

    elems = iastManager.Size();
    CHECK_EQUAL(0, elems);

    key1 = 1;
    iastManager.New(key1, mockJsObject);
    ptr = iastManager.Get(key1);
    CHECK(ptr != nullptr);

    key2 = 2;
    iastManager.New(key2, mockJsObject);
    ptr2 = iastManager.Get(key2);
    CHECK(ptr != nullptr);

    key3 = 3;
    iastManager.New(key3, mockJsObject);
    ptr3 = iastManager.Get(key3);
    CHECK(ptr3 == nullptr);

    iastManager.Remove(key1);
    iastManager.Remove(key2);
}

TEST(TransactionManager, create_beyond_limit)
{
    int elems = 0;
    TransactionManager<FakeTransaction, transaction_key_t> iastManager;
    v8::Local<v8::Value> mockJsObject;
    
    elems = iastManager.getMaxItems();
    CHECK_EQUAL(2, elems);

    iastManager.New(1, mockJsObject);
    CHECK_EQUAL(1, iastManager.Size());

    iastManager.New(2, mockJsObject);
    CHECK_EQUAL(2, iastManager.Size());

    CHECK_EQUAL(static_cast<FakeTransaction*>(nullptr), iastManager.New(3, mockJsObject));

    iastManager.setMaxItems(3);
    iastManager.New(3, mockJsObject);
    CHECK_EQUAL(3, iastManager.Size());

    iastManager.Clear();
    CHECK_EQUAL(0, iastManager.Size());
}
