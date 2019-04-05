// Copyright Marjan Drndarevski 2019
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md
//  or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef __FUNDAMENTAL_H__
#define __FUNDAMENTAL_H__

#include <vector>

#define BOOST_NO_AUTO_PTR
#define BOOST_PYTHON_STATIC_LIB
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <ambi/config.hpp>

using namespace boost::python;

AMBI_NAMESPACE_BEGIN
    template<typename T>
    struct Array {
        static void wrap(const char *py_name)
        {
            class_<std::vector<T>>(py_name)
                .def(vector_indexing_suite<std::vector<T>>());
        }
    };

    template<typename T>
    std::vector<T> array(std::uint64_t size, T value = 0)
    {
        return std::vector<T>(size, value);
    }

    template<typename T>
    std::vector<T> array_from_str(const std::string& str)
    {
        std::vector<T> array_;

        for (auto const& v : str) {
            array_.push_back(v);
        }

        // NULL terminated string
        array_.push_back(0);

        return array_;
    }
AMBI_NAMESPACE_END

#endif
