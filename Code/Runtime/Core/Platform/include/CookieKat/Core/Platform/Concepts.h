#pragma once

#include <type_traits>

template <typename Base, typename Derived>
concept IsDerived = std::is_base_of_v<Base, Derived>;
