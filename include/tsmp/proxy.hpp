#pragma once
#include <memory>
#include <type_traits>
#include <utility>
#include <functional>

namespace tsmp {

template <class T, template<class> class Container, class Accessor, class Functor>
struct proxy;

namespace detail {

template <class T>
using Identity = T;

struct IdentityAccessor {
    decltype(auto) operator()(auto& value) const {
        return value;
    }
};

struct DereferenceAccessor {
    decltype(auto) operator()(auto& value) const {
        return *value;
    }
};

template <class T>
using UniquePtr = std::unique_ptr<T>;

template <class T>
using SharedPtr = std::shared_ptr<T>;

struct IdentityFunctor {
    decltype(auto) operator()(auto base,std::string_view,auto&&... args) {
        return base(std::forward<decltype(args)>(args)...);
    }
};

}

template <class T, class Functor = detail::IdentityFunctor>
struct value_proxy : public proxy<T, detail::Identity, detail::IdentityAccessor, Functor> {
    value_proxy(T value, Functor fn = {}) :
        proxy<T, detail::Identity, detail::IdentityAccessor, Functor>{ std::move(value), detail::IdentityAccessor{}, std::move(fn) }
    {}

    value_proxy(const value_proxy&) = default;
    value_proxy(value_proxy&&) = default;
    
    value_proxy& operator=(const value_proxy&) = default;
    value_proxy& operator=(value_proxy&&) = default;

    ~value_proxy() = default;
};

template <class T, class Functor = detail::IdentityFunctor> requires std::has_virtual_destructor_v<T>
struct polymorphic_value_proxy : public proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor> {

    template<class C> 
    requires std::derived_from<C, T>
    polymorphic_value_proxy(C value) :
        proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor> { detail::UniquePtr<T>{ new C{std::move(value) } } },
        tsmp_copy_helper{ 
            [](const T* b){ 
                return new C{ *(dynamic_cast<const C*>(b)) }; 
            }
        }
    {}

    polymorphic_value_proxy(const polymorphic_value_proxy& cpy) :
        proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor> { detail::UniquePtr<T>{ cpy.tsmp_copy_helper(cpy.base.get()) } },
        tsmp_copy_helper{cpy.tsmp_copy_helper}
    {}

    polymorphic_value_proxy(polymorphic_value_proxy&& ) = default;
    
    polymorphic_value_proxy& operator=(const polymorphic_value_proxy& cpy) {
        proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor>::base = detail::UniquePtr<T>{ cpy.tsmp_copy_helper(cpy.base.get()) };
        tsmp_copy_helper = cpy.tsmp_copy_helper;
    };

    polymorphic_value_proxy& operator=(polymorphic_value_proxy&&) = default;

    ~polymorphic_value_proxy() = default;

    private:
        std::function<T*(const T*)> tsmp_copy_helper;
};

template <class T, class Functor = detail::IdentityFunctor>
struct unique_proxy : public proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor> {

    unique_proxy(T value, Functor fn) :
        proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor>{ detail::UniquePtr<T>{ new T{ std::move(value) } }, detail::DereferenceAccessor{}, std::move(fn) }
    {}

    unique_proxy(const unique_proxy&) = default;
    unique_proxy(unique_proxy&& ) = default;
    
    unique_proxy& operator=(const unique_proxy& cpy) = default;
    unique_proxy& operator=(unique_proxy&&) = default;

    ~unique_proxy() = default;
};

template <class T, class Functor = detail::IdentityFunctor>
struct shared_proxy : public proxy<T, detail::SharedPtr, detail::DereferenceAccessor, Functor> {

    shared_proxy(T value, Functor fn) :
        proxy<T, detail::SharedPtr, detail::DereferenceAccessor, Functor>{ detail::SharedPtr<T>{ new T{ std::move(value) } }, detail::DereferenceAccessor{}, std::move(fn) }
    {}

    shared_proxy(const shared_proxy&) = default;
    shared_proxy(shared_proxy&& ) = default;
    
    shared_proxy& operator=(const shared_proxy& cpy) = default;
    shared_proxy& operator=(shared_proxy&&) = default;

    ~shared_proxy() = default;
};

}

#if !defined(INTROSPECT_PASS) && defined(TSMP_REFLECTION_ENABLED)
#include "reflection.hpp"
#endif