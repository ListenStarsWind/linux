#pragma once

#include <memory>

template <typename T>
std::shared_ptr<T> make_non_owning_shared(T* ptr) {
    return std::shared_ptr<T>(ptr, [](T*){});
}
