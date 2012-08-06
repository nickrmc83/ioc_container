/// Helper functions for dealing with tuples.
/// Copyright Nicholas Smith 2012 All right reserved

template <>
struct tuple_unwrap_impl<0>
{
    template<typename callable_type, typename return_type, typename tupe_type, typename ...arg_types>
        static return_type call( callable_type callable, tuple_type tuple, arg_types ...args )
        {
            return callable( std::get<0>( tuple ), args... );
        }
};

template<size_t index>
struct tuple_unwrap_impl
{
    template<typename callable_type, typename return_type, typename tuple_type, typename ...arg_types>
        static return_type call( callable_type callable, tuple_type tuple, arg_types ...args )
        {
            return template_unwrap_impl<index -1>
                ::call( callable, tuple, std::get<index>(tuple).value, args... ); 
        }
};

struct tuple_unwrap
{
    template<typename callable_type, typename return_type, typename tuple_type>
        static return_type call( callable_type callable, tuple_type tuple )
        {
            return tupe_unwrap_impl<tuple_type::size - 1>::
                call<callable_type, return_type, tuple_type>( callable, tuple );
        }
};
