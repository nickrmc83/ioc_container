ioc_container
=============

A C++ IOC container capable of constructor dependency injection and runtime registration of types. It is possible to register types, delegate objects and instances which maybe resolved later within an application. Due to the runtime nature of registrations it is possible to both add and remove registrations on an adhoc basis.

The source is known to both build and work when compiled with g++ 4.7 and Clang 3.0 C++ compilers. It uses a number of C++11 features including variadic templates and automatic type deduction and so requires the appropriate compiler switches to allow the use of such features e.g. -std=c++0x.

Tutorial:

Two simple examples of registering a types with and without any constrctor dependencies is outlined below. The example shows how a type bar derived from foo can be registered with the IOC container and later an instance can be resolved from the same container for use later. The example later shows how a type dah, which is derived from lardy and requires an instance of foo for constrction, can be registered, resolved and used.

int main(char **args, int argv)
{
    // Create an instance of an
    // ioc::conatianer
    ioc::container Container;

    // Register bar which is derived
    // from foo
    Container.register<foo *, bar *>();

    // elided

    // Resolve a new instance of foo
    foo *fooInstance = Container.resolve<foo *>();
    if( fooInstance )
    {
        // Call a method on our resolved
        // instance
        fooInstance->Call();

        // clean up
        delete fooInstance;
    }

    // Register dah which is derived
    // from lardy which requires an
    // instance of foo in construction
    Container.Register<lardy *, dah *, foo *>();

    // elided

    // Resolve a new instance of lardy
    lardy *lardyInstance = Container.Resolve<Lardy *>();
    if( lardyInstance )
    {
        // Call some method on our resolved
        // instance
        lardyInstance->Call();

        // clean up
        delete lardyInstance;
    }

    return 0;
};

As well as being able to register types with dependant constructor parameters, it is also possible to register delgates (callable objects such as functions or classes which implement operator ()) and Instances (an instance in this context means registering a pre-constructed object which maybe resolved at a later date). Delegates like standard registrations can require dependendant types in their signature. For example, the below code illustrates how to register a delegate which requires the type foo which we register earlier.

static SomeType *DoSomething( foo *obj )
{
    SomeType *Result = NULL:
    if( obj )
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
    Container.RegisterDelegate<SomeType *, DoSomething, foo *>();

    // elided

    // Resolve a new instance of SomeType
    SomeType *inst = Container.Resolve<SomeType *>();
    if( inst )
    {
        // Call some method
        inst->DoSometing();
        // clean up
        delete inst;
    }
}

To register a specific instance of a class which can later be resolved the below code can be used. This is useful when a singleton is required.

void RegisterInstanceExample()
{
    // Register
    Container.RegisterInstance<SomeType *>( new SomeDerivedType() );

    // elided

    // Resolve our previously registered instance
    SomeType *inst = Container.Resolve<SomeType *>();
    if( inst )
    {
        inst->DoSomething();
        // Do not delete inst as it may have been
        // resolved and therefore used by another
        // class or function.
    }
}

Standard resoltuion (Resolve<Type *>()) searches for the first matching registered type in the IOC containers dependency list. However, it is not possible to register two identical types unless using named registration. Named registration allows multiple matching types to be registered with the caveat that each is accompanied by a name by which it maybe resolved. For example the below code will throw a RegistrationException when the second registration is attempted.

void RegisterSomeTypes()
{
    // First registration works fine.
    Container.Register<SomeType *, SomeDerivedType *>();
    // Subsequent registations of type SomeType * will
    // fail unless "named" registration is used.
    Container.Register<SomeType *, SomeOtherDerivedType *>(); // This throws an error!! 
}

To enable the above code to compile correctly named registration can be used. Name registartion is available when registering types, delegates or instances. See a below for a self-explanatory example of registering and resolving types by name.

void RegisterAndResolveSomeTypes()
{
    // Register with name "TypeA"
    Container.RegisterWithName<SomeType *, SomeDerivedType *>( "TypeA" );
    // Register the same type this time with "TypeB". Note if we attempted
    // to register another version of SomeType * with the same name ("TypeA")
    // Then we would get a RegistrationException.
    Container.RegisterWithName<SomeType *, SomeOtherDerivedType *>( "TypeB" );

    // elided

    // Resolve types by name
    SomeType *AType = Container.ResolveByName<SomeType *>( "TypeA" );
    SomeType *Btype = Container.ResolveByName<SomeType *>( "TypeB" );

    // elided 
}

FAQ:

Q) What happens if an exception is thrown during construction of complex types? If a constrcutor parameter has already been resolved and an exception is thrown in our target types constructor does a memory leak occur?

A) Due to the way in which the code is structured, objects which are newed and deletable i.e. not instance registrations, are automatically destructed before an exception reaches the outlying application.

Q) If I have a type which has unresolvable constructor arguments how can I fit this in with this IOC container?

A) This is where delgates come to the fore. The below example shows the registration of a type which requires both derivable and non-derivable types for constructor arguments.

// Declare delegate which requires derivable type
static SomeType *GetSomeTypeInstance( Foo *SomeFoo )
{
    return new SomeDerivedType( "MyNonDerivableParam", 10, 12, SomeFoo );
}

void RegisterAndResolve()
{
    // Register a Bar which implements Foo
    Container.Register<Foo *, Bar *>();
    // Register a custom delegate which requires a derivable type Foo.
    Container.RegisterDelegate<SomeType *, Foo *>( &GetSomeTypeInstance );

    // elided
    
    // Resolve a new instance of SomeType. Internally the IOC container
    // will identify SomeType requires an instance of Foo, derive an
    // instance of Foo, finally call our GetSomeTypeInstance delegate
    // with our resolved instance of Foo.
    SomeType *inst = Container.Resolve<SomeType *>();
    if( inst )
    {
        // Do something
        inst->DoSomething();

        // tidy up
        delete inst;
    }

}

