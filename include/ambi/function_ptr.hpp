// Copyright Marjan Drndarevski 2019
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md
//  or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef __FUNCTION_PTR_HPP__
#define __FUNCTION_PTR_HPP__

#include <ambi/config.hpp>
#include <ambi/traits.hpp>

AMBI_NAMESPACE_BEGIN
    template<typename F>
    struct FunctionPtr {
        public:
            typedef F function_ptr;

            FunctionPtr(PyObject * const py_function) {
                this->py_function = py_function;
            }

            operator F() const noexcept
            {
                return &FunctionPtr::function;
            }

            template<typename ...Args>
            static decltype(auto) function(Args...);

        private:
            static PyObject *py_function;
    };

    template<typename F>
    PyObject *FunctionPtr<F>::py_function = nullptr;

    template<typename F>
    template<typename ...Args>
    decltype(auto) FunctionPtr<F>::function(Args... args)
    {
        using ReturnType = typename function_traits<F>::result_type;

        PyEval_InitThreads();
        PyGILState_STATE state = PyGILState_Ensure();
        auto result = call<ReturnType>(py_function, args...);
        PyGILState_Release(state);
        return result;
    }
AMBI_NAMESPACE_END

#endif
