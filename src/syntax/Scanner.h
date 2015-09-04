#pragma once
#include "Span.h"
namespace Opal {
;

/*
 tokens are namespaced as to not 
 pollute the global namespace */
namespace Tokens {
;
enum
{
	END_OF_FILE,
	ID,
	POLYID,
	INT,
	REAL,
	LONG,
	STRING,
	CHAR,
	LBRACK, RBRACK,
	LCURL, RCURL,
	LPAREN, RPAREN,
	EQUAL, COLON,
	COMMA, DOT,
	SEMICOLON, DOUBLECOLON,
	ARROW,
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
	KW_for, // unused!
	KW_if,
	KW_iface,
	KW_impl,
	KW_in, // unusued!
	KW_let,
	KW_match,
	KW_module,
	KW_new,
	KW_not,
	KW_or,
	KW_pub,
	KW_return,
	KW_true,
	KW_type,
	KW_use,
	KW_while,
};
}

/* a token of any kind */
struct Token
{
	int kind;
	Span span;

	// token-specific value
	std::string string;
	union {
		Int_t val_int;
		Long_t val_long;
		Real_t val_real;
		Char_t val_char;
		bool val_bool;
	};

	// string representation of token based on 'kind'
	static std::string str (int kind);


	inline Token ()
		: kind(Tokens::END_OF_FILE) {}
	inline Token (int _kind, 
			const Span& _span)
		: kind(_kind), span(_span) {}
	~Token ();

	// compare to a member of the above enum
	//  for simplicity
	bool operator== (int _kind) const;
	bool operator!= (int _kind) const;

	// string representation of token
	std::string str () const;

	// create an error located at this token
	SourceError die (const std::string& msg,
		const std::vector<std::string>& others = {}) const;
};

class Scanner
{
public:
	enum {
		// the Opal parser requires only LL(2) but
		//  extra lookahead is provided just in case
		Lookahead = 3
	};

	// construct scanner from input data
	Scanner (const char* buffer, size_t size, const std::string& _filename);
	inline Scanner (const std::string& data, const std::string& _filename)
		: Scanner(data.c_str(), data.size(), _filename) {}

	// construct scanner from file
	explicit Scanner (const std::string& _filename);

	// get token in buffer
	// i = 0 is the leftmost token
	const Token& get (int i = 0);

	// discard and return the leftmost token
	//  and advance the parser
	Token shift ();

	// throw error if leftmost token is not of kind 'kind'
	void expect (int kind);
	void expect (const std::vector<int>& kinds);

	// { expect(t); shift() }
	Token eat (int kind);

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
	char _readChar ();
	char _readEscape ();
};


}
