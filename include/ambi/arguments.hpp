// Copyright Marjan Drndarevski 2019
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md
//  or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef __ARGUMENTS_HPP__
#define __ARGUMENTS_HPP__

#include <ambi/traits.hpp>

AMBI_NAMESPACE_BEGIN
    namespace impl {
        template<typename F, typename Tuple, std::size_t... S>
        decltype(auto) apply_from_tuple(
            F&& fn, Tuple&& t, std::index_sequence<S...>
        )
        {
            return std::forward<F>(fn)(std::get<S>(std::forward<Tuple>(t))...);
        }

        // Need to be specialized for types that cannot be statically cast
        template <typename C, typename T>
        C cast(T value)
        {
            return static_cast<C>(value);
        }

        template<typename C>
        C cast(boost::python::list l)
        {
            using CastType = typename std::remove_pointer<C>::type;

            long len = boost::python::len(l);
            C array = new CastType[len];

            for (long i = 0; i < len; ++i) {
                array[i] = boost::python::extract<CastType>(l[i]);
            }

            return array;
        }

        // Need to be specialized for types that cannot be statically cast
        template <typename C, typename T>
        void consume(C c, T value)
        {
            value = cast<T>(c);
        }

        template<typename C>
        void consume(C c, boost::python::list l)
        {
            for (long i = 0; i < boost::python::len(l); ++i) {
                l[i] = c[i];
            }

            delete c;
        }

        template <std::size_t I, typename Fn, typename T>
        decltype(auto) forward_visit(Fn f, T value)
        {
            using ArgTypes = typename function_traits<decltype(f)>::arg_types;
            return std::make_tuple(
                cast<typename std::tuple_element<I, ArgTypes>::type>(value)
            );
        }

        template <std::size_t I, typename Fn, typename First, typename... Rest>
        decltype(auto) forward_visit(Fn f, First first_value, Rest... rest)
        {
            return std::tuple_cat(
                forward_visit<I>(f, first_value),
                forward_visit<I + 1>(f, rest...)
            );
        }

        template <typename C, typename T>
        void backprop_visit(C c, T value)
        {
            consume(c, value);
        }

        // XXX Termination template that consumes the tuple
        // from the last call in the backprop recursion
        template<std::size_t I, typename Tuple>
        void backprop_visit(Tuple t) {}

        template <std::size_t I, typename Types,
                  typename First, typename... Rest>
        void backprop_visit(Types t, First first_value, Rest... rest)
        {
            backprop_visit(std::get<I>(t), first_value);
            backprop_visit<I + 1>(t, rest...);
        }
    }

    template<typename F, typename Tuple>
    decltype(auto) apply_from_tuple(F&& fn, Tuple&& t)
    {
        std::size_t constexpr t_size =
            std::tuple_size<typename std::remove_reference<Tuple>::type>::value;

        return impl::apply_from_tuple(
            std::forward<F>(fn),
            std::forward<Tuple>(t),
            std::make_index_sequence<t_size>()
        );
    }

    template <typename Fn, typename... Args>
    decltype(auto) forward_visit(Fn f, Args... args)
    {
        return impl::forward_visit<0>(f, args...);
    }

    template <typename Types, typename... Args>
    void backprop_visit(Types t, Args... args)
    {
        impl::backprop_visit<0>(t, args...);
    }
AMBI_NAMESPACE_END

#endif
