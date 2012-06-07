/*
 * tuple.h - A simple implementation of a Tuple
 *
 * Copyright (c) 2012 Nicholas A. Smith (nickrmc83@gmail.com)
 * Distributed under the Boost software license 1.0, 
 * see boost.org for a copy.
 */

#ifndef TUPLE_H
#define TUPLE_H

#include <ioc_container/template_helpers.h>
#include <exception>

template<typename ...argtypes>
class simple_tuple;

// Specialised final item in tuple string
template<>
class simple_tuple<>
{
	public:
		static const size_t tuple_count = 0;
		typedef void value_type;
		
        simple_tuple()
		{
		}

		virtual ~simple_tuple()
		{
			// Get rid of warnings
		}

        void clear()
        {
        }

		void set()
		{
		}

		void get_value() const
		{
		}

		void set_value()
		{
		}
};

// Generic tuple.
template<typename firsttype, typename ...resttypes>
class simple_tuple<firsttype, resttypes...> : public simple_tuple<resttypes...>
{
	private:
        bool clearable;
		firsttype value;

	public:
		static const size_t tuple_count = sizeof...(resttypes) + 1;
		typedef firsttype value_type;

		simple_tuple( const firsttype &arg1, resttypes ... rest )
				: simple_tuple<resttypes...>( rest... ), 
                clearable( true ), 
                value( arg1 )
		{
		}

		simple_tuple() : simple_tuple<resttypes...>(),
                         clearable( true ),
                         value( template_helper<firsttype>::default_value() )
		{
		}

        bool has_clearable_value() const
        {
            return clearable;
        }

        void set_clearable_value( bool is_clearable_in )
        {
            clearable = is_clearable_in;
        }

        void clear_value()
        {
            if( clearable )
            {
                template_helper<firsttype>::destruct( value );
                clearable = false;
            }
        }

        void clear()
        {
            // Destruct internal item
            clear_value();
            next().clear(); 
        }

		simple_tuple<resttypes...> &next()
		{
			return ( *this );
		}
		
		const simple_tuple<resttypes...> &next_ref() const
		{
			return ( *this );
		}

		void set_value( const firsttype &value_in )
		{
			value = value_in;
		}

		void set( const firsttype &value_in, resttypes ...rest )
		{
			value = value_in;
			simple_tuple<resttypes...>::set( rest... );
		}

		firsttype get_value() const
		{
			return value;
		}

		firsttype &get_value_ref() const
		{
			return value;
		}
};

// Tuple helpers allow easy access to tuple members
template<size_t index>
struct tuple_helper_impl
{
		template<typename tupletype>
		static auto get( const tupletype &tuple )
		-> decltype( tuple_helper_impl<index-1>::get( tuple.next() ) )
		{
			return tuple_helper_impl<index - 1>::get( tuple.next() );
		}

		template<typename tupletype>
		static auto get_ref( const tupletype &tuple )
		-> decltype( tuple_helper_impl<index-1>::get_ref( tuple.next_ref() ) )
		{
			return tuple_helper_impl<index - 1>::get_ref( tuple.next_ref() );
		}

		template<typename functiontype, typename tupletype, typename ...argtypes>
		static auto call(
				functiontype func,
				const tupletype &tuple,
				argtypes ...args )
				-> decltype( tuple_helper_impl<index-1>::call( func, tuple.next_ref(), args..., tuple.get_value() ) )
		{
			return tuple_helper_impl<index - 1>::call( func, tuple.next_ref(), args...,
					tuple.get_value() );
		}

		template<typename functiontype, typename tupletype, typename ...argtypes>
		static auto call_ref(
				functiontype func,
				const tupletype &tuple,
				const argtypes &...args )
				-> decltype( tuple_helper_impl<index-1>::call_ref( func, tuple.next_ref(), args..., tuple.get_value_ref() ) )
		{
			return tuple_helper_impl<index - 1>::call_ref( func, tuple.next_ref(), args...,
					tuple.get_value_ref() );
		}
};

template<>
struct tuple_helper_impl<0>
{
		template<typename tupletype>
		static auto get( const tupletype &tuple )
		-> decltype( tuple.get_value() )
		{
			return tuple.get_value();
		}

		template<typename tupletype>
		static auto get_ref( const tupletype &tuple )
		-> decltype( tuple.Get_value_ref() )
		{
			return tuple.get_value_ref();
		}

		template<typename functiontype, typename tupletype, typename ...argtypes>
		static auto call( functiontype func, const tupletype &tuple,
				argtypes ...args )
				-> decltype( func( args... ) )
		{
			return func( args... );
		}

		template<typename functiontype, typename tupletype, typename ...argtypes>
		static auto call_ref( functiontype func, const tupletype &tuple,
				const argtypes &...args )
				-> decltype( func( args... ) )
		{
			return func( args... );
		}
};

struct tuple_helper
{
    template<typename functiontype, typename tupletype>
    static auto call( functiontype func, const tupletype &tuple )
    -> decltype( tuple_helper_impl<tupletype::tuple_count>::template call<functiontype, tupletype>( func, tuple ) )
    {
        return tuple_helper_impl<tupletype::tuple_count>::template call<functiontype, tupletype>( func, tuple );
    }

    template<typename functiontype, typename tupletype>
    static auto CallRef( functiontype func, const tupletype &tuple )
    -> decltype( tuple_helper_impl<tupletype::tuple_count>::template call_ref<functiontype, tupletype>( func, tuple ) )
    {
        return tuple_helper_impl<tupletype::tuple_count>::template call_ref<functiontype, tupletype>( func, tuple );
    }
};

#endif // TUPLE_H

