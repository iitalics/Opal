#include <iostream>
#include "env/Loader.h"
#include "infer/Analysis.h"
#include "runtime/Cell.h"
#include "runtime/Exec.h"
using namespace Opal;

static std::string prgmName;
static void usage ()
{
	std::cout
		<< "usage: " << prgmName << " [options] <file> [<files>...]" << std::endl
		<< "  options:" << std::endl
		<< "    <file>            opal file to execute, containing main() function" << std::endl
		<< "    <files>...        additional opal files to execute" << std::endl
		<< "    -h, --help        show this help text" << std::endl
//		<< "    -m <Module>       require module <Module>" << std::endl
		<< "    -P <path>         search '<path>/opal_libs' for modules" << std::endl
		;
		/*
		<< "    --debug           enable debug mode" << std::endl
		<< "    --docs <path>     generate documentation files in <path>" << std::endl
		*/
}


static bool OptHelp (const std::string& _)
{ usage(); return true; }
static bool OptPath (const std::string& path)
{ Env::searchPaths.insert(path); return false; }
static bool OptModule (const std::string& name)
{ Env::loadModule(name); return false; }

static Env::Namespace* loadSources (const std::vector<std::string>& args, size_t& i) {
	Env::Namespace* nmMain = nullptr;

	for (; i < args.size(); i++)
	{
		auto nm = Env::loadSource(args[i]);
		if (nmMain == nullptr)
			nmMain = nm;
	}

	return nmMain;
}

using OptHandler = bool (*)(const std::string&);
struct Opt
{
	std::string flag;
	bool takesArg;
	OptHandler handler;
};

int main (int argc, char** argv)
{
	if (argc < 1) return 2;

	SourceError::color = true;

	std::vector<std::string> args;
	for (int j = 1; j < argc; j++)
		args.push_back(std::string(argv[j]));

	std::vector<Opt> opts {
		{ "-h",     false, OptHelp },
		{ "--help", false, OptHelp },
		{ "-P",     true,  OptPath },
//		{ "-m",     true,  OptModule }
	};

	size_t i = 0;
	while (i < args.size() && args[i][0] == '-')
	{
		auto opt = opts.begin();
		auto flag = args[i++];

		for (; opt != opts.end(); ++opt)
			if (opt->flag == flag)
				break;

		if (opt == opts.end())
		{
			std::cout << "invalid option '" << flag << "'" << std::endl;
			return 1;
		}

		std::string arg = "";
		if (opt->takesArg)
		{
			if (i >= args.size())
			{
				std::cout << "option '" << flag << "' expects an argument" << std::endl;
				return 1;
			}
			arg = args[i++];
		}

		if (opt->handler(arg))
			return 0;
	}

	try
	{
		// look for modules in the current directory
		Env::initSearchPaths(argv[0]);

		// load some code
		auto nmMain = loadSources(args, i);
		if (nmMain == nullptr)
		{
			usage();
			return 1;
		}
		Env::finishModuleLoad();

		// find 'main' function
		auto globMain = nmMain->getGlobal(AST::Name("main"));
		if (globMain == nullptr || !globMain->isFunc)
			throw SourceError("no main function defined");

		// execute it in a new thread
		auto thread = Run::Thread::start();
		thread->call(globMain->func);
//		std::cout << "executing main" << std::endl;

		// run threads until completion
		size_t count = 0;
		while (Run::Thread::stepAny()) count++;

		// log output
//		std::cout << "total commands: " << count << std::endl;
		if (thread->size() > 0)
		{
			auto res = thread->pop();
//			std::cout << "result: " << res.str() << std::endl;
			res.release();
		}
		else
			std::cout << "empty stack!" << std::endl;

		// bye
		// TODO: destroy everything?
	}
	catch (std::exception& err)
	{
		std::cerr << err.what() << std::endl;
		return 1;
	}
}