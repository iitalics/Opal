#include "Exec.h"
#include "../env/Env.h"
#include "../syntax/Span.h"
#include <cstring>
namespace Opal { namespace Run {
;
/*
uncaught: XXX
  in Module::bar
  in (hidden)::foo
*/

#define TRACE_MAX_AFTER   8
#define TRACE_MAX_BEFORE  8


Error::~Error () { delete[] _what; }
const char* Error::what () const noexcept { return _what; }

Error::Error (const Error& other)
	: _what(new char[std::strlen(other._what) + 1])
{
	std::strcpy(_what, other._what);
}

Error::Error (const std::string& name,
		const std::vector<Cell>& args, Thread& th)
{
	std::ostringstream ss;

	if (SourceError::color)
		ss << "\x1b[31;1m";
	ss << "uncaught: ";
	if (SourceError::color)
		ss << "\x1b[0m";
	ss << name;
	if (!args.empty())
	{
		ss << ": ";
		for (size_t i = 0, len = args.size(); i < len; i++)
		{
			if (i > 0)
				ss << ", ";
			ss << args[i].str();
		}
	}
	if (SourceError::color)
		ss << "\x1b[0m";
	ss << std::endl;

	bool ellipsis = false;

	for (size_t i = 0, len = th._calls.size(); i < len; i++)
		if (i >= TRACE_MAX_AFTER && i < len - TRACE_MAX_BEFORE)
		{
			if (!ellipsis)
			{
				ellipsis = true;
				ss << "  ... " << (len - TRACE_MAX_AFTER - TRACE_MAX_BEFORE)
				   << " calls omitted" << std::endl;
			}
		}
		else
		{
			auto caller = th._calls[len - i - 1].caller;

			ss << "  in ";
			if (SourceError::color)
				ss << "\x1b[1m";
			ss << caller->fullname().str();
			if (SourceError::color)
				ss << "\x1b[0m";
			ss << std::endl;
		}

	// i LOVE c-strings
	auto len = ss.str().size();
	_what = new char[len + 1];
	std::memcpy(_what, ss.str().c_str(), len);
	_what[len] = '\0';
}

}}