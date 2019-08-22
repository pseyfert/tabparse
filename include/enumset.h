// This file is copied from an example of Better Enums, released under the BSD
// 2-clause license.  See LICENSE.md for details, or visit
// http://github.com/aantron/better-enums.

#pragma once

#include <bitset> // IWYU pragma: export
#define BETTER_ENUMS_STRICT_CONVERSION 1
#include "enum.h"

namespace {
template <typename Enum>
constexpr Enum max_loop(Enum accumulator, size_t index)
{
    return
        index >= Enum::_size() ? accumulator :
        Enum::_values()[index] > accumulator ?
            max_loop<Enum>(Enum::_values()[index], index + 1) :
            max_loop<Enum>(accumulator, index + 1);
}

template <typename Enum>
constexpr Enum max()
{
    return max_loop<Enum>(Enum::_values()[0], 1);
}
}

template <typename Enum>
using EnumSet = std::bitset<max<Enum>()._to_integral() + 1>;
