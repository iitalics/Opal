#include <syntax/Scanner.h>
#include <cstring>

using namespace Tokens;


// TODO: lookup table
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
		   c == '$';
}
static char comment[] = "//";

using Seq = std::pair<std::string, int>;
static std::vector<Seq> seqs {
	Seq("[",  LBRACK),
	Seq("]",  RBRACK),
	Seq("(",  LPAREN),
	Seq(")",  RPAREN),
	Seq("{",  LCURL),
	Seq("}",  RCURL),
	Seq("=",  EQUAL),
	Seq(":",  COLON),
	Seq(",",  COMMA),
	Seq(".",  DOT),
	Seq(";",  SEMICOLON),
	Seq("+",  PLUS),
	Seq("-",  MINUS),
	Seq("*",  TIMES),
	Seq("/",  DIVIDE),
	Seq("%",  MODULO),
	Seq("::", CONS),
	Seq("<",  LT),
	Seq("<=", LE),
	Seq("==", EQ),
	Seq("!=", NE),
	Seq(">=", GE),
	Seq(">",  GR),
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
	Seq("new",      KW_new),
	Seq("not",      KW_not),
	Seq("or",       KW_or),
	Seq("return",   KW_return),
	Seq("true",     KW_true),
	Seq("type",     KW_type),
	Seq("while",    KW_while),
};


Token::~Token () {}

bool Token::operator== (int _kind)
{
	return kind == _kind;
}

SourceError Token::die (const std::string& msg,
		const std::vector<std::string>& others)
{
	if (span.file == nullptr)
		return SourceError(msg, others, std::vector<Span> {});
	else
		return SourceError(msg, others, span);
}



Scanner::Scanner (char* buffer, size_t size,
		const std::string& filename)
	: _file(new SpanFile { filename, new char[size], size }),
	  _pos(0)
{
	std::memcpy(_file->data, buffer, size);
	_init();
}

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



const Token& Scanner::get (int i)
{
	if (i < 0 || i >= Lookahead)
		throw SourceError("INTERNAL: trying to access token beyond lookahead");

	return _buf[i];
}
Token Scanner::shift ()
{
	for (size_t i = Lookahead; i-- > 1; )
		_buf[i] = _buf[i - 1];
	
	auto gone = _buf[0];
	_read(_buf[0]);
	return gone;
}


#define at(i)  (_file->data[_pos + i])
#define cur()  at(0)
#define left() (_file->size - _pos)
#define eof()  (left() <= 0)
#define adv()  ++_pos


void Scanner::_init ()
{
	for (size_t i = 0; i < Lookahead; i++)
		_read(_buf[i]);
}
void Scanner::_trim ()
{
	for (;;)
	{
		while (!eof() && is_space(cur()))
			_pos++;

		if (left() >= 2 &&
				at(0) == comment[0] &&
				at(1) == comment[1])
		{
			while (!eof() && cur() != '\n')
				adv();
		}
		else
			break;
	}
}
void Scanner::_read (Token& out)
{
	_trim();
	out.span.file = _file;
	out.span.pos = _pos;

	if (eof())
	{
		out.kind = END_OF_FILE;
		return;
	}
	else
		throw SourceError("invalid character", out.span);
}

