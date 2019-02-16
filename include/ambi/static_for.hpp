// Copyright Marjan Drndarevski 2019
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md
//  or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef __STATIC_FOR_HPP__
#define __STATIC_FOR_HPP__

#include <ambi/config.hpp>

AMBI_NAMESPACE_BEGIN
    namespace impl {
        template<std::size_t N>
        struct integral_const {
            static const constexpr auto value = N;
        };

        template <typename F, std::size_t... I>
        void for_(F f, std::index_sequence<I...>)
        {
          (f(integral_const<I>{}), ...);
        }
    }

    template <std::size_t N, typename F>
    void for_(F func)
    {
      impl::for_(func, std::make_index_sequence<N>());
    }
AMBI_NAMESPACE_END

#endif
