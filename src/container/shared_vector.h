#ifndef _SHARED_VECTOR_H_
#define _SHARED_VECTOR_H_
#include <vector>

using std::size_t;

namespace iast {
namespace container {

    template <class T>
    class SharedVector {
    public:
        SharedVector(int id): id(id) {
            elements = new std::vector<T>();
            refs = new unsigned int(1);
        }

        SharedVector(SharedVector& v) : elements(v.elements), refs (v.refs), id(v.id) {
            (*refs)++;
        }

        ~SharedVector() {
            (*refs)--;
            if (!*refs) {
                remove_elements();
                delete elements;
                delete refs;
            }
        }

        SharedVector<T>& operator=(const SharedVector<T> &v) {
            if (this != &v) {
                (*refs)--;
                elements = v.elements;
                refs = v.refs;
                id = v.id;
                (*refs)++;
            }
            return *this;
        }

        inline size_t size() const {
            return this->elements->size();
        }

        inline void push_back(T element) {
            this->elements->push_back(element);
        }

        inline void add(SharedVector* taintedRangeVector) {
            elements->insert(elements->begin(), taintedRangeVector->begin(), taintedRangeVector->end());
        }

        inline T at(int index) {
            return elements->at(index);
        }

        inline void clear(void) {
            //TODO: call remove_elements when TaintedRanges don't be manually accounted for.
            elements->resize(0);
        }

        inline typename std::vector<T>::iterator begin() {
            return elements->begin();
        }

        inline typename std::vector<T>::iterator end() {
            return elements->end();
        }

        inline int getId(void) {
            return id;
        }

        inline unsigned int getRefs() {
            return *refs;
        }

    private:
        std::vector<T>* elements;
        unsigned int *refs;
        int id;
        void remove_elements() {
            if (std::is_pointer<T>::value) {
                for (int i = 0; i < elements->size(); ++i) {
                    delete (*elements)[i];
                }
            }
        }
    };
} // namespace tainted
} // namespace iast
#endif //_SHARED_VECTOR_H_
