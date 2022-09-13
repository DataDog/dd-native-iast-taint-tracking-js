#ifndef _IAST_H_
#define _IAST_H_

#include <cstddef>
#include <map>
#include <cstdint>
#include <iostream>


#include "container/queued_pool.h"
#include "iast_types.h"
#include "tainted/transaction.h"

using iast::tainted::Transaction;
using iast_key_t = uintptr_t;

namespace iast {
class IastManager {
 public:
    static IastManager& GetInstance(void) {
        static IastManager _ctx;
        return _ctx;
    }

    IastManager(IastManager const&) = delete;
    void operator=(IastManager const&) = delete;

    Transaction* New(iast_key_t id) {
        if (_map.size() >= _maxItems) {
            return nullptr;
        }

        auto found = _map.find(id);
        if (found == _map.end()) {
            Transaction* item = _pool.pop();
            if (item != nullptr) {
                _map[id] = item;
                item->setId(id);
            }
            return item;
        } else {
            auto item = found->second;
            return item;
        }
    }

    Transaction* Get(iast_key_t id) {
        auto found = _map.find(id);
        if (found == _map.end()) {
            return nullptr;
        } else {
            return found->second;
        }
    }

    void Remove(iast_key_t id) {
        auto found = _map.find(id);
        if (found != _map.end()) {
            Transaction* item = found->second;
            _map.erase(found);
            item->clean();
            _pool.push(item);
        }
    }

    void RehashAll(void) {
        for (auto entry : _map) {
            if (entry.second)
                //entry.second->taintedMap->rehash();
                entry.second->RehashMap();
        }
    }

    void Clear(void) {
        for (auto it = _map.begin(); it != _map.end(); ++it) {
            _map.erase(it);
            delete it->second;
        }

        _pool.clear();
    }

    size_t Size() { return _map.size(); }
    void setMaxItems(size_t max) { _maxItems = max; }
    int getMaxItems(void) { return _maxItems; }

 private:
    IastManager() = default;
    size_t _maxItems = 2;
    TransactionPool _pool;
    std::map<iast_key_t, Transaction*> _map;
};
}  // namespace iast

#endif /* ifndef _IAST_H_ */

