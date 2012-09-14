/*
 * ioc.h - An implementation of a IOC dependency injection
 * engine
 *
 * Copyright (c) 2012 Nicholas A. Smith (nickrmc83@gmail.com)
 * Distributed under the Boost software license 1.0, 
 * see boost.org for a copy.
 */ 


#ifndef IOC_H
#define IOC_H

#include <stdlib.h>
#include <typeinfo>
#include <vector>
#include <string>
//#include <tuple.h>
#include <tuple>
#include <template_helpers.h>
#include <tuple_helper.h>

namespace ioc
{
    // Constant identifiers
    static const std::string 
        ioc_type_name_registration = "IOC Container";
    static const std::string 
        unnamed_type_name_registration = "Unnamed registration";

    // pre-declaration to satisfy factory types
    class container;

    // ifactory is the base interface for a factory 
    // type. CreateItem returns a void * which can
    // then be reinterpret_cast'd to the required type.
    class ifactory 
    {
        public:
            ifactory()
            {
            }

            virtual ~ifactory()
            {
            }

            virtual const std::type_info &get_type() const = 0;
            virtual const std::string &get_name() const = 0;
            virtual void* create_item() const = 0;
            virtual bool is_destructable() const = 0;
    };

    class resolution_attributes
    {
        private:
            bool destructable;
        public:
            resolution_attributes()
                : destructable( false )
            {
            }

            resolution_attributes( bool destructable_in )
                : destructable( destructable_in )
            {
            }

            bool is_destructable() const
            {
                return destructable;
            }
    };

    // BaseFatory extends ifactory to provide some standard
    // functionality that is required by most concrete
    // factoy types.
    template<typename I>
        class base_factory : public ifactory
    {
        private:
            std::string name;
            virtual I internal_create_item() const = 0;

        public:

            base_factory( const std::string &name_in ) 
                : ifactory(), name( name_in )
            {
            }

            ~base_factory()
            {
            }

            const std::type_info &get_type() const
            {
                return typeid(I);
            }

            const std::string &get_name() const
            {
                return name;
            }

            void *create_item() const
            {
                return static_cast<void *>( internal_create_item() );
            }
    };

    // DelegateFactory allows delegate objects or routines to be
    // supplied and called for object construction. All delegate
    // arguments are resolved by the resolver before being send
    // to the delegate instance.
    template<typename I, typename callable, typename ...argtypes>
        class delegate_factory : public base_factory<I>
    {
        private:
            ioc::container &container_obj;
            callable callable_obj;

            I internal_create_item() const
            {
                // Resolve delgate arguments
                // Resolve all variables for construction.
                // If there is an error during resolution
                // then the Resolver will de-allocate any
                // already resolved objects for us.
                std::tuple<argtypes...> args;
                tuple_resolve::resolve( container_obj, args );
                I result = tuple_unwrap::call( callable_obj, args );
                return result;
            }

        public:
            delegate_factory( const std::string &name_in, 
                    ioc::container &container_in, const 
                    callable &callable_obj_in )
                : base_factory<I>( name_in ), container_obj( container_in ), 
                callable_obj( callable_obj_in )
        {
        }

            ~delegate_factory()
            {
            }

            bool is_destructable() const
            {
                return true;
            }
    };

    // ResolvableFactory extends DelegateFactory by supplying
    // a standard function which can be used to instantiate
    // and return an instance of a specific type.
    template<typename I, typename T, typename ...argtypes>
        class resolvable_factory 
        : public delegate_factory<I, T (*)( argtypes...), argtypes...>
        {
            private: 
                static T creator( argtypes ...args )
                {
                    return template_helper<T>::default_new( args... );
                }

            public:
                resolvable_factory( 
                        const std::string &name_in, 
                        ioc::container &container_in )
                    : delegate_factory<I, T (*)( argtypes... ), argtypes...>
                      ( name_in, container_in, resolvable_factory::creator )
            {
            }

                ~resolvable_factory()
                {
                }
        };

    // isntance_factory stores an instance of the required type.
    // create_item simply returns the stored instance.
    // It should be noted that there is no guard around the instance
    // to stop it being deleted by some other object once it has
    // been resolved.
    template<typename I>
        class instance_factory
        : public base_factory<I>
        {
            private: 
                I instance;

                I internal_create_item() const
                {
                    return instance;
                }

            public:
                instance_factory( const std::string &name_in, I instance_in )
                    : base_factory<I>( name_in ), instance( instance_in )
                {
                }

                ~instance_factory()
                {
                }

                bool is_destructable() const
                {
                    return false;
                }
        };

    // Registration exception classes
    class registration_exception : public std::exception
    {
        private:
            std::string type_name;
            std::string registration_name;
        public:
            registration_exception( const std::string &type_name_in, 
                    const std::string &registration_name_in )
                : std::exception(), type_name( type_name_in ), 
                registration_name( registration_name_in )
        {
        }

            const std::string &get_type_name() const
            {
                return type_name;
            }

            const std::string &get_registration_name() const
            {
                return registration_name;
            }

            const char *what() const throw()
            {
                std::string error = 
                    std::string( "Previous registration of type (Type: " ) +
                    type_name + std::string( " , " ) + registration_name + 
                    std::string( ")" );
                return error.c_str(); 
            }
    };

    // Container. All object types are registered with the container
    // at run-time and can then be resolved. Resolver supports
    // constructor injection.
    class container
    {
        private:
            // Internal list of registered types
            std::vector<ifactory *> types;

            static inline void destroy_factory( ifactory *factory )
            {
                if( factory )
                {
                    delete factory;
                    factory = NULL;
                }
            }

        public:
            container()
            {
                // Register Container so it can be resolved into
                // objects for delayed resolution later after
                // construction
                register_instance_with_name<ioc::container *>( 
                        ioc_type_name_registration, this );
            }

            ~container()
            {
                // Destroy all factories
                for( std::vector<ifactory *>::reverse_iterator i = types.rbegin();
                        i != types.rend(); ++i )
                {
                    destroy_factory( *i );
                }

                types.clear();
            }

            // Check if a factory to create a gievn interface
            // already exists
            template<typename I>
                bool type_is_registered( const std::string &name_in ) const
                {
                    bool result = false;	
                    std::vector<ifactory *>::const_iterator i = types.begin();
                    std::vector<ifactory *>::const_iterator end = types.end();
                    while( ( result == false ) && ( i != end ) )
                    {
                        if( ( (*i)->get_type() == typeid(I) ) &&
                                ( (*i)->get_name() == name_in ) )
                        {
                            result = true;
                        }
                        ++i;
                    }

                    return result;
                }

            template<typename I>
                bool type_is_registered() const
                {
                    return type_is_registered<I>( 
                            unnamed_type_name_registration );
                }

            // Registration helper
            template<typename F, typename I, typename ...argtypes>
                void register_with_name_template( const std::string &name_in,
                        argtypes... args )
                {
                    if( type_is_registered<I>( name_in ) )
                    {
                        // Throw an exception as we cannot register a type
                        // which has already been registered
                        throw registration_exception( typeid(I).name(), 
                                name_in );
                    }
                    F *new_factory = new F( name_in, args... );
                    types.push_back( new_factory );
                }

            template<typename I, typename callable, typename ...argtypes>
                void register_delegate_with_name( const std::string &name_in,
                        callable call_obj )
                {
                    // Create a functor which returns an Interface type
                    // but actually news a Concretion.
                    typedef delegate_factory<I, callable, argtypes...> 
                        factorytype;
                    register_with_name_template<factorytype, I,
                        ioc::container &, callable>( name_in, *this, call_obj );
                }

            template<typename I, typename callable, typename ...argtypes>
                void register_delegate( callable call_obj )
                {
                    // Register nameless delegate constructor
                    register_delegate_with_name<I, callable, argtypes...>( 
                            unnamed_type_name_registration, call_obj );
                }

            template<typename I, typename T, typename ...argtypes>
                void register_type_with_name( const std::string &name_in )
                {
                    typedef resolvable_factory<I, T, argtypes...> factorytype;
                    register_with_name_template<factorytype, I, 
                        ioc::container &>( name_in, *this );
                }

            template<typename I, typename T, typename ...argtypes>
                void register_type()
                {
                    // Register nameless constructor object
                    register_type_with_name<I, T, argtypes...>( 
                            unnamed_type_name_registration );
                }

            template<typename I>
                void register_instance_with_name( const std::string &name_in,
                        I instance_in )
                {
                    // Create instance constuctor and register in our type list
                    typedef instance_factory<I> factorytype;
                    register_with_name_template<factorytype, I, I>( name_in, 
                            instance_in );
                }


            template<typename I>
                void register_instance( I instance_in )
                {
                    register_instance_with_name<I>( 
                            unnamed_type_name_registration, instance_in );
                }

            // Resolve factory for interface. If that fails then return NULL.
            template<typename I>
                const ifactory *resolve_factory() const
                {
                    // Lookup interface type. If it cannot be found return
                    // the default for that type.
                    ifactory *result = NULL;
                    std::vector<ifactory *>::const_iterator i = types.begin();
                    while( ( result == NULL ) && ( i != types.end() ) )
                    {
                        if( (*i)->get_type() == typeid(I) )
                        {
                            result = *i;
                        }
                        i++;
                    }
                    return result;
                }
            
            template<typename I>
                std::pair<I, resolution_attributes> resolve_with_attributes() const
                {
                    I result = template_helper<I>::default_value();
                    const ifactory *factory = resolve_factory<I>();
                    if( !factory )
                    {
                        // TODO create special exception for this type
                        throw std::bad_exception();
                    }
                    
                    resolution_attributes attribs( factory->is_destructable() );
                    result = reinterpret_cast<I>( factory->create_item() );
                    return std::pair<I, resolution_attributes>( result, attribs );
                }

            // Resolve interface type. If that fails then return NULL.
            template<typename I>
                I resolve() const
                {
                    I result = template_helper<I>::default_value();
                    const ifactory *factory = resolve_factory<I>();
                    if( factory )
                    {
                        result = reinterpret_cast<I>( factory->create_item() );
                    }

                    return result;
                }

            // Resolve factory for interface type by name. 
            // If that fails then return NULL.
            template<typename I>
                ifactory *
                resolve_factory_by_name( const std::string &name_in ) const
                {
                    // Lookup interface type. If it cannot be found return
                    // the default for that type.
                    ifactory *result = NULL;
                    std::vector<ifactory *>::const_iterator i = types.begin();
                    while( ( result == NULL ) && ( i != types.end() ) )
                    {
                        if( ( (*i)->get_type() == typeid(I) ) &&
                                ( (*i)->get_name() == name_in ) )
                        {
                            result = *i;
                        }
                        i++;
                    }
                    return result;
                }

            // Resolve interface type by name. If that fails then return NULL.
            template<typename I>
                I resolve_by_name( const std::string &name_in ) const
                {
                    I result = template_helper<I>::default_value();
                    const ifactory *factory = 
                        resolve_factory_by_name<I>( name_in );
                    if( factory )
                    {
                        result = reinterpret_cast<I>( factory->create_item() );
                    }
                    return result;
                }

            // Destroy the first factory which creates an interface
            template<typename I>
                bool remove_registration()
                {
                    bool result = false;
                    std::vector<ifactory *>::iterator i = types.begin();
                    std::vector<ifactory *>::iterator end = types.end();
                    while( ( result == false ) && ( i != end ) )
                    {
                        if( (*i)->get_type() == typeid(I) )
                        {
                            destroy_factory( *i );
                            types.erase( i ); 
                            result = true;   
                        }
                        ++i;
                    }
                    return result;
                }

            // Destroy the first named factory which creates an
            // interface
            template<typename I>
                bool remove_registration_by_name( const std::string &name_in )
                {
                    bool result = false;
                    std::vector<ifactory *>::iterator i = types.begin();
                    std::vector<ifactory *>::iterator end = types.end();
                    while( ( result == false ) && ( i != end ) )
                    {
                        if( ( (*i)->get_type() == typeid(I) ) &&
                                ( (*i)->get_name() == name_in ) )
                        {
                            destroy_factory( *i );
                            types.erase( i ); 
                            result = true;   
                        }
                        ++i;
                    }
                    return result;
                }
    }; // namespace IOC
};
#endif // IOC_H

