// Copyright Marjan Drndarevski 2019
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md
//  or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef __STRUCTURE_HPP__
#define __STRUCTURE_HPP__

#include <any>

#include <boost/hana/adapt_struct.hpp>
#include <boost/hana/find.hpp>

#include <ambi/config.hpp>
#include <ambi/fundamental.hpp>
#include <ambi/static_for.hpp>

namespace hana = boost::hana;

AMBI_NAMESPACE_BEGIN
    template<typename T>
    struct Structure {
        public:
            typedef T struct_type;

            Structure() : wrapped_struct{std::make_shared<T>()} {}

            template<typename...Args>
            Structure(const Args&... args);

            template<typename R, typename NAME>
            R getattr() const;

            template<typename R, typename NAME>
            R getptrattr() const;

            template<typename R, std::size_t I>
            void setattr(const R& value);

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
    template<typename...Args>
    Structure<T>::Structure(const Args&... args)
    {
        using Tuple = std::tuple<const Args&...>;

        wrapped_struct = std::make_shared<T>();
        constexpr std::size_t n = sizeof...(Args);
        auto arguments = Tuple(args...);

        for_<n>([&] (auto i) {
           using RType = typename std::tuple_element<i.value, Tuple>::type;
           this->template setattr<RType, i.value>(
               std::get<i.value>(arguments)
           );
        });

        // XXX creates dangling pointer if an arg of type Fundamental<T>
        // is outlived in python. Must assume ownership of
        // Fundamental<T> objects by calling setattr()
        // *wrapped_struct = {args...};
    }


    template<typename T>
    template<typename R, typename NAME>
    R Structure<T>::getattr() const
    {
        return *hana::find(*wrapped_struct, NAME{});
    }

    template<typename T>
    template<typename R, typename NAME>
    R Structure<T>::getptrattr() const
    {
        R* member = *hana::find(*wrapped_struct, NAME{});
        if (member != nullptr) {
            return *member;
        }

        return {};
    }

    template<typename T>
    template<typename R, std::size_t I>
    void Structure<T>::setattr(const R& value)
    {
        constexpr auto accessors = hana::accessors<T>();
        auto get_member = hana::second(accessors[hana::size_c<I>]);

        // XXX Calls T*() operator on Fundamental<T> types of objects
        // that may be outlived in python making Structure<T> 'member'
        // a dangling pointer of type R.
        // We must assume ownership of the Fundamental<T> type objects.
        auto& member = get_member(*wrapped_struct);

        using MemberType = std::remove_reference_t<
            std::remove_pointer_t<decltype(member)>
        >;
        using MemberPointeeType = std::remove_pointer_t<MemberType>;
        using RType = std::remove_cv_t<std::remove_reference_t<R>>;

        //show_types(value);

        if (std::is_same<RType, Fundamental<MemberPointeeType>>::value) {
            // Assume ownership of Fundamental<T> type objects.
            // take_ownership() calls the copy constructor of
            // Fundamental<T> on the value object.
            member = std::any_cast<const R&>(
                take_ownership(I, std::move(value))
            );
        } else {
            member = value;
        }
    }

#define STRUCT_MEMBER(struct_, name, type, index)                \
    constexpr auto name##_string = BOOST_HANA_STRING(#name);     \
    struct_.add_property(                                        \
        name##_string.c_str(),                                   \
        &Structure<decltype(struct_)::wrapped_type::struct_type> \
            ::getattr<type, decltype(name##_string)>,            \
        &Structure<decltype(struct_)::wrapped_type::struct_type> \
            ::setattr<type, index>                               \
    )                                                            \

#define STRUCT_PTR_MEMBER(struct_, name, type, index)            \
    constexpr auto name##_string = BOOST_HANA_STRING(#name);     \
    struct_.add_property(                                        \
        name##_string.c_str(),                                   \
        &Structure<decltype(struct_)::wrapped_type::struct_type> \
            ::getptrattr<type, decltype(name##_string)>,         \
        &Structure<decltype(struct_)::wrapped_type::struct_type> \
            ::setattr<Fundamental<type>&, index>                 \
    )                                                            \

AMBI_NAMESPACE_END

#endif
