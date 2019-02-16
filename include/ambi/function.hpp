// Copyright Marjan Drndarevski 2019
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md
//  or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef __FUNCTION_HPP__
#define __FUNCTION_HPP__

#include <ambi/arguments.hpp>
#include <ambi/config.hpp>

AMBI_NAMESPACE_BEGIN
    template <typename... Types>
    struct TypeGroup;

    template <auto *F, typename...>
    struct Function;

    template <
        typename R,
        template <typename...> class TypeGroupT,
        typename... Args,
        typename... PythonArgs,
        auto (*F)(Args...) -> R
    >
    struct Function<F, TypeGroupT<PythonArgs...>> {
        static decltype(auto) forward(PythonArgs... python_args)
        {
            return F(std::forward<PythonArgs>(python_args)...);
        }

        static decltype(auto) propagate(PythonArgs... python_args)
        {
            auto arg_tuple = forward_visit(F, python_args...);
            auto return_value = apply_from_tuple(F, arg_tuple);
            backprop_visit(arg_tuple, python_args...);

            return return_value;
        }

        template<R RV>
        static R stub(PythonArgs...)
        {
            return RV;
        }
    };
AMBI_NAMESPACE_END

#endif
