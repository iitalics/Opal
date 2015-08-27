#include "Scanner.h"
#include <cstring>
namespace Opal {
;

using namespace Tokens;

// syntax constants
#define POLY_ID_CHAR  '#'
#define QUOTE_CHAR    '\"'
#define SQUOTE_CHAR   '\''
#define LONG_CHAR     'L'
static char COMMENT[] = "//";
static char HEX[] = "0x";


// TODO: lookup table to speed these up
static bool is_space (char c) {
	return c == ' ' || c == '\n' ||
	       c == '\t' || c == '\r' ||
	       c == '\b';
}
static bool is_digit (char c) {
	return c >= '0' && c <= '9';
}
static bool is_ident (char c) {
	return (c >= 'a' && c <= 'z') ||
		   (c >= 'A' && c <= 'Z') ||
		   (c >= '0' && c <= '9') ||
		   c == '_' || c == '?' ||
		   c == '@' || c == '\'';
}


// list different token formats
using Seq = std::pair<std::string, int>;
static std::vector<Seq> seqs {
	Seq("[",  LBRACK),
	Seq("]",  RBRACK),
	Seq("(",  LPAREN),
	Seq(")",  RPAREN),
	Seq("{",  LCURL),
	Seq("}",  RCURL),
	Seq("::", DOUBLECOLON),
	Seq(":",  COLON),
	Seq(",",  COMMA),
	Seq(".",  DOT),
	Seq(";",  SEMICOLON),
	Seq("+",  PLUS),
	Seq("->", ARROW),
	Seq("-",  MINUS),
	Seq("*",  TIMES),
	Seq("/",  DIVIDE),
	Seq("%",  MODULO),
	Seq("$",  CONS),
	Seq("<=", LE),
	Seq("<",  LT),
	Seq(">=", GE),
	Seq(">",  GR),
	Seq("==", EQ),
	Seq("=",  EQUAL),
	Seq("!=", NE),
};
static std::vector<Seq> keywords {
	Seq("and",      KW_and),
	Seq("break",    KW_break),
	Seq("continue", KW_continue),
	Seq("else",     KW_else),
	Seq("extern",   KW_extern),
	Seq("false",    KW_false),
	Seq("fn",       KW_fn),
	Seq("for",      KW_for),
	Seq("if",       KW_if),
	Seq("iface",    KW_iface),
	Seq("impl",     KW_impl),
	Seq("in",       KW_in),
	Seq("let",      KW_let),
	Seq("module",   KW_module),
	Seq("new",      KW_new),
	Seq("not",      KW_not),
	Seq("or",       KW_or),
	Seq("pub",      KW_pub),
	Seq("return",   KW_return),
	Seq("true",     KW_true),
	Seq("type",     KW_type),
	Seq("use",      KW_use),
	Seq("while",    KW_while),
};


Token::~Token () {}
bool Token::operator== (int _kind) const { return kind == _kind; }
bool Token::operator!= (int _kind) const { return kind != _kind; }
SourceError Token::die (const std::string& msg,
		const std::vector<std::string>& others) const
{
	if (span.file == nullptr)
		return SourceError(msg, others, std::vector<Span> {});
	else
		return SourceError(msg, others, span);
}


// quote format
// alternatives:   `foo'
//                 'foo'
//                 "foo"
#define LQ "`"
#define RQ "`"

std::string Token::str (int kind)
{
	switch (kind)
	{
	case END_OF_FILE: return LQ "<eof>" RQ;
	case ID:          return LQ "<ident>" RQ;
	case POLYID:      return LQ "#<ident>" RQ;
	case REAL:        return LQ "<number/real>" RQ;
	case LONG:        return LQ "<number/long>" RQ;
	case INT:         return LQ "<number>" RQ;
	case STRING:      return LQ "<string>" RQ;
	case CHAR:        return LQ "<char>" RQ;
	default: break;
	}

	if (kind < KW_and)
	{
		for (auto& seq : seqs)
			if (seq.second == kind)
				return LQ + seq.first + RQ;
	}
	else
	{
		for (auto& kw : keywords)
			if (kw.second == kind)
				return "keyword " LQ + kw.first + RQ;
	}

	return "?";
}
std::string Token::str () const
{
	if (kind == ID)
		return LQ + string + RQ;
	if (kind == CHAR)
	{
		std::ostringstream ss;
		ss << "\'" << val_char << "\'";
		return ss.str();
	}
	else if (kind == POLYID)
		return LQ "#" + string + RQ;
	else
		return str(kind);
}




// create scanner based on buffer
Scanner::Scanner (const char* buffer, size_t size,
		const std::string& filename)
	: _file(new SpanFile { filename, new char[size], size }),
	  _pos(0)
{
	std::memcpy(_file->data, buffer, size);
	_init();
}

// create scanner from file
Scanner::Scanner (const std::string& filename)
	: _pos(0)
{
	std::ifstream fs(filename);

	if (!fs.good())
	{
		std::ostringstream ss;
		ss << "unable to read file `" << filename << "'";
		throw SourceError(ss.str());
	}

	fs.seekg(0, std::ios_base::end);
	size_t size = fs.tellg();

	_file = SpanFilePtr(new SpanFile {
		filename,
		new char[size],
		size
	});

	fs.seekg(0, std::ios_base::beg);
	fs.read(_file->data, size);

	_init();
}



// utilities
// character at offset 'i'
#define at(i)  (_file->data[_pos + i])
// advance stream 'n' characters
#define adv(n) _pos += n
// # of characters left in file
#define left() (_file->size - _pos)
// reached eof?
#define eof()  (left() == 0)

void Scanner::_init ()
{
	for (size_t i = 0; i < Lookahead; i++)
		_read(_buf[i]);
}

// trim spaces and comments
void Scanner::_trim ()
{
	// ideally implemented with recursion
	//  but the for loop just to ensure "tail calls"
	/*
	fn trim ()
		while is_space(cur)
			adv

		if is_comment()
			while cur != '\n'
				adv
			trim
	*/
	for (;;)
	{
		while (!eof() && is_space(at(0)))
			adv(1);

		if (left() >= 2 &&
				at(0) == COMMENT[0] &&
				at(1) == COMMENT[1])
		{
			while (!eof() && at(0) != '\n')
				adv(1);
		}
		else
			break;
	}
}

// read a token from file
void Scanner::_read (Token& out)
{
	_trim();
	out.span.file = _file;
	out.span.pos = _pos;

	if (eof())
		out.kind = END_OF_FILE;
	else if (at(0) == QUOTE_CHAR)
		_readString(out);
	else if (at(0) == SQUOTE_CHAR)
	{
		// '<char>'
		adv(1);
		out.kind = CHAR;
		out.val_char = _readChar();
		if (at(0) != SQUOTE_CHAR)
			throw SourceError("excess characters in char literal",
				Span(_file, _pos));
		adv(1);
	}
	else if (is_digit(at(0)) || (at(0) == '.' && is_digit(at(1))))
		_readNumber(out);
	else if (is_ident(at(0)) || at(0) == POLY_ID_CHAR)
		_readId(out);
	else
	{
		for (auto& seq : seqs)
		{
			auto& name = seq.first;
			int token = seq.second;
			size_t size = name.size();

			if (left() < size)
				continue;

			for (size_t i = 0; i < size; i++)
				if (at(i) != name[i])
					goto again;

			out.kind = token;
			adv(size);
			return;
		again:;
		}
		throw SourceError("invalid character", Span(_file, _pos));
	}
}

// read ID or POLYID or keyword
void Scanner::_readId (Token& out)
{
	if (at(0) == POLY_ID_CHAR)
	{
		adv(1);
		out.kind = POLYID;
	}
	else
		out.kind = ID;

	size_t len;
	std::ostringstream ss;
	for (len = 0; !eof() && is_ident(at(len)); len++)
		ss << at(len);
	adv(len);

	if (len == 0)
		throw SourceError("invalid empty identifier",
			Span(_file, _pos));

	// check if identifier is a keyword
	if (out.kind != POLYID)
		for (auto& kw : keywords)
			if (kw.first == ss.str())
			{
				out.kind = kw.second;
				return;
			}

	// it's not
	out.string = ss.str();
}

// convert char to digit in base
#define BAD_DIGIT  -1
static int digit (char c, int base)
{
	if (c >= '0' && c <= '9')
		return int(c - '0');
	if (base > 10)
	{
		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;
	}
	return BAD_DIGIT;
}

/*
Handles base-10, hex, 'L' extension for long, 
 floating point numbers

Example		Type
1234		INT
0123		INT
123L		LONG
0xA1B2		INT
0xA1B2L		LONG
12.34		REAL
12.			REAL
.12			REAL
*/
void Scanner::_readNumber (Token& out)
{
	// read identifiers in a row
	size_t len = 0;
	while (is_ident(at(len)))
		len++;

	int kind = INT;
	long long int num = 0;
	double fp = 0.0;
	int base = 10;

	// check for hex prefix
	if (len > 2 &&
			at(0) == HEX[0] &&
			at(1) == HEX[1])
	{
		base = 16;
		len -= 2;
		adv(2);
	}

	// check for long suffix
	if (len > 1 && at(len - 1) == LONG_CHAR)
	{
		len--;
		kind = LONG;
	}

	// parse into an actual integer
	for (size_t i = 0; i < len; i++)
	{
		auto dig = digit(at(0), base);
		if (dig == BAD_DIGIT)
			goto bad_digit;

		num = (num * base) + dig;
		adv(1);
	}

	// advance the 'L'
	if (kind == LONG)
	{
		adv(1);
	}
	else if (base == 10 && at(0) == '.' && 
				(is_digit(at(1)) || !is_ident(at(1))))
	{
		// if seen a '.' the do floating point
		adv(1);

		double n = 0, d = 1;
		while (is_ident(at(0)))
		{
			auto dig = digit(at(0), 10);
			if (dig == BAD_DIGIT)
				goto bad_digit;

			n = (n * 10) + dig;
			d = (d * 10);
			adv(1);
		}
		fp = n / d;
		kind = REAL;
	}

	// we're done here
	// TODO: check bounds
	out.kind = kind;
	if (kind == INT)
		out.val_int = num;
	else if (kind == LONG)
		out.val_long = num;
	else if (kind == REAL)
		out.val_real = fp + num;

	return;
bad_digit:
	throw SourceError("invalid numeric digit", Span(_file, _pos));
}

/*
string scanner supports standard escape sequences:
	\n -> newline
	\r -> cr
	\t -> tab
	\0 -> null
	\\ -> backslash
	\', \" -> corresponding quotes
	\x## -> hexidecimal

no \b  no \f  no \/
those are all pretty dated escape sequences

TODO: unicode or nah?
*/
void Scanner::_readString (Token& out)
{
	char quote = at(0);
	size_t start = _pos;

	std::ostringstream ss;

	adv(1);
	while (at(0) != quote)
	{
		if (eof())
			throw SourceError("unclosed quote in string",
				Span(_file, start));
		ss << _readChar();
	}
	adv(1);

	out.kind = STRING;
	out.string = ss.str();
}
char Scanner::_readChar ()
{
	if (at(0) == '\\')
	{
		adv(1);
		return _readEscape();
	}
	else
	{
		auto res = at(0);
		adv(1);
		return res;
	}
}
char Scanner::_readEscape ()
{
	char c;
	size_t start = _pos;

	if (left() < 1)
		goto bad_escape;

	c = at(0);
	adv(1);
	switch (c)
	{
	case 'n':  return '\n';
	case 'r':  return '\r';
	case 't':  return '\t';
	case '0':  return '\0';
	case '\\': return '\\';
	case '\"': return '\"';
	case '\'': return '\'';
	case 'x':
		if (left() < 2)
			goto bad_escape;
		else
		{
			auto a = digit(at(0), 16);
			auto b = digit(at(1), 16);
			if (a == BAD_DIGIT || b == BAD_DIGIT)
				goto bad_escape;
			adv(2);
			return (char)(a * 16 + b);
		}

	default: break;
	}

bad_escape:
	throw SourceError("invalid escape sequence", Span(_file, start));
}







const Token& Scanner::get (int i)
{
	if (i < 0 || i >= Lookahead)
		throw SourceError("INTERNAL: trying to access token beyond lookahead");

	return _buf[i];
}
Token Scanner::shift ()
{
	auto gone = _buf[0];

	for (size_t i = 0; i < Lookahead - 1; i++)
		_buf[i] = _buf[i + 1];

	_read(_buf[Lookahead - 1]);
	return gone;
}

void Scanner::expect (int kind)
{
	expect(std::vector<int> { kind });
}
void Scanner::expect (const std::vector<int>& kinds)
{
	auto cur = get().kind;
	for (auto& k : kinds)
		if (cur == k)
			return;

	std::ostringstream ss;
	ss << "expected ";
	for (size_t i = 0, len = kinds.size(); i < len; i++)
	{
		if (i > 0)
		{
			if (i == len - 1)
				ss << " or ";
			else
				ss << ", ";
		}
		ss << Token::str(kinds[i]);
	}
	throw get().die(ss.str());
}
Token Scanner::eat (int kind)
{
	expect({ kind });
	return shift();
}


}
