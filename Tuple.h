/*
 * Tuple.h
 *
 *  Created on: 3 Feb 2012
 *      Author: nick
 */

#ifndef TUPLE_H
#define TUPLE_H
#include <ioc_container/TemplateHelpers.h>
#include <exception>

template<typename ...ArgTypes>
class SimpleTuple;

// Specialised final item in tuple string
template<>
class SimpleTuple<>
{
	public:
		static const size_t TupleCount = 0;
		typedef void ValueType;
		
        SimpleTuple()
		{
		}

		virtual ~SimpleTuple()
		{
			// Get rid of warnings
		}

        void Clear()
        {
        }

		void Set()
		{
		}

		void GetValue() const
		{
		}

		void SetValue()
		{
		}
};

// Generic tuple.
template<typename FirstType, typename ...RestTypes>
class SimpleTuple<FirstType, RestTypes...> : public SimpleTuple<RestTypes...>
{
	private:
        bool Clearable;
		FirstType Value;

	public:
		static const size_t TupleCount = sizeof...(RestTypes) + 1;
		typedef FirstType ValueType;

		SimpleTuple( const FirstType &Arg1, RestTypes ... Rest )
				: SimpleTuple<RestTypes...>( Rest... ), 
                Clearable( true ), 
                Value( Arg1 )
		{
		}

		SimpleTuple() : SimpleTuple<RestTypes...>()
		{
		}

        bool HasClearableValue() const
        {
            return Clearable;
        }

        void SetClearableValue( bool IsClearableIn )
        {
            Clearable = IsClearableIn;
        }

        void ClearValue()
        {
            if( Clearable )
            {
                TemplateHelper<FirstType>::Destruct( Value );
                Clearable = false;
            }
        }

        void Clear()
        {
            // Destruct internal item
            ClearValue();
            Next().Clear(); 
        }

		SimpleTuple<RestTypes...> &Next()
		{
			return ( *this );
		}
		
		const SimpleTuple<RestTypes...> &NextRef() const
		{
			return ( *this );
		}

		void SetValue( const FirstType &ValueIn )
		{
			Value = ValueIn;
		}

		void Set( const FirstType &ValueIn, RestTypes ...Rest )
		{
			Value = ValueIn;
			SimpleTuple<RestTypes...>::Set( Rest... );
		}

		FirstType GetValue() const
		{
			return Value;
		}

		FirstType &GetValueRef() const
		{
			return Value;
		}
};

// Tuple helpers allow easy access to tuple members
template<size_t Index>
struct TupleHelperImpl
{
		template<typename TupleType>
		static auto Get( const TupleType &Tuple )
		-> decltype( TupleHelperImpl<Index-1>::Get( Tuple.Next() ) )
		{
			return TupleHelperImpl<Index - 1>::Get( Tuple.Next() );
		}

		template<typename TupleType>
		static auto GetRef( const TupleType &Tuple )
		-> decltype( TupleHelperImpl<Index-1>::GetRef( Tuple.NextRef() ) )
		{
			return TupleHelperImpl<Index - 1>::GetRef( Tuple.NextRef() );
		}

		template<typename FunctionType, typename TupleType, typename ...ArgTypes>
		static auto Call(
				FunctionType Func,
				const TupleType &Tuple,
				ArgTypes ...Args )
				-> decltype( TupleHelperImpl<Index-1>::Call( Func, Tuple.NextRef(), Args..., Tuple.GetValue() ) )
		{
			return TupleHelperImpl<Index - 1>::Call( Func, Tuple.NextRef(), Args...,
					Tuple.GetValue() );
		}

		template<typename FunctionType, typename TupleType, typename ...ArgTypes>
		static auto CallRef(
				FunctionType Func,
				const TupleType &Tuple,
				const ArgTypes &...Args )
				-> decltype( TupleHelperImpl<Index-1>::CallRef( Func, Tuple.NextRef(), Args..., Tuple.GetValueRef() ) )
		{
			return TupleHelperImpl<Index - 1>::CallRef( Func, Tuple.NextRef(), Args...,
					Tuple.GetValueRef() );
		}
};

template<>
struct TupleHelperImpl<0>
{
		template<typename TupleType>
		static auto Get( const TupleType &Tuple )
		-> decltype( Tuple.GetValue() )
		{
			return Tuple.GetValue();
		}

		template<typename TupleType>
		static auto GetRef( const TupleType &Tuple )
		-> decltype( Tuple.GetValueRef() )
		{
			return Tuple.GetValueRef();
		}

		template<typename FunctionType, typename TupleType, typename ...ArgTypes>
		static auto Call( FunctionType Func, const TupleType &Tuple,
				ArgTypes ...Args )
				-> decltype( Func( Args... ) )
		{
			return Func( Args... );
		}

		template<typename FunctionType, typename TupleType, typename ...ArgTypes>
		static auto CallRef( FunctionType Func, const TupleType &Tuple,
				const ArgTypes &...Args )
				-> decltype( Func( Args... ) )
		{
			return Func( Args... );
		}
};

struct TupleHelper
{
    template<typename FunctionType, typename TupleType>
    static auto Call( FunctionType Func, const TupleType &Tuple )
    -> decltype( TupleHelperImpl<TupleType::TupleCount>::template Call<FunctionType, TupleType>( Func, Tuple ) )
    {
        return TupleHelperImpl<TupleType::TupleCount>::template Call<FunctionType, TupleType>( Func, Tuple );
    }

    template<typename FunctionType, typename TupleType>
    static auto CallRef( FunctionType Func, const TupleType &Tuple )
    -> decltype( TupleHelperImpl<TupleType::TupleCount>::template CallRef<FunctionType, TupleType>( Func, Tuple ) )
    {
        return TupleHelperImpl<TupleType::TupleCount>::template CallRef<FunctionType, TupleType>( Func, Tuple );
    }
};

#endif // TUPLE_H

