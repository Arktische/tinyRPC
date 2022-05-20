#pragma once

#include <type_traits>
template<typename T>
concept is_pod = std::is_pod_v<T>;