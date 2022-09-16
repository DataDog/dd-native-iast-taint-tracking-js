// Copyright 2022 Datadog, Inc.
#ifndef SRC_TAINTED_STRING_RESOURCE_H_
#define SRC_TAINTED_STRING_RESOURCE_H_
#include <node.h>

namespace iast {
namespace tainted {
class StringResource : public v8::String::ExternalStringResource {
 public:
    explicit StringResource(const char* dataChars, int length) {
        this->length_ = length;
        auto data = new uint16_t[length];
        CopyCharArrToUint16Arr(dataChars, data);
        this->data_ = data;
    }
    ~StringResource() { delete[] this->data_; }

    virtual const uint16_t* data() const { return data_; }
    virtual size_t length()  const { return length_; }

 private:
    void CopyCharArrToUint16Arr(const char* charArr, uint16_t* result);     const uint16_t* data_;
    int length_ = 0;
};

v8::Local<v8::String> NewExternalString(v8::Isolate* isolate, v8::Local<v8::Value> obj);
}  // namespace tainted
}  // namespace iast
#endif  // SRC_TAINTED_STRING_RESOURCE_H_

