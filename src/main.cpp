#include <iostream>
#include "env/Loader.h"
#include "infer/Analysis.h"
#include "runtime/Cell.h"
#include "runtime/Exec.h"
using namespace Opal;


// example custom runtime type
//  runtime objects:
//  - must inheret from GC::Object to be garbage collected
//  - optionally overloads `void markChildren ()`
struct Foo : public GC::Object
{
	static std::vector<std::string> foos;
	static Env::Type* type;

	Foo () : i(0) {}
	virtual ~Foo () {}

	// some methods
	Run::Cell advance ()
	{
		std::cout << "===> " << foos[i] << std::endl;
		i = (i + 1) % foos.size();
		return Run::Cell::Unit();
	}
	Run::Cell str () { return Run::Cell::String(foos[i]); }

	int i;
};
std::vector<std::string> Foo::foos {
	"foo", "bar", "baz"
};
Env::Type* Foo::type = nullptr;


// constructor
static void foo (Run::Thread& th)
{
	auto a = Run::Cell::Object(Foo::type, new Foo());
	th.push(a);
	a.release();
}
// 'linker functions' for object methods
static void foo_advance (Run::Thread& th)
{
	auto a = th.pop();
	auto foo = (Foo*) (a.obj);
	th.push(foo->advance());
	a.release();
}
static void foo_str (Run::Thread& th)
{
	auto a = th.pop();
	auto foo = (Foo*) (a.obj);
	th.push(foo->str());
	a.release();
}

// package loader
static void loadPackage (Env::Package& pkg)
{
	// get type from module
	auto fooMod = Env::Module::get("Test");
	if (!(Foo::type = fooMod->getType("foo")))
		throw SourceError("no foo type!");

	// export functions
	pkg
	.put("foo", foo)
	.put("foo.advance", foo_advance)
	.put("foo.str", foo_str);

	std::cout << "load package '" << pkg.name() << "' is a success" << std::endl;
}

// package request
static Env::PackageLoad _1("opal.test", loadPackage);



int main ()
{
	SourceError::color = true;

	Run::Thread thread;
	try
	{
		// initialize
		// TODO: group this
		Infer::Analysis::initTypes();
		Run::Cell::initTypes();

		// load some code
		auto nm = Env::loadSource("tests/external.opal");
		Env::finishModuleLoad();

		// find 'main' function
		auto global_main = nm->getGlobal(AST::Name("main"));
		if (global_main == nullptr)
			throw SourceError("no main function defined");

		// execute it in a new thread
		std::cout << "executing main" << std::endl;
		size_t count = 0;
		thread.call(global_main->func);
		while (thread.step()) count++;

		// log output
		std::cout << "total commands: " << count << std::endl;
		if (thread.size() > 0)
		{
			auto res = thread.pop();
			std::cout << "result: " << res.str() << std::endl;
			res.release();
		}
		else
			std::cout << "empty stack!" << std::endl;

		// bye
		// TODO: destroy everything?
	}
	catch (SourceError& err)
	{
		std::cerr << err.what() << std::endl;
	}
}