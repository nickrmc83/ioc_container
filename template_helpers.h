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
#include <exception>

class null_argument_exception : public std::exception
{
    private:
        std::string function_name;
        std::string argument_name;
    
    public:
        null_argument_exception( const std::string &function,
                const std::string &argument )
            : function_name( function ), 
            argument_name( argument )
        {
        }

        ~null_argument_exception() throw()
        {
        }

        const std::string &get_function_name() const
        {
            return function_name;
        }

        const std::string &get_argument_name() const
        {
            return argument_name;
        }

        const char *what() const throw()
        {
            std::string result = std::string( "Argument " ) +
                argument_name + 
                std::string( " was null in function " ) +
                function_name;
            return result.c_str();
        }
};

template<typename T>
struct template_helper
{
    typedef T type;

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
    typedef T* type;
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

template<typename T>
struct template_helper<T []>
{
    template<typename ...argtypes>
        static T *default_value()
        {
            return NULL;
        }

        static T *default_new( size_t count )
        {
            return new T[count];
        }

    static void destruct( T *object )
    {
        // This is an array type so delete if not NULL
        if( object )
        {
            delete [] object;
            object = NULL;
        }
    }
};
#endif //TEMPLATE_HELPERS_H
