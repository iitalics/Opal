#include "syntax/Scanner.h"
#include "syntax/Parse.h"
#include "env/Loader.h"
using namespace Opal;


int main ()
{
	SourceError::color = true;

	try
	{
		// bake types
		auto core = Env::Module::getCore();
		core->types.push_back(new Env::Type("int", core, 0, false));
		core->types.push_back(new Env::Type("bool", core, 0, false));
		core->types.push_back(new Env::Type("real", core, 0, false));
		core->types.push_back(new Env::Type("string", core, 0, false));

		auto nm = Env::loadSource("tests/infer-test.opal");
		Env::finishModuleLoad();
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}