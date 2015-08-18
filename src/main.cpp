#include "env/Loader.h"
#include "infer/Analysis.h"
#include "runtime/Cell.h"
#include "runtime/Exec.h"
using namespace Opal;


static void proc_int_add (Run::Thread& th)
{
	auto b = th.pop().dataInt;
	auto a = th.pop().dataInt;
	th.push(Run::Cell::Int(a + b));
}


int main ()
{
	SourceError::color = true;

	Run::Thread thread;
	try
	{
		auto core = Env::Module::getCore();
		Infer::Analysis::initTypes();
		Run::Cell::initTypes();

		auto core_int = core->getType("int");
		auto intType = Infer::Type::concrete(core_int, {});

		// define native functions
		//  TODO: refactor this
		auto fn_int_add = new Env::Function(Env::Function::NativeFunction, "add", core);
		fn_int_add->nativeFunc = proc_int_add;
		fn_int_add->args = { { "x", intType }, { "y", intType } };
		fn_int_add->ret = intType;
		fn_int_add->parent = core_int;
		core_int->methods.push_back(fn_int_add);

		// load some code
		auto nm = Env::loadSource("tests/runtime.opal");
		Env::finishModuleLoad();

		// find 'main' function
		auto global_main = nm->getGlobal(AST::Name("main"));
		if (global_main == nullptr)
			throw SourceError("no main function defined");

		// execute it in a new thread
		std::cout << "executing main" << std::endl;
		thread.call(global_main->func);
		while (thread.step()) ;

		// bye
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}