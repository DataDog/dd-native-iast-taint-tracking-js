#include <codecvt>
#include <locale>
#include "string_resource.h"

namespace iast {
namespace tainted {

void StringResource::CopyCharArrToUint16Arr(const char* charArr, uint16_t* result) {
    std::string originalString(charArr);
    std::u16string utf16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(originalString.data());
    int i = 0;
    for (char16_t c : utf16) {
        result[i] = c;
        i++;
    }
}

v8::Local<v8::String> NewExternalString(v8::Isolate* isolate, v8::Local<v8::Value> obj) {
    v8::String::Utf8Value originalStringValue(isolate, obj);
    const char* originalCharArr = *originalStringValue;
    auto resource = new StringResource(originalCharArr, originalStringValue.length());
    return v8::String::NewExternalTwoByte(isolate, resource).ToLocalChecked();
}
}  // namespace tainted
}  // namespace iast
