#define BOOST_PYTHON_STATIC_LIB
#include <boost/python.hpp>

char const* SayHello()
{
	return "Hello, from c++ dll!";
}
BOOST_PYTHON_MODULE(HelloExt)
{
	using namespace boost::python;
	def("SayHello", SayHello);
}