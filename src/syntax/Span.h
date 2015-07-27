#pragma once
#include "../opal.h"
#include <exception>
namespace Opal {
;

/*
 struct for representing the contents of a file
  (or input buffer)
 also used to find line # in error messages  */
struct SpanFile
{
	std::string filename;
	char* data;
	size_t size;

	~SpanFile ();
};
using SpanFilePtr = std::shared_ptr<SpanFile>;


/*
 a span represents a location in source code.
 it is used for pretty error messsages */
struct Span
{
	SpanFilePtr file;
	size_t pos;

	inline Span (SpanFilePtr _file, size_t _pos)
		: file(_file), pos(_pos) {}
	Span ();
	~Span ();
};

/*
 exception class thrown by most functions in Opal
 */
struct SourceError : public std::exception
{
	// show coloreful error messages?
	static bool color;


	// primary error messages
	std::string message;
	// additional lines of info
	std::vector<std::string> others;
	// locations that the error occured
	std::vector<Span> positions;

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


}
