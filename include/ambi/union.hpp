// Copyright Marjan Drndarevski 2019
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md
//  or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef __UNION_HPP__
#define __UNION_HPP__

#include <ambi/structure.hpp>

#define BOOST_HANA_ADAPT_UNION BOOST_HANA_ADAPT_STRUCT

AMBI_NAMESPACE_BEGIN
    template<typename T>
    struct Union : public Structure<T> {};
AMBI_NAMESPACE_END

#define UNION_MEMBER STRUCT_MEMBER
#define UNION_PTR_MEMBER STRUCT_PTR_MEMBER

#endif
