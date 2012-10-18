/*
 * tuple_helper.h - Helper functions for dealing with tuples
 *
 * Copyright (c) 2012 Nicholas A. Smith (nickrmc83@gmail.com)
 * Distributed under the Boost software license 1.0, 
 * see boost.org for a copy.
 */

#include <tuple>
#include <stdint.h>
#include <exception>
#include <ioc_container/template_helpers.h>

template<size_t index>
struct tuple_unwrap_impl;

template<>
struct tuple_unwrap_impl<0>
{
    template<typename callable_type, typename tuple_type, typename ...arg_types>
        static auto call( callable_type callable, tuple_type tuple )
        -> decltype( callable() )
        {
            return callable();
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
            //static_assert( false, "Cannot unwarp an empty tuple" );
        }
};

template<>
struct tuple_resolve_impl<1>
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
            typedef typename std::tuple_element<index - 1, tuple_type>::type this_type;
            this_type &val = std::get<index - 1>( tuple );
            // Below call may throw. Should be handled by outer recursive call
            auto resolved_value = resolver.template resolve_with_attributes<this_type>();
            try
            {
                tuple_resolve_impl<index-1>::resolve( resolver, tuple );
                val = resolved_value.first;
            }
            catch( const std::exception &e )
            {
                // destroy the reference to our resolved value
                if( resolved_value.second.is_destructable() )
                {
                    template_helper<this_type>::destruct( resolved_value.first );
                }
                // Re throw so higher levels can do the same.
                throw;
            }
        }
};

struct tuple_resolve
{
    template<typename resolver_type, typename ...arg_types>
        static void resolve( resolver_type &resolver, std::tuple<arg_types...> &tuple )
        {
            tuple_resolve_impl<sizeof...(arg_types)>::resolve( resolver, tuple );
        }
};
