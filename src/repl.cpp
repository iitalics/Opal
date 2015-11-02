#include <iostream>
#include "env/Loader.h"
#include "syntax/Parse.h"
#include "syntax/Desugar.h"
#include "code/CodeGen.h"
#include "runtime/Exec.h"
using namespace Opal;

static Env::Function* replGenerate (const std::string& string,
		Env::Namespace* nm, Env::Module* module,
		Run::ThreadPtr thread)
{
	// parse
	Scanner scan(string, "<input>");
	if (scan.get() == Tokens::END_OF_FILE)
		return nullptr;

	auto ast = Parse::parseExp(scan);
	scan.expect(Tokens::END_OF_FILE);
	Desugar::desugar(ast);

	// create
	auto fn = new Env::Function(Env::Function::CodeFunction,
		"<input>", module);
	fn->nm = nm;
	fn->body = ast;

	// analyze
	fn->infer();
	fn->code = Code::CodeGen::generate(fn);

	return fn;
}

static void codeDump (const Run::Cell& data)
{
	auto fn = data.ctor;
	auto obj = data.simple;

	std::cout << "=== Code Dump ===" << std::endl;

	if (obj != nullptr && !obj->children.empty())
	{
		std::cout << "  closure:" << std::endl;
		for (auto& val : obj->children)
			std::cout << "    " << val.str(SourceError::color) << std::endl;
	}

	if (fn->codegen)
		fn->codegen->showCode();
	else
		std::cout << "<no code available>" << std::endl;
}

void runRepl ()
{
	if (SourceError::color)
		std::cout << "\x1b[1m";
	std::cout << "@@@  Opal interactive prompt  @@@" << std::endl;
	if (SourceError::color)
		std::cout << "\x1b[0m";
	std::cout << "  enter !help for a list of commands" << std::endl
	          << std::endl;

	// create a namespace and private module
	auto module = new Env::Module("(input)");
	auto nm = new Env::Namespace(module, module);
	Env::Function* fn;

	bool opt_types = false;
	bool opt_codedump = false;

	while (!std::cin.eof())
	{
		// Read
		std::cout << ">> ";
		std::cout.flush();

		std::string input;
		std::getline(std::cin, input);

		if (input == "!help")
		{
			std::cout << "commands: " << std::endl
			          << "  !help       show this help text" << std::endl
			          << "  !types      show expression types" << std::endl
			          << "  !dump       dump function byte code" << std::endl
			          << "  !debug      enable some debug printing" << std::endl;
			continue;
		}
		if (input == "!types")
		{
			opt_types = !opt_types;
			continue;
		}
		if (input == "!dump")
		{
			opt_codedump = !opt_codedump;
			continue;
		}
		if (input == "!debug")
		{
			Infer::Analysis::debuggingEnabled =
				!Infer::Analysis::debuggingEnabled;
			continue;
		}

		Run::ThreadPtr thread(nullptr);

		try
		{
			// Eval
			fn = replGenerate(input, nm, module, thread);
			if (fn != nullptr)
			{
				thread = Run::Thread::start();
				thread->call(fn);
				while (Run::Thread::stepAny()) ;
			}
		}
		catch (std::exception& err)
		{
			std::cerr << err.what() << std::endl;
			if (thread)
				Run::Thread::stop(thread);
			continue;
		}

		if (!thread) continue;

		// Print
		auto res = thread->pop();
		std::cout << res.str(SourceError::color);
		if (opt_types)
			std::cout << " : " << fn->ret->str();
		std::cout << std::endl;
		if (res.type->isFunction() && opt_codedump)
			codeDump(res);

		res.release();
		delete fn;

		// Loop
	}
	std::cout << std::endl;
}
