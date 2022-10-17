/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_CONTAINER_SHARED_VECTOR_H_
#define SRC_CONTAINER_SHARED_VECTOR_H_
#include <vector>

using std::size_t;

namespace iast {
namespace container {

template <class T>
class SharedVector {
 public:
    SharedVector() {
        elements = new std::vector<T>();
        refs = new int(1);
    }

    SharedVector(SharedVector& v) : elements(v.elements), refs(v.refs) {
        (*refs)++;
    }

    ~SharedVector() {
        (*refs)--;
        if (!*refs) {
            delete elements;
            delete refs;
        }
    }

    SharedVector<T>& operator=(const SharedVector<T> &v) {
        if (this != &v) {
            (*refs)--;
            elements = v.elements;
            refs = v.refs;
            (*refs)++;
        }
        return *this;
    }

    size_t Size() const {
        return this->elements->size();
    }

    void PushBack(T element) {
        this->elements->push_back(element);
    }

    void Add(SharedVector* taintedRangeVector) {
        elements->insert(elements->begin(), taintedRangeVector->begin(), taintedRangeVector->end());
    }

    T At(int index) {
        return elements->at(index);
    }

    void Clear(void) {
        elements->resize(0);
    }

    typename std::vector<T>::iterator begin() {
        return elements->begin();
    }

    typename std::vector<T>::iterator end() {
        return elements->end();
    }

    unsigned int GetRefs() {
        return *refs;
    }

 private:
    std::vector<T>* elements;
    int *refs;
};
}   // namespace container
}   // namespace iast
#endif  // SRC_CONTAINER_SHARED_VECTOR_H_
