// Boost_Python_Build.cpp : Defines the entry point for the application.
//

char const* greet()
{
	return "hello, world";
}

#define BOOST_PYTHON_STATIC_LIB
#include <boost/python.hpp>

BOOST_PYTHON_MODULE(hello)
{
	using namespace boost::python;
	def("greet", greet);
}
