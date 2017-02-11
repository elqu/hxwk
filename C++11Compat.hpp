#ifndef HXWK_CPP11COMPAT_H
#define HXWK_CPP11COMPAT_H

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#if __cplusplus < 201402L
namespace std {

template <class T, class... Args>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
typename std::enable_if<std::is_array<T>::value && std::extent<T>::value == 0,
                        std::unique_ptr<T>>::type
make_unique(std::size_t n) {
    return std::unique_ptr<T>(new typename std::remove_extent<T>::type[n]());
}

template <class T, class... Args>
typename std::enable_if<std::extent<T>::value != 0, std::unique_ptr<T>>::type
make_unique(Args &&... args)
        = delete;
}
#endif

#endif
