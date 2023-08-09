#pragma once
#include "tsmp/introspect.hpp"
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace tsmp {

template<class GlobalNamespaceHelper, class T, class Accessor, class Functor, class... Base>
struct proxy_impl;

template<class T, class Accessor, class Functor, class... Base>
using proxy = proxy_impl<global_t, T, Accessor, Functor, Base...>;

}

#if !defined(TSMP_INTROSPECT_PASS) && defined(TSMP_REFLECTION_ENABLED)
#include "reflection.hpp"
#else
namespace tsmp {

template<class T, class Accessor, class Functor, class... Base>
struct proxy : public Base...
{
    template<class... Args>
    proxy(Args&&...)
    {
    }
};

}
#endif

namespace tsmp {

template<class T>
struct ThisAccessor
{
    decltype(auto) operator()(auto* proxy) const { return proxy; }
};

template<class T, class Functor>
using virtual_proxy = proxy<T, ThisAccessor<T>, Functor, T>;

template<class T>
struct value_accessor_t
{

    value_accessor_t(T value)
        : value(std::move(value))
    {
    }

    T& operator()(auto*) { return value; }

    T value;
};

template<class T, class Functor>
struct value_proxy : public proxy<T, value_accessor_t<T>, Functor>
{
    value_proxy(T value = {}, Functor f = {})
        : proxy<T, value_accessor_t<T>, Functor>{value_accessor_t<T>{std::move(value)}, std::move(f)}
    {
    }

    value_proxy(const value_proxy&) = default;
    value_proxy(value_proxy&&) = default;

    value_proxy& operator=(const value_proxy&) = default;
    value_proxy& operator=(value_proxy&&) = default;

    ~value_proxy() = default;
};

template<class T>
struct unique_accessor_t
{

    template<std::derived_from<T> D>
    unique_accessor_t(D&& value)
        : ptr(std::unique_ptr<T>(new D{std::move(value)}))
    {
    }

    template<std::derived_from<T> D>
    unique_accessor_t(const D& value)
        : ptr(std::unique_ptr<T>(new D{value}))
    {
    }

    decltype(auto) operator()(auto*) const { return ptr.get(); }

    std::unique_ptr<T> ptr;
};

template<class T, class Functor>
struct unique_proxy : public proxy<T, unique_accessor_t<T>, Functor>
{
    unique_proxy(T value = {}, Functor functor = {})
        : proxy<T, unique_accessor_t<T>, Functor>(unique_accessor_t<T>(std::move(value)), std::move(functor))
    {
    }
};

template<class T, class Functor>
unique_proxy(T, Functor) -> unique_proxy<T, Functor>;

template<class T>
struct shared_accessor_t
{

    template<std::derived_from<T> D>
    shared_accessor_t(D&& value)
        : ptr(std::unique_ptr<T>(new D{std::move(value)}))
    {
    }

    template<std::derived_from<T> D>
    shared_accessor_t(const D& value)
        : ptr(std::unique_ptr<T>(new D{value}))
    {
    }

    decltype(auto) operator()(auto*) const { return ptr.get(); }

    std::shared_ptr<T> ptr;
};

template<class T, class Functor>
struct shared_proxy : public proxy<T, shared_accessor_t<T>, Functor>
{
    shared_proxy(T value = {}, Functor functor = {})
        : proxy<T, shared_accessor_t<T>, Functor>(shared_accessor_t<T>(std::move(value)), std::move(functor))
    {
    }
};

template<class T, class Functor>
shared_proxy(T, Functor) -> shared_proxy<T, Functor>;

struct identity
{
    decltype(auto) operator()(auto base, std::string_view name, auto&&... args) const
    {
        return std::invoke(base, std::forward<decltype(args)>(args)...);
    }
};

template<class T, class Functor = identity>
struct polymorphic_value : public proxy<T, unique_accessor_t<T>, Functor>
{

    template<class C>
        requires std::derived_from<C, T>
    polymorphic_value(C value)
        : proxy<T, unique_accessor_t<T>, Functor>{unique_accessor_t<T>{std::move(value)}}
        , tsmp_copy_helper{[](const unique_accessor_t<T>& b) {
            if constexpr (std::is_copy_constructible_v<C>) {
                return unique_accessor_t<T>{*dynamic_cast<const C*>(b.ptr.get())};
            } else {
                throw std::runtime_error("Trying to copy a non-copyable value.");
            }
        }}
    {
    }

    polymorphic_value(const polymorphic_value& cpy)
        : proxy<T, unique_accessor_t<T>, Functor>{cpy.tsmp_copy_helper(cpy.accessor)}
        , tsmp_copy_helper{cpy.tsmp_copy_helper}
    {
    }

    polymorphic_value(polymorphic_value&&) = default;

    polymorphic_value& operator=(const polymorphic_value& cpy)
    {
        proxy<T, unique_accessor_t<T>, Functor>::_accessor = cpy.tsmp_copy_helper(cpy.__tsmp_accessor);
        tsmp_copy_helper = cpy.tsmp_copy_helper;
    }

    polymorphic_value& operator=(polymorphic_value&&) = default;

    ~polymorphic_value() = default;

private:
    std::function<unique_accessor_t<T>(const unique_accessor_t<T>&)> tsmp_copy_helper;
};

template<class T>
polymorphic_value(T) -> polymorphic_value<T>;

}