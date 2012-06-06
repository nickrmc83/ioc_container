ioc_container
=============

A C++ IOC container capable of constructor dependency injection and runtime registration of types.

The source is known to both build and work when compiled with g++ 4.7 and Clang 3.0 C++ compilers. It uses a number of c++11 features including variadic templates and automatic type deduction.

Two simple eaxmples of registering a types with and without any constrctor dependencies si outlined below. The example shows how a type bar derived from foo can be registered with the IOC container and later an instance can be resolved from the same container for use later. The example later shows how a type dah, which is derived from lardy and requires an instance of foo for constrction, can be registered, resolved and used.

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

As well as being able to register types with dependant constructor parameters, it is also possible to register delgates (callable objects such as functions or classes which implement operator ()) and Instances (an instance in this context means registering a pre-constructed object which maybe resolved at a later date). Delegates like standard registrations can require dependendant types in the signature. For example, the below code illustrates how to register a delegate which requires the type foo which we register earlier.

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

    // Resolve an instance of SomeType
    SomeType *inst = Container.Resolve<SomeType *>();
    if( inst )
    {
        // Call some method
        inst->DoSometing();
        // clean up
        delete inst;
    }
}
