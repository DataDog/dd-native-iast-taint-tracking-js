#ifndef SRC_TAINTED_STRING_METHODS_H_
#define SRC_TAINTED_STRING_METHODS_H_

#include <node.h>

namespace iast {
namespace tainted {
using v8::Local;
using v8::Object;

class StringMethods {
    public:
        static void Init(Local<Object> exports);

    private:
        explicit StringMethods();
        ~StringMethods();
};
} // namespace tainted
} // namespace iast

#endif  // SRC_TAINTED_STRING_METHODS_H_
