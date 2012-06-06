#ifndef IOC_H
#define IOC_H

#include <stdlib.h>
#include <typeinfo>
#include <vector>
#include <string>
#include <ioc_container/Tuple.h>
#include <ioc_container/TemplateHelpers.h>

namespace IOC
{
// Constant identifiers
static const std::string IOCTypeNameRegistration = "IOC Container";
static const std::string UnnamedTypeRegistration = "Unnamed registration";

// IFactory is the base interface for a factory 
// type. CreateItem returns a void * which can
// then be reinterpret_cast'd to the required type.
class IFactory
{
    public:
		IFactory()
		{
		}

		virtual ~IFactory()
		{
		}

		virtual const std::type_info &GetType() const = 0;
		virtual const std::string &GetName() const = 0;
		virtual void *CreateItem() const = 0;
        virtual bool IsDestructable() const = 0;
};

template<size_t Index>
struct TupleValueResolverImpl
{
	template<typename ResolverType, typename TupleType>
	static void Set( ResolverType *Resolver,
			TupleType &Tuple )
	{
        typedef typename TupleType::ValueType ThisType; 
        // Resolve factory. We need to do this so we can ascertain
        // if we can manually clear up items in case of an exception.
        // For example, if we register an instance type then we cannot
        // automatically destruct this value. However if the resolver
        // has new'd an object then we can destruct it.
        bool Destructable = false;
        ThisType Value = TemplateHelper<ThisType>::Default();
        const IFactory *Factory = Resolver->template ResolveFactory<ThisType>();
        if( Factory )
        {
            Destructable = Factory->IsDestructable();
            Value = reinterpret_cast<ThisType>( Factory->CreateItem() );
        }
        // Now create object type wrapper and assign it to our tuple
		Tuple.SetValue( Value );
        Tuple.SetClearableValue( Destructable );
		TupleValueResolverImpl<Index - 1>::Set( Resolver, Tuple.Next() );
	}
};

template<>
struct TupleValueResolverImpl<0>
{
	template<typename ResolverType, typename TupleType>
	static void Set( ResolverType *Resolver,
			TupleType &Tuple )
	{
		// This should get optimised out.
	}
};

struct TupleValueResolver
{
    template<typename ResolverType, typename ...ArgTypes>
    static SimpleTuple<ArgTypes...> Get( ResolverType *Resolver )
    {   
        SimpleTuple<ArgTypes...> Result;
        try
        {
            TupleValueResolverImpl<sizeof...(ArgTypes)>::Set( Resolver, Result );
        }
        catch( const std::exception &e )
        {
            // Release all pre-allocated values.
            Result.Clear();
            throw;
        }
        return Result;
    }
};

// Registration exception classes
class RegistrationException : public std::exception
{
    private:
        std::string TypeName;
        std::string RegistrationName;
    public:
        RegistrationException( const std::string &TypeNameIn, const std::string &RegistrationNameIn )
            : std::exception(), TypeName( TypeNameIn ), RegistrationName( RegistrationNameIn )
        {
        }

        const std::string &GetTypeName() const
        {
            return TypeName;
        }

        const std::string &GetRegistrationName() const
        {
            return RegistrationName;
        }

        const char *what() const throw()
        {
            std::string Error = std::string( "Previous registration of type (Type: " ) +
               TypeName + std::string( " , " ) + RegistrationName + std::string( ")" );
           return Error.c_str(); 
        }
};

// Resolver. All object types are registered with the resolver
// at run-time and can then be resolved. Resolver supports
// constructor injection.
class Container
{
    private:
		// Below are classes that allow items
		// to be created. Depending on the registration
		// method a different type of factory object
		// is added to the internal factory list. Each
        // factory is specialised to only be able to
        // create a single type.
        
        // BaseFatory extends IFactory to provide some standard
        // functionality that is required by most concrete
        // factoy types.
		template<typename I>
		class BaseFactory : public IFactory
		{
			private:
				std::string Name;
				virtual I InternalCreateItem() const = 0;

			public:

				BaseFactory( const std::string &NameIn ) 
				: IFactory(), Name( NameIn )
				{
				}

                ~BaseFactory()
                {
                }

				const std::type_info &GetType() const
				{
					return typeid(I);
				}
				
				const std::string &GetName() const
				{
					return Name;
				}
                
				void *CreateItem() const
				{
					return static_cast<void *>( InternalCreateItem() );
				}
		};
        
        // DelegateFactory allows delegate objects or routines to be
        // supplied and called for object construction. All delegate
        // arguments are resolved by the resolver before being send
        // to the delegate instance.
		template<typename I, typename Callable, typename ...ArgTypes>
		class DelegateFactory : public BaseFactory<I>
		{
			private:
                IOC::Container *ContainerObj;
				Callable CallableObj;

				I InternalCreateItem() const
				{
                    // Resolve delgate arguments
                    typedef SimpleTuple<ArgTypes...> ThisTupleType;
                    // Resolve all variables for construction.
                    // If there is an error during resolution
                    // then the Resolver will de-allocate any
                    // already resolved objects for us.
                    I Result = TemplateHelper<I>::Default();
                    ThisTupleType Args = 
                        TupleValueResolver::Get<IOC::Container, ArgTypes...>( ContainerObj );
                    try
                    {
				        Result = TupleHelper::Call( CallableObj, Args );
                    }
                    catch( const std::exception &e )
                    {
                        Args.Clear();
                        throw;
                    }
					return Result;
				}

			public:
				DelegateFactory( const std::string &NameIn, IOC::Container *ContainerIn, const Callable &CallableObjIn )
				: BaseFactory<I>( NameIn ), ContainerObj( ContainerIn ), CallableObj( CallableObjIn )
				{
				}

                ~DelegateFactory()
                {
                }

                bool IsDestructable() const
                {
                    return true;
                }
		};
        
        // ResolvableFactory extends DelegateFactory by supplying
        // a standard function which can be used to instantiate
        // and return an instance of a specific type.
        template<typename I, typename T, typename ...ArgTypes>
		class ResolvableFactory 
        : public DelegateFactory<I, T (*)( ArgTypes...), ArgTypes...>
		{
            private: 
                static T Creator( ArgTypes ...Args )
                {
                    return TemplateHelper<T>::DefaultNew( Args... );
                }

			public:
				ResolvableFactory( const std::string &NameIn, IOC::Container *ContainerIn )
				: DelegateFactory<I, T (*)( ArgTypes... ), ArgTypes...>( NameIn, ContainerIn, ResolvableFactory::Creator )
				{
				}

                ~ResolvableFactory()
                {
                }
		};
		
		template<typename I>
		class InstanceFactory
        : public BaseFactory<I>
		{
            private: 
                I Instance;
				
				I InternalCreateItem() const
				{
					return Instance;
				}

			public:
				InstanceFactory( const std::string &NameIn, I InstanceIn )
				: BaseFactory<I>( NameIn ), Instance( InstanceIn )
				{
				}

                ~InstanceFactory()
                {
                }

                bool IsDestructable() const
                {
                    return false;
                }
		};
		
		// Internal list of registered types
        std::vector<IFactory *> Types;

        static void DestroyFactory( IFactory *Factory )
        {
            if( Factory )
            {
                delete Factory;
                Factory = NULL;
            }
        }

    public:
        Container()
        {
			// Register Container so it can be resolved into
			// objects for delayed resolution later after
			// construction
			RegisterInstanceWithName<IOC::Container *>( IOCTypeNameRegistration, this );
        }

        ~Container()
        {
            // Destroy all factories
            for( std::vector<IFactory *>::const_iterator i = Types.begin(); i != Types.end(); ++i )
            {
                DestroyFactory( *i );
            }
            Types.clear();
        }

        // Check if a factory to create a gievn interface
        // already exists
        template<typename Interface>
        bool TypeIsRegistered( const std::string &NameIn ) const
        {
            bool Result = false;	
            std::vector<IFactory *>::const_iterator i = Types.begin();
            while( ( Result == false ) && ( i != Types.end() ) )
            {
                if( ( (*i)->GetType() == typeid(Interface) ) &&
                        ( (*i)->GetName() == NameIn ) )
                {
                    Result = true;
                }
                ++i;
            }
            return Result;
        }

        template<typename Interface>
        bool TypeIsRegistered() const
        {
            return TypeIsRegistered<Interface>( UnnamedTypeRegistration );
        }
        
        // Registration helper
        template<typename FactoryType, typename Interface, typename ...ArgTypes>
        void RegisterWithNameTemplate( const std::string &NameIn, ArgTypes... Args )
        {
            if( TypeIsRegistered<Interface>( NameIn ) )
            {
                // Throw an exception as we cannot register a type
                // which has already been registered
                // TODO: typeid does not return quite what I expected!!!!!
                throw RegistrationException( typeid(Interface).name(), NameIn );
            }
            FactoryType *NewFactory = new FactoryType( NameIn, Args... );
            Types.push_back( NewFactory );
        }

        template<typename Interface, typename CallableType, typename ...ArgTypes>
		void RegisterDelegateWithName( const std::string &NameIn, CallableType CallObj )
		{
			// Create a functor which returns an Interface type
			// but actually news a Concretion.
			typedef DelegateFactory<Interface, CallableType, ArgTypes...> FactoryType;
            RegisterWithNameTemplate<FactoryType, Interface, IOC::Container *, CallableType>( NameIn, this, CallObj );
		}
		
		template<typename Interface, typename CallableType, typename ...ArgTypes>
		void RegisterDelegate( CallableType CallObj )
		{
			// Register nameless delegate constructor
			RegisterDelegateWithName<Interface, CallableType, ArgTypes...>( UnnamedTypeRegistration, CallObj );
		}

        template<typename Interface, typename Concretion, typename ...ArgTypes>
		void RegisterWithName( const std::string &NameIn )
		{
			typedef ResolvableFactory<Interface, Concretion, ArgTypes...> FactoryType;
			RegisterWithNameTemplate<FactoryType, Interface, IOC::Container *>( NameIn, this );
		}
		
		template<typename Interface, typename Concretion, typename ...ArgTypes>
		void Register()
		{
			// Register nameless constructor object
			RegisterWithName<Interface, Concretion, ArgTypes...>( UnnamedTypeRegistration );
		}

		template<typename Interface>
		void RegisterInstanceWithName( const std::string &NameIn, Interface InstanceIn )
		{
			// Create instance constuctor and register in our type list
			typedef InstanceFactory<Interface> FactoryType;
            RegisterWithNameTemplate<FactoryType, Interface, Interface>( NameIn, InstanceIn ); 
		}

		
		template<typename Interface>
		void RegisterInstance( Interface InstanceIn )
		{
			RegisterInstanceWithName<Interface>( UnnamedTypeRegistration, InstanceIn );
		}
        
        // Resolve factory for interface. If that fails then return NULL.
        template<typename Interface>
		const IFactory *ResolveFactory() const
		{
			// Lookup interface type. If it cannot be found return
            // the default for that type.
			IFactory *Result = NULL;
            std::vector<IFactory *>::const_iterator i = Types.begin();
			while( ( Result == NULL ) && ( i != Types.end() ) )
			{
				if( (*i)->GetType() == typeid(Interface) )
				{
					Result = *i;
				}
				i++;
			}
			return Result;
		}
        
        // Resolve interface type. If that fails then return NULL.
        template<typename Interface>
        Interface Resolve() const
        {
            Interface Result = TemplateHelper<Interface>::Default();
            const IFactory *Factory = ResolveFactory<Interface>();
            if( Factory )
            {
                Result = reinterpret_cast<Interface>( Factory->CreateItem() );
            }

            return Result;
        }
		
        // Resolve factory for interface type by name. 
        // If that fails then return NULL.
		template<typename Interface>
		IFactory *ResolveFactoryByName( const std::string &NameIn ) const
		{
			// Lookup interface type. If it cannot be found return
			// the default for that type.
			IFactory *Result = NULL;
            std::vector<IFactory *>::const_iterator i = Types.begin();
			while( ( Result == NULL ) && ( i != Types.end() ) )
			{
				if( ( (*i)->GetType() == typeid(Interface) ) &&
					( (*i)->GetName() == NameIn ) )
				{
					Result = *i;
				}
				i++;
			}
			return Result;
		}

        // Resolve interface type by name. If that fails then return NULL.
        template<typename Interface>
        Interface ResolveByName( const std::string &NameIn ) const
        {
            Interface Result = TemplateHelper<Interface>::Default();
            const IFactory *Factory = ResolveFactoryByName<Interface>( NameIn );
            if( Factory )
            {
                Result = reinterpret_cast<Interface>( Factory->CreateItem() );
            }
            return Result;
        }
        
        // Destroy the first factory which creates an interface
        template<typename Interface>
        bool RemoveRegistration()
        {
            bool Result = false;
            std::vector<IFactory *>::iterator i = Types.begin();
            while( ( Result == false ) && ( i != Types.end() ) )
            {
                if( (*i)->GetType() == typeid(Interface) )
                {
                    DestroyFactory( *i );
                    Types.erase( i ); 
                    Result = true;   
                }
                ++i;
            }
            return Result;
        }

        // Destroy the first named factory which creates an
        // interface
        template<typename Interface>
        bool RemoveRegistrationByName( const std::string &NameIn )
        {
            bool Result = false;
            std::vector<IFactory *>::iterator i = Types.begin();
            while( ( Result == false ) && ( i != Types.end() ) )
            {
                if( ( (*i)->GetType() == typeid(Interface) ) &&
                        ( (*i)->GetName() == NameIn ) )
                {
                    DestroyFactory( *i );
                    Types.erase( i ); 
                    Result = true;   
                }
                ++i;
            }
            return Result;
        }
}; // namespace IOC
};
#endif // IOC_H

