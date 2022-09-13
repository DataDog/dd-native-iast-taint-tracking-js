#ifndef IASTNODE_H_
#define IASTNODE_H_

#if defined(NODE_VERSION_v4) || defined(NODE_VERSION_v6)
#define CHECK() FromJust()
#elif defined(NODE_VERSION_v8) || defined(NODE_VERSION_v10)
#define CHECK() ToChecked()
#else
#define CHECK() Check()
#endif

#if defined(NODE_VERSION_v4) || defined(NODE_VERSION_v6)
#define VAL_STR(isolate, val)   valStr(val)
#else
#define VAL_STR(isolate, val)   valStr(isolate, val)
#endif

#if defined(NODE_VERSION_v4) || defined(NODE_VERSION_v6)
#define UTF8_VALUE(isolate, val)    utf8Value(val)
#else
#define UTF8_VALUE(isolate, val)    utf8Value(isolate, val)
#endif

#if defined(NODE_VERSION_v16)
#define IS_EXTERNAL()   IsExternalTwoByte()
#else
#define IS_EXTERNAL()   IsExternal()
#endif

#endif /* ifndef IASTNODE_H_ */
