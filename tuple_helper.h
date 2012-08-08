/// Helper functions for dealing with tuples.
/// Copyright Nicholas Smith 2012 All right reserved

#include <tuple>
#include <stdint.h>
#include <exception>

template<std::size_t index>
struct tuple_unwrap_impl;

template<>
struct tuple_unwrap_impl<0>
{
    template<typename callable_type, typename return_type, typename tuple_type, typename ...arg_types>
        static return_type call( callable_type callable, tuple_type tuple, arg_types ...args )
        {
            static_assert( true, "Can't unwrap an empty tuple" );
        }
};

template<>
struct tuple_unwrap_impl<1>
{
    template<typename callable_type, typename return_type, typename tuple_type, typename ...arg_types>
        static return_type call( callable_type callable, tuple_type tuple, arg_types ...args )
        {
            return callable( args..., std::get<sizeof...(arg_types)>(tuple) );
        }
};

template<size_t index>
struct tuple_unwrap_impl
{
    template<typename callable_type, typename return_type, typename tuple_type, typename ...arg_types>
        static return_type call( callable_type callable, const tuple_type &tuple, arg_types ...args )
        {
            typedef typename std::tuple_element<sizeof...(arg_types), tuple_type>::type this_type;
            return tuple_unwrap_impl<index - 1>
                ::template call<callable_type, return_type, tuple_type, arg_types..., this_type>
                ( callable, tuple, args..., std::get<sizeof...(arg_types)>(tuple) ); 
        }
};

struct tuple_unwrap
{
    template<typename callable_type, typename return_type, typename ...arg_types>
        static return_type call( callable_type callable, const std::tuple<arg_types...> &tuple )
        {
            return tuple_unwrap_impl<sizeof...(arg_types)>::
                template call<callable_type, return_type, std::tuple<arg_types...>>( callable, tuple );
        }
};
