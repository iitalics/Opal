#pragma once
#include <opal.h>
#include <syntax/Span.h>

namespace Tokens {
enum
{
	END_OF_FILE,
	ID,
	POLYID,
	INT,
	REAL,
	LONG,
	STRING,
	LBRACK, RBRACK,
	LCURL, RCURL,
	LPAREN, RPAREN,
	EQUAL, COLON,
	COMMA, DOT,
	SEMICOLON,
	PLUS, MINUS,
	TIMES, DIVIDE,
	MODULO, CONS,
	LT, LE, EQ, NE, GE, GR,
	KW_and,
	KW_break,
	KW_continue,
	KW_else,
	KW_extern,
	KW_false,
	KW_fn,
	KW_for,
	KW_if,
	KW_iface,
	KW_impl,
	KW_in,
	KW_let,
	KW_new,
	KW_not,
	KW_or,
	KW_return,
	KW_true,
	KW_type,
	KW_while,
};
}

struct Token
{
	int kind;
	Span span;

	std::string string;
	union {
		oint val_int;
		olong val_long;
		oreal val_real;
		bool val_bool;
	};


	inline Token ()
		: kind(Tokens::END_OF_FILE) {}
	inline Token (int _kind, 
			const Span& _span)
		: kind(_kind), span(_span) {}
	inline Token (int _kind,
			const std::string& str,
			const Span& _span)
		: kind(_kind), span(_span), string(str) {}
	template <typename T>
	inline Token (int _kind, T data, const Span& _span)
		: kind(_kind), span(_span)
	{
		*((T*) &val_bool) = data;
	}
	~Token ();

	bool operator== (int _kind);

	SourceError die (const std::string& msg,
		const std::vector<std::string>& others = {});
};

class Scanner
{
public:
	enum {
		Lookahead = 3
	};

	Scanner (char* buffer, size_t size, const std::string& _filename);
	explicit Scanner (const std::string& _filename);

	const Token& get (int i = 0);
	Token shift ();

private:
	SpanFilePtr _file;
	long int _pos;

	Token _buf[Lookahead];
	void _init ();

	void _trim ();
	void _read (Token& out);
	void _readId (Token& out);
	void _readNumber (Token& out);
	void _readString (Token& out);
};