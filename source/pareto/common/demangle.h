//
// Created by Alan Freitas on 12/12/20.
//

#ifndef PARETO_DEMANGLE_H
#define PARETO_DEMANGLE_H

#include <string>
#include <typeinfo>

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#endif

namespace pareto {
#ifdef __GNUG__
    /// \brief Demangle a type name
    /// https://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname
    inline std::string demangle(const char *name) {
        // some arbitrary value to eliminate the compiler warning
        int status = -4;
        std::unique_ptr<char, void (*)(void *)> res{
            abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free};
        return (status == 0) ? res.get() : name;
    }
#else
    /// \brief Demangle a type name
    /// https://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname
    /// does nothing if not g++ compatible
    inline std::string demangle(const char *name) { return name; }
#endif

    template <class T>
    std::string demangle() {
        return pareto::demangle(typeid(T).name());
    }

} // namespace pareto

#endif // PARETO_DEMANGLE_H
