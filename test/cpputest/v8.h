/**
* Mock V8 header for unit testing
* This file provides mock implementations of V8 types for unit tests only
**/
#ifndef V8_H_
#define V8_H_

namespace v8 {
    class Value {};
    
    class Isolate { 
    public:
        static Isolate* GetCurrent() { static Isolate i; return &i; }
    };
    
    template<class T> 
    class Local { 
        T val;
    public:
        Local() = default;
        T* operator->() { return &val; }
        T& operator*() { return val; }
    };
    
    template<class T> 
    class PersistentBase { 
        bool empty = true;
    public:
        PersistentBase() = default;
        template<class S> PersistentBase(Isolate*, Local<S>) : empty(false) {}
        void Reset() { empty = true; }
        template<class S> void Reset(Isolate*, Local<S>) { empty = false; }
        bool IsEmpty() const { return empty; }
    };
    
    template<class T> 
    class Persistent : public PersistentBase<T> {
    public:
        Persistent() = default;
        template<class S> Persistent(Isolate* i, Local<S> l) : PersistentBase<T>(i, l) {}
    };
}

#endif // V8_H_
