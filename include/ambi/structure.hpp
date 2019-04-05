// Copyright Marjan Drndarevski 2019
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md
//  or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef __STRUCTURE_HPP__
#define __STRUCTURE_HPP__

#include <any>

#include <boost/hana/adapt_struct.hpp>
#include <boost/hana/find.hpp>

#include <ambi/array.hpp>
#include <ambi/config.hpp>
#include <ambi/static_for.hpp>

namespace hana = boost::hana;

AMBI_NAMESPACE_BEGIN
    template<typename T>
    struct Structure {
        public:
            typedef T struct_type;

            Structure() : wrapped_struct{std::make_shared<T>()} {}

            template<typename...Args>
            Structure(Args... args);

            template<typename R, typename NAME>
            typename std::remove_pointer_t<R> getattr() const;

            template<std::size_t I, typename R,
                     std::enable_if_t<
                         std::is_fundamental_v<R>, R
                     > v = 0>
            void setattr(const R& value);

            template<std::size_t I, typename R,
                     std::enable_if_t<
                         std::is_pointer_v<R>, R
                     > p = nullptr>
            void setattr(const std::vector<std::remove_pointer_t<R>>& value);

            operator T() const noexcept
            {
                return *wrapped_struct;
            }

            operator T*() const noexcept
            {
                return wrapped_struct.get();
            }

            inline std::any& take_ownership(
                const std::size_t key, const std::any& object
            )
            {
                std::map<std::size_t, std::any>::iterator it = owned.find(key);
                if (it != owned.end()) {
                    it->second = object;
                } else {
                    owned.insert(std::make_pair(key, object));
                }
                return owned.at(key);
            }

        private:
            std::map<std::size_t, std::any> owned;
            std::shared_ptr<T> wrapped_struct;
    };

    template<typename T>
    template<typename... Args>
    Structure<T>::Structure(Args... args)
    {
        using Tuple = std::tuple<Args...>;

        wrapped_struct = std::make_shared<T>();
        constexpr std::size_t n = sizeof...(Args);
        auto arguments = Tuple(args...);

        // *wrapped_struct = {args...} creates dangling pointer if an
        // argument of type std::vector<T> is outlived in python.
        // Must call setattr() on all arguments to take ownership of the
        // vectors
        for_<n>([&] (auto i) {
            constexpr auto member_index = i.value;
            using RType = typename std::tuple_element_t<member_index, Tuple>;

            const auto& member = std::get<member_index>(arguments);

            if constexpr (std::is_fundamental_v<RType>) {
                this->template setattr<member_index>(member);
            } else {
                this->template
                    setattr<member_index, typename RType::value_type*>(member);
            }
        });
    }

    template<typename T>
    template<typename R, typename NAME>
    typename std::remove_pointer_t<R> Structure<T>::getattr() const
    {
        if constexpr (std::is_pointer_v<R>) {
            R member = *hana::find(*wrapped_struct, NAME{});
            if (member != nullptr) {
                return *member;
            }

            return {};
        } else {
            return *hana::find(*wrapped_struct, NAME{});
        }
    }

    template<typename T>
    template<std::size_t I, typename R,
             std::enable_if_t<std::is_fundamental_v<R>, R>>
    void Structure<T>::setattr(const R& value)
    {
        constexpr auto accessors = hana::accessors<T>();
        auto get_member = hana::second(accessors[hana::size_c<I>]);
        auto& member = get_member(*wrapped_struct);

        member = value;
    }

    template<typename T>
    template<std::size_t I, typename R,
             std::enable_if_t<std::is_pointer_v<R>, R>>
    void Structure<T>::setattr(
        const std::vector<std::remove_pointer_t<R>>& value
    )
    {
        constexpr auto accessors = hana::accessors<T>();
        auto get_member = hana::second(accessors[hana::size_c<I>]);
        auto& member = get_member(*wrapped_struct);

        using RType = std::vector<std::remove_pointer_t<R>>;

        const auto& v = std::any_cast<const RType&>(
            take_ownership(I, std::move(value))
        );

        member = const_cast<R>(v.data());
    }

#define STRUCT_MEMBER(struct_, name, type, index)                \
    constexpr auto name##_string = BOOST_HANA_STRING(#name);     \
    struct_.add_property(                                        \
        name##_string.c_str(),                                   \
        &Structure<decltype(struct_)::wrapped_type::struct_type> \
            ::getattr<type, decltype(name##_string)>,            \
        &Structure<decltype(struct_)::wrapped_type::struct_type> \
            ::setattr<index, type>                               \
    )                                                            \

AMBI_NAMESPACE_END

#endif
