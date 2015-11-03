#include <iostream>
#include "env/Loader.h"
#include "infer/Analysis.h"
#include "runtime/Cell.h"
#include "runtime/Exec.h"
using namespace Opal;

#define VERSION_STRING "0.9.3"

static std::string prgmName = "opal";
static void usage ()
{
	std::cout
		<< "usage: " << prgmName << " [options] <file> [<files>...]" << std::endl
		<< "  options:" << std::endl
		<< "    <file>            opal file to execute, containing main() function" << std::endl
		<< "    <files>...        additional opal files to execute" << std::endl
		<< "    -h, --help        show this help text" << std::endl
		<< "    -m <Module>       require module <Module>" << std::endl
		<< "    -P <path>         search '<path>/opal_libs' for modules" << std::endl
		<< "    --repl            use interactive prompt" << std::endl
		<< "    --nocolor         disable colored error messages" << std::endl

		/*
		<< "    --debug           enable debug mode" << std::endl
		<< "    --docs <path>     generate documentation files in <path>" << std::endl
		*/

		<< std::endl
		<< "Opal " << VERSION_STRING << ", github.com/iitalics/Opal" << std::endl;
}



using OptHandler = bool (*)(const std::string&);
struct Opt
{
	std::string flag;
	bool takesArg;
	OptHandler handler;
};



static bool OptHelp (const std::string& _)
{ usage(); return true; }

static bool OptPath (const std::string& path)
{ Env::searchPaths.insert(path); return false; }

static bool OptModule (const std::string& name)
{ Env::loadModule(name); return false; }

static bool OptNoColor (const std::string& _)
{ SourceError::color = false; return false; }

static bool interactive = false;
extern void runRepl ();

static bool OptInteractive (const std::string& _)
{ interactive = true; return false; }



static Env::Namespace* loadSources (const std::vector<std::string>& args, size_t& i)
{
	Env::Namespace* nmMain = nullptr;

	for (; i < args.size(); i++)
	{
		auto nm = Env::loadSource(args[i]);
		if (nmMain == nullptr)
			nmMain = nm;
	}

	return nmMain;
}

static bool parseOptions (const std::vector<std::string>& args, size_t& i)
{
	static std::vector<Opt> opts {
		{ "-h",        false, OptHelp },
		{ "--help",    false, OptHelp },
		{ "-P",        true,  OptPath },
		{ "-m",        true,  OptModule },
		{ "--repl",    false, OptInteractive },
		{ "--nocolor", false, OptNoColor },
	};

	while (i < args.size() && args[i][0] == '-')
	{
		auto opt = opts.begin();
		auto flag = args[i++];

		// find flag
		for (; opt != opts.end(); ++opt)
			if (opt->flag == flag)
				break;

		if (opt == opts.end())
			throw std::runtime_error("invalid option '" + flag + "'");

		// maybe read an argument
		std::string arg = "";
		if (opt->takesArg)
		{
			if (i >= args.size())
				throw std::runtime_error("option '" + flag + "' expects an argument");

			arg = args[i++];
		}

		// handle it appropriately
		if (opt->handler(arg))
			return true;
	}

	return false;
}



static void runMain (Env::Namespace* nm)
{
	// find 'main' function
	auto globMain = nm->getGlobal(AST::Name("main"));
	if (globMain == nullptr || !globMain->isFunc)
		throw SourceError("no main function defined");

	auto thread = Run::Thread::start();
	thread->call(globMain->func);

	// run threads until completion
	size_t count = 0;
	while (Run::Thread::stepAny()) count++;

	if (thread->size() == 0)
		std::cout << "empty stack!" << std::endl;
}


// main function here
int main (int argc, char** argv)
{
	if (argc < 1) return 2;

	SourceError::color = true;

	// convert c-strings into std::strings
	std::vector<std::string> args;
	for (int j = 1; j < argc; j++)
		args.push_back(std::string(argv[j]));

	try
	{
		// look for modules in the current directory
		Env::initSearchPaths(argv[0]);

		// parse command line options
		size_t i = 0;
		if (parseOptions(args, i))
			return 0;

		// load the source files
		auto nmMain = loadSources(args, i);
		if (nmMain == nullptr && !interactive)
		{
			usage();
			return 0;
		}
		Env::finishModuleLoad();

		// execute!
		if (interactive)
			runRepl();
		else
			runMain(nmMain);

		// bye
		// TODO: destroy everything?
	}
	catch (std::exception& err)
	{
		std::cerr << err.what() << std::endl;
		return 1;
	}
}