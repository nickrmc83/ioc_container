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
#include <map>
#include <string>
#include <cstring>
#include <memory>
#include <typeindex>

namespace ioc
{
    // Constant identifiers
    static const std::string 
        ioc_type_name_registration = "IOC Container";
    static const std::string 
        unnamed_type_name_registration = "Unnamed registration";

    class container;

    // ifactory is the base interface for a factory 
    // type. CreateItem returns a void * which can
    // then be reinterpret_cast'd to the required type.
    class ifactory 
    {
        public:
            virtual ~ifactory(){}
            virtual const std::type_info &get_type() const = 0;
            virtual const std::string &get_name() const = 0;
            virtual void* create_item() const = 0;
    };

    // BaseFactory extends ifactory to provide some standard
    // functionality that is required by most concrete
    // factory types.
    template<typename I>
        class base_factory : public ifactory
    {
        private:
            std::string name;
            virtual I *internal_create_item() const = 0;

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

    template<size_t index>
        struct recursive_resolve_impl;

    template<>
        struct recursive_resolve_impl<0>
        {
            template<typename resolver_type, typename t, typename callable_type>
                static t *resolve(resolver_type &resolver, callable_type callable)
                {
                    return callable();
                }
        };

    template<size_t i>
        struct recursive_resolve_impl
        {
            template<typename resolver_type, typename t, 
                typename callable_type, typename ...argtypes>
                    static t *resolve(resolver_type &resolver, callable_type callable)
                    {
                        return callable(resolver.template resolve<argtypes>()...);
                    }
        };

    struct recursive_resolve
    {
        template<typename t, typename resolver_type, 
            typename callable_type, typename ...argtypes>
                static t *resolve(resolver_type &resolver, callable_type callable)
                {
                    return recursive_resolve_impl<sizeof...(argtypes)>
                        ::template resolve<resolver_type, t, callable_type, argtypes...>(resolver, callable);
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

            I *internal_create_item() const
            {
                // Resolve all variables for construction.
                // If there is an error during resolution
                // then the Resolver will de-allocate any
                // already resolved objects for us.

                //auto args =
                //    tuple_resolve::
                //        resolve<ioc::container, argtypes...>( container_obj );
                //I *result = tuple_unwrap::call( callable_obj, args );
                I *result = recursive_resolve
                    ::resolve<I, ioc::container, callable, argtypes...>(container_obj, callable_obj);
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

    };

    // ResolvableFactory extends DelegateFactory by supplying
    // a standard function which can be used to instantiate
    // and return an instance of a specific type.
    template<typename I, typename T, typename ...argtypes>
        class resolvable_factory 
        : public delegate_factory<I, I* (*)( std::shared_ptr<argtypes>...), 
        argtypes...>
    {
        private:
            static I *creator(std::shared_ptr<argtypes>... args)
            {
                return new T(args...);
            }
        public:
            typedef I *(func_type)(std::shared_ptr<argtypes>...);

            resolvable_factory( 
                    const std::string &name_in, 
                    ioc::container &container_in )
                : delegate_factory<I, I *(*)(std::shared_ptr<argtypes>...), argtypes...>
                  ( name_in, container_in, resolvable_factory::creator )
        {
        }

            ~resolvable_factory()
            {
            }
    };

    // instance_factory stores an instance of the required type.
    // create_item simply returns the stored instance.
    // It should be noted that there is no guard around the instance
    // to stop it being deleted by some other object once it has
    // been resolved.
    template<typename I>
        class instance_factory
        : public base_factory<I>
        {
            private: 
                std::shared_ptr<I> instance;

                I *internal_create_item() const
                {
                    return instance.get();
                }

            public:
                instance_factory( const std::string &name_in, std::shared_ptr<I> instance_in )
                    : base_factory<I>( name_in ), instance( instance_in )
                {
                }

                ~instance_factory()
                {
                }
        };

    // Registration exception classes
    class registration_exception : public std::exception
    {
        private:
            std::string type_name;
            std::string registration_name;
            std::string error;
        public:
            registration_exception( const std::string &type_name_in, 
                    const std::string &registration_name_in )
                : std::exception(), type_name( type_name_in ), 
                registration_name( registration_name_in )
        {
            error = std::string( "Previous registration of type (Type: " ) +
                    type_name + std::string( " , " ) + registration_name + 
                    std::string( ")" );
        }

            ~registration_exception() throw()
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
                return error.c_str(); 
            }
    };

    // Container. All object types are registered with the container
    // at run-time and can then be resolved. Resolver supports
    // constructor injection.
    class container
    {
        private:
            template<typename T>
            struct ellided_deleter
            {
                void operator()(T *val)
                {
                    // Shhhhh, don't actually delete the ptr.
                }
            };
            typedef ellided_deleter<container> container_deleter;
            
            // Internal map of registered types -> map of named instances of
            // type factories.
            typedef std::map<std::string, ifactory*> named_factory;
            typedef std::map<std::type_index, named_factory> registration_types;

            registration_types types;

            std::shared_ptr<container> self;

            static inline void destroy_factory( ifactory *factory )
            {
                if( factory )
                {
                    delete factory;
                    factory = NULL;
                }
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
                    types[std::type_index(typeid(I))][name_in] = new_factory;
                }
            
            // Resolve factory for interface. If that fails then return NULL.
            template<typename I>
                const ifactory *resolve_factory() const
                {
                    // Lookup interface type. If it cannot be found return
                    // the default for that type.
                    ifactory *result = NULL;
                    registration_types::const_iterator i = types.find(std::type_index(typeid(I)));
                    if( i != types.end() )
                    {
                        const named_factory candidates =
                            i->second;
                        result = (candidates.begin())->second;
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
                    registration_types::const_iterator i = types.find(std::type_index(typeid(I)));
                    if( i != types.end() )
                    {
                        // We've got the type registered but we now need to look
                        // up the named version.
                        const named_factory::const_iterator c = 
                            i->second.find(name_in);
                        if( c != i->second.end() )
                        {
                            result = c->second;
                        }
                    }
                    return result;
                }
            
            

        public:
            container() : self(this, container_deleter())
            {
                // Register our special shared_ptr which will not
                // delete if a container is resolved.
                this->register_instance<container>(self);
            }

            ~container()
            {
                // Destroy all factories
                for( registration_types::reverse_iterator i = types.rbegin();
                        i != types.rend(); ++i )
                {
                    for(named_factory::reverse_iterator j = i->second.rbegin(); 
                            j != i->second.rend(); ++j)
                    {
                        destroy_factory( j->second );
                    }
                    i->second.clear();
                }

                types.clear();
            }

            // Check if a factory to create a given interface
            // already exists
            template<typename I>
                bool type_is_registered( const std::string &name_in ) const
                {
                    const ifactory *f = resolve_factory_by_name<I>( name_in );    
                    return f ? true : false;
                }

            template<typename I>
                bool type_is_registered() const
                {
                    const ifactory *f = resolve_factory<I>();    
                    return f ? true : false;
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
                        std::shared_ptr<I> instance_in )
                {
                    // Create instance constuctor and register in our type list
                    typedef instance_factory<I> factorytype;
                    register_with_name_template<factorytype, I, std::shared_ptr<I>>( 
                            name_in, 
                            instance_in );
                }


            template<typename I>
                void register_instance( std::shared_ptr<I> instance_in )
                {
                    register_instance_with_name<I>( 
                            unnamed_type_name_registration, instance_in );
                }

            // Resolve interface type. If that fails then return NULL.
            template<typename I>
                std::shared_ptr<I> resolve() const
                {
                    I *result = NULL;
                    const ifactory *factory = resolve_factory<I>();
                    if( factory )
                    {
                        result = reinterpret_cast<I *>( factory->create_item() );
                    }

                    return std::shared_ptr<I>(result);
                }

            // Resolve interface type by name. If that fails then return NULL.
            template<typename I>
                std::shared_ptr<I> resolve_by_name( const std::string &name_in ) const
                {
                    I *result = NULL;
                    const ifactory *factory = 
                        resolve_factory_by_name<I>( name_in );
                    if( factory )
                    {
                        result = reinterpret_cast<I *>( factory->create_item() );
                    }
                    return std::shared_ptr<I>(result);
                }

            // Destroy all factories implementing the given interface
            template<typename I>
                bool remove_registration()
                {
                    bool result = false;
                    registration_types::iterator i = types.find(std::type_index(typeid(I)));
                    if( i != types.end() )
                    {
                        for( named_factory::iterator j = i->second.begin(); 
                                j != i->second.end(); ++j )
                        {
                            destroy_factory( j->second );
                        }
                        types.erase(i);
                        result = true;
                    }
                    return result;
                }

            // Destroy the first named factory which creates an
            // interface
            template<typename I>
                bool remove_registration_by_name( const std::string &name_in )
                {
                    bool result = false;
                    registration_types::iterator i = types.find(std::type_index(typeid(I)));
                    if( i != types.end() )
                    {
                        named_factory::iterator j = i->second.find(name_in); 
                        if( j != i->second.end() )
                        {
                            destroy_factory( j->second );
                            i->second.erase( j );
                           result = true; 
                        }
                    }
                    return result;
                }
    }; // namespace IOC
};
#endif // IOC_H

