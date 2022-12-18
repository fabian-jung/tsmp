#pragma once
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <functional>

namespace tsmp {

template <class T, template<class> class Container, class Accessor, class Functor>
struct proxy;

}

#if !defined(TSMP_INTROSPECT_PASS) && defined(TSMP_REFLECTION_ENABLED)
#include "reflection.hpp"
#else 
namespace tsmp {

template <class T, template<class> class Container, class Accessor, class Functor>
struct proxy : public T {
    template <class... Args>
    proxy(Args&&...) {}
};

}
#endif

namespace tsmp {

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
    value_proxy(T value, Functor f = {}) :
        proxy<T, detail::Identity, detail::IdentityAccessor, Functor>{ std::move(value), detail::IdentityAccessor{}, std::move(f) }
    {}

    value_proxy(const value_proxy&) = default;
    value_proxy(value_proxy&&) = default;
    
    value_proxy& operator=(const value_proxy&) = default;
    value_proxy& operator=(value_proxy&&) = default;

    ~value_proxy() = default;
};

template <class T, class Functor = detail::IdentityFunctor> requires std::has_virtual_destructor_v<T>
struct polymorphic_value : public proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor> {

    template<class C> 
    requires std::derived_from<C, T>
    polymorphic_value(C value) :
        proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor> { detail::UniquePtr<T>{ new C{std::move(value) } }, {}, {} },
        tsmp_copy_helper{ 
            [](const T* b){ 
                if constexpr(std::is_copy_constructible_v<C>) {
                    return new C{ *(dynamic_cast<const C*>(b)) };
                } else {
                    return nullptr;
                }
            }
        }
    {}

    polymorphic_value(const polymorphic_value& cpy) :
        proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor> { detail::UniquePtr<T>{ cpy.tsmp_copy_helper(cpy.__tsmp_base.get()) }, {}, {} },
        tsmp_copy_helper{cpy.tsmp_copy_helper}
    {
        if(this->__tsmp_base.get() == nullptr) {
            throw std::runtime_error("Trying to copy non copyable value.");
        }
    }

    polymorphic_value(polymorphic_value&& ) = default;
    
    polymorphic_value& operator=(const polymorphic_value& cpy) {
        proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor>::__tsmp_base = detail::UniquePtr<T>{ cpy.tsmp_copy_helper(cpy.__tsmp_base.get()), {}, {} };
        tsmp_copy_helper = cpy.tsmp_copy_helper;
        if(this->__tsmp_base.get() == nullptr) {
            throw std::runtime_error("Trying to copy non copyable value.");
        }
    }

    polymorphic_value& operator=(polymorphic_value&&) = default;

    ~polymorphic_value() = default;

    private:
        std::function<T*(const T*)> tsmp_copy_helper;
};

template <class T, class Functor = detail::IdentityFunctor>
struct unique_proxy : public proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor> {

    unique_proxy(T value, Functor f) :
        proxy<T, detail::UniquePtr, detail::DereferenceAccessor, Functor>{ detail::UniquePtr<T>{ new T{ std::move(value) } }, detail::DereferenceAccessor{}, std::move(f) }
    {}

    unique_proxy(const unique_proxy&) = default;
    unique_proxy(unique_proxy&& ) = default;
    
    unique_proxy& operator=(const unique_proxy& cpy) = default;
    unique_proxy& operator=(unique_proxy&&) = default;

    ~unique_proxy() = default;
};

template <class T, class Functor = detail::IdentityFunctor>
struct shared_proxy : public proxy<T, detail::SharedPtr, detail::DereferenceAccessor, Functor> {

    shared_proxy(T value, Functor f) :
        proxy<T, detail::SharedPtr, detail::DereferenceAccessor, Functor>{ detail::SharedPtr<T>{ new T{ std::move(value) } }, detail::DereferenceAccessor{}, std::move(f) }
    {}

    shared_proxy(const shared_proxy&) = default;
    shared_proxy(shared_proxy&& ) = default;
    
    shared_proxy& operator=(const shared_proxy& cpy) = default;
    shared_proxy& operator=(shared_proxy&&) = default;

    ~shared_proxy() = default;
};

}