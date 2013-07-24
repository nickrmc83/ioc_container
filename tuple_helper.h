/*
 * tuple_helper.h - Helper functions for dealing with tuples
 *
 * Copyright (c) 2012 Nicholas A. Smith (nickrmc83@gmail.com)
 * Distributed under the Boost software license 1.0, 
 * see boost.org for a copy.
 */

#include <tuple>
#include <memory>
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

struct tuple_concat
{
    template<typename concat, typename ...arg_types>
        static std::tuple<concat, arg_types...> 
            concat( concat c, const std::tuple<arg_types...> &tuple )
        {
            std::tuple<concat, arg_types...> result;
            // Set first value
            concat &tv = std::get<0>( result );
            tv = c;
            // Set all remianing values.
            for( size_t i = 0; i < sizeof...(arg_types); i++ )
            {
                // This will never work!!!!!
                auto from = std::get<i>( tuple );
                auto &to = std::get<i+1>( result );
                to = from;
            }
            
            return result;
        }
};

template<size_t index>
struct tuple_resolve_impl;

template<>
struct tuple_resolve_impl<0>
{
    template<typename resolver_type>
        static std::tuple<> resolve( resolver_type &resolver )
        {
            return std::tuple<>();
        }
};

template<>
struct tuple_resolve_impl<1>
{
    template<typename resolver_type, typename val_type>
        static std::tuple<std::shared_ptr<val_type>> resolve( resolver_type &resolver)
        {
            return std::tuple<std::shared_ptr<val_type>>( resolver.template resolve<val_type>() );
        }
};

template<size_t index>
struct tuple_resolve_impl
{
    template<typename resolver_type, typename first, typename ...arg_types>
        static std::tuple<std::shared_ptr<first>, std::shared_ptr<arg_types>...> 
            resolve( resolver_type &resolver )
        {
            std::shared_ptr<first> f = resolver.template resolve<first>();
            auto result = tuple_concat::concat( f, 
                    tuple_resolve_impl<index-1>::
                        template resolve<resolver_type, arg_types...>(resolver) );
            return result;
        }
};

struct tuple_resolve
{
    // Return a std::tuple containing shared ptr of resolved arg_type instances.
    template<typename resolver_type, typename ...arg_types>
        static std::tuple<std::shared_ptr<arg_types>...> resolve( resolver_type &resolver )
        {
            return tuple_resolve_impl<sizeof...(arg_types)>::
                template resolve<resolver_type, arg_types...>( resolver );
        }
};
