// Copyright 2022 Datadog, Inc.

#include <node.h>
#include "iast.h"
#include "transaction.h"



namespace iast {

template<>
void IastManager<Transaction>::RehashAll(void) {
    for (auto entry : _map) {
        if (entry.second)
            entry.second->RehashMap();
    }
}

template<>
void IastManager<Transaction>Remove(iast_key_t id) {
    auto found = _map.find(id);
    if (found != _map.end()) {
        T* item = found->second;
        _map.erase(found);
        item->clean();
        _pool.push(item);
    }
}

void Init(v8::Local<v8::Object> exports) {
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
}   // namespace iast
