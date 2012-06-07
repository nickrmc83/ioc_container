/*
 * template_helpers.h - Some helper templates for default construction
 *                     and destruction of template types
 *
 * Copyright (c) 2012 Nicholas A. Smith (nickrmc83@gmail.com)
 * Distributed under the Boost software license 1.0, 
 * see boost.org for a copy.
 */

#ifndef TEMPLATE_HELPERS_H
#define TEMPLATE_HELPERS_H

template<typename T>
struct template_helper
{
	template<typename ...argtypes>
	static T default_value( argtypes ...args )
	{
		// This will be compiled out during optimisation.
		return T( args... );
	}

	template<typename ...argtypes>
	static T default_new( argtypes ...args )
	{
		// This will be compiled out during optimisation.
		return template_helper::default_value( args... );
	}

	static void destruct( T object )
	{
		// This is a reference type so do not do anything.
		// This will be compiled out during optimisation.
	}
};

template<typename T>
struct template_helper<T *>
{
    template<typename ...argtypes>
	static T *default_value()
	{
		return NULL;
	}

	template<typename ...argtypes>
	static T *default_new( argtypes ...args )
	{
		// This will be compiled out during optimisation.
		return new T( args... );
	}

	static void destruct( T *object )
	{
		// This is a pointer type so delete if not NULL
		if( object )
		{
			delete object;
			object = NULL;
		}
	}
};
#endif //TEMPLATE_HELPERS_H
