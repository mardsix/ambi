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

#include <ambi/config.hpp>

using namespace boost::python;

AMBI_NAMESPACE_BEGIN
    template<typename T>
    class Fundamental {
        public:
            Fundamental(long size) : array{std::vector<T>(size, 0)} {}

            Fundamental(const std::string& str)
            {
                for (auto const& v : str) {
                    array.push_back(v);
                }

                // NULL terminated string
                array.push_back(0);
            }

            Fundamental(const Fundamental<T>& other) : array{other.array} {}

            Fundamental(const Fundamental<T>&& other) :
                array{std::move(other.array)} {}

            virtual ~Fundamental() {}

            friend std::ostream& operator<<(
                std::ostream &os,
                const Fundamental<T>& primitive
            )
            {
                os << "[";
                bool is_first_element = true;
                for (auto const& v : primitive.array) {
                    if (!is_first_element) {
                        os << ", ";
                    }
                    os << v;
                    is_first_element = false;
                }
                os << "]";

                return os;
            }

            operator T*() const noexcept
            {
                return const_cast<T*>(array.data());
            }

            Fundamental<T>& operator=(Fundamental<T>&& other)
            {
                array = std::move(other.array);
                return *this;
            }

            Fundamental<T>& operator=(const Fundamental<T>& other)
            {
                array = other.array;
                return *this;
            }

            auto get_item(long index) const
            {
                translate_index(index);
                return array[index];
            }

            void set_item(long index, T value)
            {
                translate_index(index);
                array[index] = value;
            }

            auto begin() const noexcept
            {
                return array.begin();
            }

            auto end() const noexcept
            {
                return array.end();
            }

            static void wrap(const char *py_name)
            {
                class_<Fundamental<T>>(py_name, init<long>())
                    .def(init<const std::string&>())
                    .def("__getitem__", &Fundamental<T>::get_item)
                    .def("__setitem__", &Fundamental<T>::set_item)
                    .def(
                        "__iter__",
                        range(&Fundamental<T>::begin, &Fundamental<T>::end)
                    )
                    .def(self_ns::str(self_ns::self))
                    ;
            }

        private:
            std::vector<T> array;

            void translate_index(long &index) const
            {
                // Support python's negative indexing
                if (index < 0) {
                    index += array.size();
                }

                if (index < 0 || index >= array.size()) {
                    throw std::out_of_range("Vector index out of range");
                }
            }
    };
AMBI_NAMESPACE_END

#endif
