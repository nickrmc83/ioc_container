#ifndef TEMPLATE_HELPERS_H
#define TEMPLATE_HELPERS_H
template<typename T>
struct TemplateHelper
{
	template<typename ...ArgTypes>
	static T Default( ArgTypes ...Args )
	{
		// This will be compiled out during optimisation.
		T Result( Args... );
		return Result;
	}

	template<typename ...ArgTypes>
	static T DefaultNew( ArgTypes ...Args )
	{
		// This will be compiled out during optimisation.
		return Default( Args... );
	}

	static void Destruct( T Object )
	{
		// This is a reference type so do not do anything.
		// This will be compiled out during optimisation.
	}
};

template<typename T>
struct TemplateHelper<T *>
{
    template<typename ...ArgTypes>
	static T *Default()
	{
		T *Result = NULL;
		return Result;
	}

	template<typename ...ArgTypes>
	static T *DefaultNew( ArgTypes ...Args )
	{
		// This will be compiled out during optimisation.
		return new T( Args... );
	}

	static void Destruct( T *Object )
	{
		// This is a pointer type so delete if not NULL
		if( Object )
		{
			delete Object;
			Object = NULL;
		}
	}
};
#endif //TEMPLATE_HELPERS_H
