// Copyright 2022 Datadog, Inc.
#ifndef SRC_CONTAINER_SINGLETON_H_
#define SRC_CONTAINER_SINGLETON_H_

namespace iast {
namespace container {
template<typename T>
class Singleton {
 public:
    static T& GetInstance() {
        static T instance;
        return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton(Singleton &&) = delete;

 private:
    Singleton();
};
}   // namespace container
}   // namespace iast
#endif  // SRC_CONTAINER_SINGLETON_H_

