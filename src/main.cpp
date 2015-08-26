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
static void proc_int_sub (Run::Thread& th)
{
	auto b = th.pop().dataInt;
	auto a = th.pop().dataInt;
	th.push(Run::Cell::Int(a - b));
}
static void proc_int_cmp (Run::Thread& th)
{
	auto b = th.pop().dataInt;
	auto a = th.pop().dataInt;
	th.push(Run::Cell::Int(a < b ? (-1) : (a > b) ? 1 : 0));
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

		auto fn_int_cmp = new Env::Function(Env::Function::NativeFunction, "cmp", core);
		fn_int_cmp->nativeFunc = proc_int_cmp;
		fn_int_cmp->args = { { "x", intType }, { "y", intType } };
		fn_int_cmp->ret = intType;
		fn_int_cmp->parent = core_int;

		auto fn_int_sub = new Env::Function(Env::Function::NativeFunction, "sub", core);
		fn_int_sub->nativeFunc = proc_int_sub;
		fn_int_sub->args = { { "x", intType }, { "y", intType } };
		fn_int_sub->ret = intType;
		fn_int_sub->parent = core_int;

		core_int->methods.push_back(fn_int_add);
		core_int->methods.push_back(fn_int_cmp);
		core_int->methods.push_back(fn_int_sub);

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

		size_t count = 0;
		while (thread.step()) count++;

		std::cout << "total commands: " << count << std::endl;
		if (thread.size() > 0)
		{
			auto res = thread.pop();
			std::cout << "result: " << res.str() << std::endl;
			res.release();
		}

		// bye
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}