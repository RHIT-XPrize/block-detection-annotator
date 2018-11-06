// Boost_Python_Deploy.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#define BOOST_PYTHON_STATIC_LIB
#include <boost/python.hpp>

struct World
{
	char const* SayHello()
	{
		return "Hello, from c++ dll!";
	}
};

BOOST_PYTHON_MODULE(HelloExt)
{
	using namespace boost::python;
	class_<World>("World")
		.def("SayHello", &World::SayHello)
		;
}
