ioc_container
=============

A C++ IOC container capable of constructor dependency injection and runtime registration of types. It is possible to register types, delegate objects and instances which maybe resolved later within an application. Due to the runtime nature of registrations it is possible to both add and remove registrations on an adhoc basis.

The source is known to both build and work when compiled with g++ 4.7 and Clang 3.0 C++ compilers. It uses a number of C++11 features including variadic templates and automatic type deduction and so requires the appropriate compiler switches to allow the use of such features e.g. -std=c++0x.

Tutorial:
---------

Two simple examples of registering a types with and without any constrctor dependencies is outlined below. The example shows how a type bar derived from foo can be registered with the IOC container and later an instance can be resolved from the same container for use later. The example later shows how a type dah, which is derived from lardy and requires an instance of foo for constrction, can be registered, resolved and used.

```cpp
// Example. Simple registration and resolution
int main(char **args, int argv)
{
	// Create an instance of an
	// ioc::conatianer
	ioc::container Container;

	// Register bar which is derived
	// from foo
	Container.register_type<foo, bar>();

	// elided

	// Resolve a new instance of foo
	std::shared_ptr<foo> fooInstance = Container.resolve<foo>();
	// Call a method on our resolved
	// instance
	fooInstance->Call();

	// Register dah which is derived
	// from lardy which requires an
	// instance of foo in construction
	Container.register_type<lardy, dah, foo>();

	// elided

	// Resolve a new instance of lardy
	std::shared_ptr<lardy> lardyInstance = Container.resolve<Lardy>();
	// Call some method on our resolved
	// instance
	lardyInstance->Call();

	return 0;
};
```

As well as being able to register types with dependant constructor parameters, it is also possible to register delgates (callable objects such as functions or classes which implement operator ()) and Instances (an instance in this context means registering a pre-constructed object which maybe resolved at a later date). Delegates like standard registrations can require dependendant types in their signature. For example, the below code illustrates how to register a delegate which requires the type foo which we register earlier.

```cpp
// Example. Delegate registration
static SomeType *DoSomething( std::shared_ptr<foo> obj )
{
	SomeType *Result = NULL:
	if( obj.get() )
	{
		// New an instance of SomeDerivedType which
		// derives from SomeType. Pass obj to the
		// constructor as well as some non-resolvable
		// constructor parameters
		Result = new SomeDerivedType( obj, 10, "WOOOO" );
	}
	return Result;
}

void RegisterDelegateExample()
{
	// Register
	typedef SomeType (*DelegateSignature)( foo * );
	Container.register_delegate<SomeType, DelegateSignature, foo>( &DoSomething );

	// elided

	// Resolve a new instance of SomeType
	std::shared_ptr<SomeType> inst = Container.resolve<SomeType>();
	// Call some method
	inst->DoSometing();
}
```

To register a specific instance of a class which can later be resolved the below code can be used. This is useful when a singleton is required.

```cpp
// Example. Register and instance
void RegisterInstanceExample()
{
	// Register
	std::shared_ptr<SomeDervied_type> singleton( new SomeDerivedType() );
	Container.register_instance<SomeType>( singleton );

	// elided

	// Resolve our previously registered instance
	std::shared_ptr<SomeType> inst = Container.resolve<SomeType>();
	inst->DoSomething();
}
```

Standard resoltuion (Resolve<Type>()) searches for the first matching registered type in the IOC containers dependency list. However, it is not possible to register two identical types unless using named registration. Named registration allows multiple matching types to be registered with the caveat that each is accompanied by a name by which it maybe resolved. For example the below code will throw a RegistrationException when the second registration is attempted.

```cpp
// Example. Matching registration exception
void RegisterSomeTypes()
{
	// First registration works fine.
	Container.register_type<SomeType, SomeDerivedType>();
	// Subsequent registations of type SomeType * will
	// fail unless "named" registration is used.
	Container.register_type<SomeType, SomeOtherDerivedType>(); // This throws an exception!! 
}
```

To enable the above code to compile correctly named registration can be used. Name registartion is available when registering types, delegates or instances. See a below for a self-explanatory example of registering and resolving types by name.

```cpp
// Example. Named registration and resolution example
void RegisterAndResolveSomeTypes()
{
	// Register with name "TypeA"
	Container.register_type_with_name<SomeType, SomeDerivedType>( "TypeA" );
	// Register the same type this time with "TypeB". Note if we attempted
	// to register another version of SomeType * with the same name ("TypeA")
	// Then we would get a RegistrationException.
	Container.Register_type_with_name<SomeType, SomeOtherDerivedType>( "TypeB" );

	// elided

	// Resolve types by name
	std::shared_ptr<SomeType> AType = Container.resolve_by_name<SomeType>( "TypeA" );
	std::shared_ptr<SomeType> Btype = Container.resolve_by_name<SomeType>( "TypeB" );

	// elided 
}
```

FAQ:
----

Q) What happens if an exception is thrown during construction of complex types? If a constrcutor parameter has already been resolved and an exception is thrown in our target types constructor does a memory leak occur?

A) Due to the way in which the code is structured, objects which are newed and deletable i.e. not instance registrations, are automatically destructed before an exception reaches the outlying application.

Q) If I have a type which has unresolvable constructor arguments how can I fit this in with this IOC container?

A) This is where delgates come to the fore. The below example shows the registration of a type which requires both derivable and non-derivable types for constructor arguments.

```cpp
// Declare delegate which requires a derivable type
// as a constructor argument
static SomeType *GetSomeTypeInstance( std::shared_ptr<Foo> SomeFoo )
{
	return new SomeDerivedType( "MyNonDerivableParam", 10, 12, SomeFoo );
}

void RegisterAndResolve()
{
	// Register a Bar which implements Foo
	Container.register_type<Foo, Bar>();
	// Register a custom delegate which requires a derivable type Foo.
	Container.register_delegate<SomeType, Foo>( &GetSomeTypeInstance );

	// elided
	
	// Resolve a new instance of SomeType. Internally the IOC container
	// will identify SomeType requires an instance of Foo, derive an
	// instance of Foo, finally call our GetSomeTypeInstance delegate
	// with our resolved instance of Foo.
	std::shared_ptr<SomeType> inst = Container.resolve<SomeType>();
	// Do something
	inst->DoSomething();
}
```

Q) Are there any unit tests? Where can I get examples of using the IOC container?

A) Yes there are unit tests. These unit tests provide a good way of learning how to configure the IOC container as they are designed to excercise all aspects of it.

The unit tests can be found in the sub-folder ./test. To build the unit tests you will need either Clang 3.0 installed or g++ 4.7. The unit test application is called TestApp and returns a non-zero result if any test fail. 

To build against the Clang compiler use the commandline below:

make clang

To build a g++ executable use the commandline below:

make gcc

If the compiler has troubles finding the necessary standard library includes you may need to massage the makefile.
