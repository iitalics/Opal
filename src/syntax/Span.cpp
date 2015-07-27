#include "Span.h"
#include <cstring>
namespace Opal {
;

Span::Span ()
	: file(nullptr), pos(0) {}
Span::~Span () {}
SpanFile::~SpanFile () 
{
	delete[] data;
}




bool SourceError::color = true;

SourceError::SourceError (const SourceError& other)
	: message(other.message),
	  others(other.others),
	  positions(other.positions),
	  str(nullptr)
{
	generate();
}

SourceError::~SourceError ()
{
	if (str != nullptr)
		delete[] str;
}

const char* SourceError::what () const throw()
{
	if (str == nullptr)
		return "undefined SourceError";
	else
		return str;
}

#define COL_BOLD "\x1b[1m"
#define COL_RED  "\x1b[31;1m"
#define COL_BLUE "\x1b[36;1m"
#define COL_OFF  "\x1b[0m"

static void writeSpan (std::ostringstream& os, const Span& sp)
{
	if (sp.file == nullptr)
		return;

	size_t bol = 0, line = 0, col;

	for (size_t i = 0; i < sp.pos; i++)
	{
		if (sp.file->data[i] == '\n')
		{
			bol = i + 1;
			line++;
		}
	}
	col = sp.pos - bol;

	os << "in ";
	if (SourceError::color)
		os << COL_BOLD;
	os << sp.file->filename << ":"
	   << (line + 1) << ":" // line #
	   << (col + 1) << ":"; // collumn #
	if (SourceError::color)
		os << COL_OFF;
	os << std::endl << " ";

	for (size_t i = bol; i < sp.file->size && sp.file->data[i] != '\n'; i++)
		switch (sp.file->data[i])
		{
		case '\t':
			os << ' ';
			break;
		case '\r':
		case '\n':
			break;
		default:
			os << sp.file->data[i];
		}

	os << std::endl << " ";
	for (size_t i = 0; i < col; i++)
		os << " ";
	if (SourceError::color)
		os << COL_BLUE;
	os << "^";
	if (SourceError::color)
		os << COL_OFF;
	os << std::endl;
}

void SourceError::generate ()
{
	std::ostringstream ss;

	for (auto& sp : positions)
		writeSpan(ss, sp);

	if (SourceError::color)
		ss << COL_RED;
	ss << "error: ";
	if (SourceError::color)
		ss << COL_OFF << COL_BOLD;
	ss << message;
	if (SourceError::color)
		ss << COL_OFF;
	ss << std::endl;
	for (auto& msg : others)
		ss << " " << msg << std::endl;
	
	auto res = ss.str();
	str = new char[res.size() + 1];
	memcpy(str, res.c_str(), res.size());
	str[res.size()] = '\0';
}


}
