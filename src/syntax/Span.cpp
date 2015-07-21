#include <syntax/Span.h>
#include <cstring>


Span::Span (const std::string& _file, long int _pos)
	: file(_file), pos(_pos) {}
Span::~Span () {}



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
	size_t bol = 0, line = 0, col;

	std::ifstream fs(sp.file);
	if (!fs.good())
		return;

	char c;

	for (size_t i = 0; i < sp.pos; i++)
	{
		fs.get(c);
		if (c == '\n')
		{
			bol = i + 1;
			line++;
		}
	}
	col = sp.pos - bol;

	os << "in ";
	if (SourceError::color)
		os << COL_BOLD;
	os << sp.file << ":"
	   << (line + 1) << ":" // line #
	   << (col + 1) << ":"; // collumn #
	if (SourceError::color)
		os << COL_OFF;
	os << std::endl << " ";

	fs.seekg(bol, std::ios_base::beg);
	for (;;)
	{
		fs.get(c);
		if (c == '\n')
			break;
		else if (c == '\r')
			continue;
		else if (c == '\t')
			os << ' ';
		else
			os << c;
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