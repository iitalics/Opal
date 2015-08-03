#include "syntax/Scanner.h"
#include "syntax/Parse.h"
#include "env/Loader.h"
using namespace Opal;


int main ()
{
	SourceError::color = true;

	try
	{
		Env::Module::getCore();

		auto nm = Env::loadSource("tests/infer-test.opal");
		Env::finishModuleLoad();
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}