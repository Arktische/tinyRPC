#pragma once

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <type_traits>

namespace common {
template<typename T>
struct slice {
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    // using iterator = 

    int length;
    int capacity;
    T* start;
    slice(std::initializer_list<T> data) {
        start = data.begin();
        capacity = length = data.size();
    }

    T operator[](int index) {
        return *(start+index);
    }

    slice<T> operator()(int start_idx, int stop_idx) {
        return slice<T>{.start=start+start_idx,.length=(stop_idx-start_idx),.capacity=capacity};
    }
};

template<typename T>
slice<T> make(int len, int capacity,T init_val) {
        
}

}



