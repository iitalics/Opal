#include "syntax/Scanner.h"
#include "syntax/Parse.h"
#include "env/Loader.h"
using namespace Opal;


int main ()
{
	SourceError::color = true;

	try
	{
		Env::loadSource("tests/syntax-toplevel.opal");
		Env::finishModuleLoad();
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}