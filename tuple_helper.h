/// Helper functions for dealing with tuples.
/// Copyright Nicholas Smith 2012 All right reserved

#include <tuple>
#include <stdint.h>
#include <exception>

template<size_t index>
struct tuple_unwrap_impl;

template<>
struct tuple_unwrap_impl<0>
{
    template<typename callable_type, typename tuple_type, typename ...arg_types>
        static void call( callable_type callable, tuple_type tuple, arg_types ...args )
        {
            static_assert( true, "Can't unwrap an empty tuple" );
        }
};

template<>
struct tuple_unwrap_impl<1>
{
    template<typename callable_type, typename tuple_type, typename ...arg_types>
        static auto call( callable_type callable, tuple_type tuple, arg_types ...args )
        -> decltype( callable( args..., std::get<sizeof...(arg_types)>( tuple ) ) ) 
        {
            return callable( args..., std::get<sizeof...(arg_types)>(tuple) );
        }
};

template<size_t index>
struct tuple_unwrap_impl
{
    template<typename callable_type, typename tuple_type, typename ...arg_types>
        static auto call( callable_type callable, const tuple_type &tuple, arg_types ...args )
        -> decltype(tuple_unwrap_impl<index - 1>::
                template call<callable_type, tuple_type, arg_types..., typename std::tuple_element<sizeof...(arg_types), tuple_type>::type>
            ( callable, tuple, args..., std::get<sizeof...(arg_types)>(tuple) ) )
        {
            typedef typename std::tuple_element<sizeof...(arg_types), tuple_type>::type this_type;
            return tuple_unwrap_impl<index - 1>
                ::template call<callable_type, tuple_type, arg_types..., this_type>
                ( callable, tuple, args..., std::get<sizeof...(arg_types)>( tuple ) ); 
        }
};

struct tuple_unwrap
{
    template<typename callable_type, typename ...arg_types>
        static auto call( callable_type callable, const std::tuple<arg_types...> &tuple )
        -> decltype(tuple_unwrap_impl<sizeof...(arg_types)>::
                template call<callable_type, std::tuple<arg_types...>>( callable, tuple ) )
        {
            return tuple_unwrap_impl<sizeof...(arg_types)>::
                template call<callable_type, std::tuple<arg_types...>>( callable, tuple );
        }
};

template<size_t index>
struct tuple_resolve_impl;

template<>
struct tuple_resolve_impl<0>
{
    template<typename resolver_type, typename tuple_type>
    static void resolve( resolver_type &resolver, tuple_type &tuple )
    {
        typedef typename std::tuple_element<0, tuple_type>::type this_type;
        this_type &val = std::get<0>( tuple );
        val = resolver.resolve<this_type>();
    }
};

template<size_t index>
struct tuple_resolve_impl
{
    template<typename resolver_type, typename tuple_type>
        static void resolve( resolver_type &resolver, tuple_type &tuple )
        {
            typedef typename std::tuple_element<index, tuple_type>::type this_type;
            this_type &val = std::get<index>( tuple );
            val = resolver.resolve<this_type>();
            tuple_resolve_impl<index-1>::resolve( resolver, tuple );
        }
};

struct tuple_resolve
{
    template<typename resolver_type, typename ...arg_types>
        static void resolve( resolver_type &resolver, std::tuple<arg_types...> &tuple )
        {
            tuple_resolve_impl<sizeof...(arg_types) - 1>::resolve( resolver, tuple );
        }
};