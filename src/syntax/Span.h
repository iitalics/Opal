#pragma once
#include <opal.h>
#include <exception>

struct SpanFile
{
	std::string filename;
	char* data;
	size_t size;

	~SpanFile ();
};
using SpanFilePtr = std::shared_ptr<SpanFile>;

struct Span
{
	SpanFilePtr file;
	size_t pos;

	Span ();
	~Span ();
};

struct SourceError : public std::exception
{
	std::string message;
	std::vector<std::string> others;
	std::vector<Span> positions;

	static bool color;

	inline SourceError (const std::string& msg,
			const std::vector<std::string>& _others,
			const std::vector<Span>& _pos)
		: message(msg), 
		  others(_others),
		  positions(_pos),
		  str(nullptr)
		{ generate(); }

	inline SourceError (const std::string& msg, const Span& sp)
		: message(msg),
		  positions({ sp }),
		  str(nullptr)
		{ generate(); }

	inline SourceError (const std::string& msg, 
			const std::vector<std::string>& _others,
			const Span& sp)
		: message(msg),
		  others(_others),
		  positions({ sp }),
		  str(nullptr)
		{ generate(); }

	explicit inline SourceError (const std::string& msg,
			const std::vector<Span>& _pos = {})
		: message(msg),
		  positions(_pos),
		  str(nullptr)
		{ generate(); }

	SourceError (const SourceError& other);

	virtual ~SourceError ();

	virtual const char* what () const throw();
private:
	void generate ();
	char* str;
};