ioc_container
=============

A C++ IOC container capable of constructor dependency injection and runtime registration of types.

The source is known to both build and work when compiled with g++ 4.7 and Clang 3.0 C++ compilers.

A simple eaxmple of registering a type foo is outlined below. The example shows how a type bar derived from foo can be registered with the IOC container and later an instance can be resolved from the same container for use later.

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
    foo *NewInstance = Container.resolve<foo *>();
    if( NewInstance )
    {
        // Call a method on our resolved
        // instance
        NewInstance->Call();

        // clean up
        delete NewInstance;
    }

    return 0;
};
