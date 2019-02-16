// Copyright Marjan Drndarevski 2019
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md
//  or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef __TRAITS_HPP__
#define __TRAITS_HPP__

#include <tuple>

#include <ambi/config.hpp>

AMBI_NAMESPACE_BEGIN
    template<typename T>
    struct function_traits;

    template<typename R, typename... Args>
    struct function_traits<R(*)(Args...)>
    {
        static const std::size_t nargs = sizeof...(Args);

        typedef R result_type;
        typedef std::tuple<Args...> arg_types;
    };
AMBI_NAMESPACE_END

#endif
