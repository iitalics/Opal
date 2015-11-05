#include "Parse.h"
#include "../Names.h"
namespace Opal { namespace Parse {
;

using namespace AST;
using namespace Tokens;

static void parseUse (Toplevel& top, Scanner& scan);
static void parseModule (Toplevel& top, Scanner& scan);
static void parseImpl (Toplevel& top, Scanner& scan);

static DeclPtr parseFuncDecl (Scanner& scan, const Var& impl = {"", nullptr});
static DeclPtr parseTypeDecl (Scanner& scan);
static DeclPtr parseIFaceDecl (Scanner& scan);
static DeclPtr parsePublic (Scanner& scan);

static ExpPtr parseExp (Scanner& scan, int prec);
static ExpPtr parseTerm (Scanner& scan);
static ExpPtr parseConstant (Scanner& scan);
static ExpPtr parseSuffix (Scanner& scan, ExpPtr e);
static ExpPtr parseBlockExp (Scanner& scan);
static ExpPtr parseCond (Scanner& scan, bool req_else);
static ExpPtr parseLambda (Scanner& scan);
static ExpPtr parseLambdaSuffix (Scanner& scan);
static ExpPtr parseObject (Scanner& scan);
static bool parseStmt (Scanner& scan, ExpList& exps, bool& unit);
static bool parseFinalStmt (Scanner& scan, ExpList& exps);
static ExpPtr parseWhileLoop (Scanner& scan);
static ExpPtr parseAssign (Scanner& scan, bool& unit);
static ExpPtr parseLet (Scanner& scan);
static ExpPtr parseMatch (Scanner& scan);





/*
given <T>, <L>, <R>, Opt

if Opt
	<L> [<T> {"," <T>}] <R>
else
	<L> <T> {"," <T>} <R>
*/
template <typename T>
using ParserFn = T (*) (Scanner&);
template <typename T>
static std::vector<T> commaList (Scanner& scan, ParserFn<T> parse_it,
		int left, int right, bool optional, Span& span)
{
	std::vector<T> result;
	span = scan.eat(left).span;

	if (!(optional && scan.get() == right))
		for (;;)
		{
			result.push_back(parse_it(scan));

			scan.expect({ right, COMMA });
			if (scan.get() == COMMA)
				scan.shift();
			else
				break;
		}

	scan.eat(right);
	return result;
}
template <typename T>
inline static std::vector<T> commaList (Scanner& scan, ParserFn<T> parse_it,
		int left, int right, bool opt = true)
{
	Span dummy;
	return commaList<T>(scan, parse_it, left, right, opt, dummy);
}





/*
var:
	ID ":" type
*/
static Var parseVar (Scanner& scan)
{
	auto span = scan.get().span;
	auto name = scan.eat(ID).string;
	scan.eat(COLON);
	auto type = parseType(scan);
	return Var { name, type, span };
}
static Var parseVarOpt (Scanner& scan)
{
	auto name = scan.eat(ID).string;
	TypePtr type = nullptr;

	// types optional
	if (scan.get() == COLON)
	{
		scan.shift();
		type = parseType(scan);
	}

	return Var { name, type };
}

/*
name:
	ID ["::" ID]
*/
static Name parseName (Scanner& scan)
{
	if (scan.get(1) == DOUBLECOLON)
	{
		auto mod = scan.eat(ID).string;
		scan.shift();
		auto name = scan.eat(ID).string;

		return Name(name, mod);
	}
	else
		return Name(scan.eat(ID).string);
}


/*
	"use" ID
*/
static void parseUse (Toplevel& top, Scanner& scan)
{
	auto sp = scan.shift().span;
	auto name = scan.eat(ID).string;

	for (auto& s : top.uses)
		if (s == name)
			throw SourceError("using same module more than once", sp);

	top.uses.push_back(name);
}
/*
	"module" ID
*/
static void parseModule (Toplevel& top, Scanner& scan)
{
	if (!top.module.empty())
		throw SourceError("module declared more than once",
			scan.get().span);

	scan.shift();
	top.module = scan.eat(ID).string;
}

/*
funcDecl:
	"func" ID "(" [var {"," var}] ")" funcBody
funcBody:
	blockExp
	"extern" "(" STRING ")" STRING "->" type
*/
static FuncDecl* parseFuncBody (Scanner& scan,
		const std::string& name, const std::vector<Var>& args)
{
	scan.expect({ LCURL, KW_extern });

	if (scan.get() == KW_extern)
	{
		scan.shift();
		scan.eat(LPAREN);
		auto pkg = scan.eat(STRING).string;
		scan.eat(RPAREN);
		auto key = scan.eat(STRING).string;
		scan.eat(ARROW);
		auto ret = parseType(scan);

		return new FuncDecl(name, args, pkg, key, ret);
	}
	else
		return new FuncDecl(name, args, parseBlockExp(scan));
}
static DeclPtr parseFuncDecl (Scanner& scan, const Var& impl)
{
	auto span = scan.shift().span;
	auto name = scan.eat(ID).string;
	auto args = commaList<Var>(scan, parseVarOpt, LPAREN, RPAREN);

	auto fn = parseFuncBody(scan, name, args);
	fn->impl = impl;
	fn->span = span;
	return DeclPtr(fn);
}



/*
typeDecl:
	"type" ID [typeParams] typeBody
typeParams:
	"[" [POLYID {"," POLYID}] "]"
*/
static std::pair<Span, std::string> parsePOLYID (Scanner& scan)
{
	auto span = scan.get().span;
	auto id = scan.eat(POLYID).string;
	return std::pair<Span, std::string>(span, id);
}
static std::vector<Span> parseTypeSig (Scanner& scan,
		std::string& name, std::vector<std::string>& args)
{
	name = scan.eat(ID).string;
	if (scan.get() == LBRACK)
	{
		auto parsed = commaList(scan, parsePOLYID, LBRACK, RBRACK);
		std::vector<Span> spans;

		// prevent duplicates
		args.reserve(args.size() + parsed.size());
		spans.reserve(parsed.size());
		for (size_t i = 0; i < parsed.size(); i++)
		{
			for (size_t j = 0; j < i; j++)
				if (parsed[i].second == parsed[j].second)
					throw SourceError("duplicate parameter names",
						{ parsed[j].first, parsed[i].first });

			spans.push_back(parsed[i].first);
			args.push_back(parsed[i].second);
		}

		return spans;
	}
	else
		return {};
}
/*
enumFunc:
	ID "(" [type {"," type}] ")"
*/
static EnumFunc parseEnumFunc (Scanner& scan)
{
	auto span = scan.get().span;
	auto name = scan.eat(ID).string;
	auto args = commaList(scan, parseType, LPAREN, RPAREN);
	return EnumFunc {
		name, args, span
	};
}
/*
typeBody:
	"{" [var {"," var}] "}"
	"extern" "true"
	"extern" "false"
	"=" enumFunc {"or" enumFunc}
*/
static DeclPtr parseTypeDecl (Scanner& scan)
{
	TypeDecl* res;
	auto span = scan.shift().span;

	std::string name;
	std::vector<std::string> args;
	parseTypeSig(scan, name, args);

	if (scan.get() == LCURL)
	{
		// struct type
		auto mems = commaList(scan, parseVar, LCURL, RCURL);
		res = new TypeDecl(name, args, mems);
	}
	else if (scan.get() == EQUAL)
	{
		// enum type
		scan.shift();

		std::vector<EnumFunc> funcs;
		funcs.push_back(parseEnumFunc(scan));
		while (scan.get() == KW_or)
		{
			scan.shift();
			funcs.push_back(parseEnumFunc(scan));
		}

		res = new TypeDecl(name, args, funcs);
	}
	else if (scan.get() == KW_extern)
	{
		// external type
		scan.shift();
		bool gc = (scan.get() == KW_true);
		scan.expect({ KW_true, KW_false });
		scan.shift();

		res = new TypeDecl(name, args, gc);
	}
	else
	{
		scan.expect({ LCURL, EQUAL, KW_extern });
		return nullptr;
	}

	res->span = span;
	return DeclPtr(res);
}

/*
ifaceDecl:
	"iface" [POLYID ":"] ID [typeParams] ifaceBody
ifaceBody:
	"{" {ifaceFunc} "}""
ifaceFunc:
	"fn" ID "(" [type {"," type}] ")" "->" type
*/
static void parseIFaceFunc (IFaceDecl* iface, Scanner& scan)
{
	auto span = scan.shift().span;
	auto name = scan.eat(ID).string;
	auto args = commaList(scan, parseType, LPAREN, RPAREN, true);
	scan.eat(ARROW);
	auto ret = parseType(scan);
	iface->funcs.push_back({ name, args, ret, span });
}
static DeclPtr parseIFaceDecl (Scanner& scan)
{
	auto span = scan.shift().span;

	std::string self = "";
	Span selfSpan;
	if (scan.get(1) == COLON)
	{
		selfSpan = scan.get().span;
		self = scan.eat(POLYID).string;
		scan.shift();
	}

	std::string name;
	std::vector<std::string> args;
	auto argSpans = parseTypeSig(scan, name, args);

	// argument with same name as 'self'?
	if (!self.empty())
		for (size_t i = 0, len = args.size(); i < len; i++)
			if (args[i] == self)
				throw SourceError("duplicate parameter names",
					{ selfSpan, argSpans[i] });

	auto iface = new IFaceDecl(self, name, args);
	iface->span = span;

	scan.eat(LCURL);
	while (scan.get() != RCURL)
	{
		scan.expect({ RCURL, KW_fn });
		parseIFaceFunc(iface, scan);
	}
	scan.shift();

	return DeclPtr(iface);
}

/*
implDecl:
	"impl" [ID ":"] type implBody
implBody:
	"{" {funcDecl} "}"
*/
static void parseImpl (Toplevel& top, Scanner& scan)
{
	scan.shift();
	Var impl { Names::Self, nullptr };

	if (scan.get(1) == COLON)
	{
		impl.name = scan.eat(ID).string;
		scan.shift();
	}
	impl.type = parseType(scan);
	scan.eat(LCURL);

	while (scan.get() != RCURL)
	{
		scan.expect({ RCURL, KW_fn });
		auto fn = parseFuncDecl(scan, impl);
		top.decls.push_back(fn);
	}

	scan.shift();
}

/*
toplevel:
	"module" ID
	"use" ID
	["pub"] funcDecl
	["pub"] typeDecl
	["pub"] ifaceDecl
	implDecl
*/
static bool parseToplevel (Toplevel& top, Scanner& scan)
{
	switch (scan.get().kind)
	{
	case KW_use:
		parseUse(top, scan);
		return true;
	case KW_module:
		parseModule(top, scan);
		return true;
	case KW_impl:
		parseImpl(top, scan);
		return true;
	case KW_type:
		top.decls.push_back(parseTypeDecl(scan));
		return true;
	case KW_fn:
		top.decls.push_back(parseFuncDecl(scan));
		return true;
	case KW_iface:
		top.decls.push_back(parseIFaceDecl(scan));
		return true;
	case KW_pub:
		top.decls.push_back(parsePublic(scan));
		return true;
	// case KW_let:
	default:
		return false;
	}
}
static DeclPtr parsePublic (Scanner& scan)
{
	auto span = scan.shift().span;
	DeclPtr res;

	switch (scan.get().kind)
	{
	case KW_type:
		res = parseTypeDecl(scan);
		break;
	case KW_fn:
		res = parseFuncDecl(scan);
		break;
	case KW_iface:
		res = parseIFaceDecl(scan);
		break;
	// case KW_let:
	default:
		scan.expect({KW_type, KW_fn, KW_iface});
		return nullptr;
	}
	res->span = span;
	res->isPublic = true;
	return res;
}
Toplevel parseToplevel (Scanner& scan)
{
	Toplevel top;
	while (parseToplevel(top, scan))
		;

	if (scan.get() != END_OF_FILE)
		throw scan.get().die("invalid toplevel declaration");

	return top;
}



/*
type:
	POLYID [type_ifaces]
	name [type_args]
	fn_type
	tuple_type
type_ifaces:
	"(" type {"," type} ")"
type_args:
	"[" type {"," type} "]"
fn_type:
	"fn" "(" [type {"," type}] ")" "->" type
tuple_type:
	"(" type {"," type} ")"
*/
TypePtr parseType (Scanner& scan)
{
	auto span = scan.get().span;
	TypePtr ty;

	if (scan.get() == ID)
	{
		auto name = parseName(scan);
		TypeList args;

		if (scan.get() == LBRACK)
			args = commaList(scan, parseType, LBRACK, RBRACK, false);
		else if (scan.get() == LPAREN) // (a)
			scan.expect(LBRACK);

		ty = TypePtr(new ConcreteType(name, args));
	}
	else if (scan.get() == POLYID)
	{
		auto name = scan.eat(POLYID).string;
		TypeList ifaces;

		if (scan.get() == LPAREN)
			ifaces = commaList(scan, parseType, LPAREN, RPAREN, false);
		else if (scan.get() == LBRACK) // (b)
			scan.expect(LPAREN);

		ty = TypePtr(new ParamType(name, ifaces));
	}
	else if (scan.get() == KW_fn)
	{
		scan.shift();
		auto args = commaList(scan, parseType, LPAREN, RPAREN, true);
		scan.eat(ARROW);
		args.push_back(parseType(scan));

		ty = TypePtr(new FuncType(args));
	}
	else if (scan.get() == LPAREN)
	{
		auto args = commaList(scan, parseType, LPAREN, RPAREN, false);
		if (args.size() == 1)
			return args[0];

		ty = TypePtr(new TupleType(args));
	}
	else
		throw SourceError("expected type", span);

	// (a) and (b) warn about using the wrong kind of parenthesis/brackets
	//  to denote arguments/ifaces
	// should this be happening or should we delegate this syntax error?

	ty->span = span;
	return ty;
}







/*
	operator precedence parser
*/
struct Op
{
	int token;
	int precedence;
	int order;
	int kind;
	std::string str;
};
enum {
	Left = 0, Right,
	MaxPrec = 7,

	Compare = 0, Cons, Lazy, Standard
};

static Op getOp (int k)
{
	static std::vector<Op> operators {
		{ LBIND,   0, Left, Standard, Names::OperLBind },
		{ RBIND,   0, Left, Standard, Names::OperRBind },
		{ LBLOCK,  0, Left, Standard, Names::OperLBlock },
		{ RBLOCK,  0, Left, Standard, Names::OperRBlock },
		{ LSHIFT,  0, Left, Standard, Names::OperLShift },
		{ RSHIFT,  0, Left, Standard, Names::OperRShift },
		{ KW_and,  1, Left, Lazy },
		{ KW_or,   1, Left, Lazy },
		{ LT,      2, Left, Compare },
		{ LE,      2, Left, Compare },
		{ EQ,      2, Left, Compare },
		{ NE,      2, Left, Compare },
		{ GE,      2, Left, Compare },
		{ GR,      2, Left, Compare },
		{ CONS,    3, Right, Cons },
		{ PLUS,    4, Left, Standard, Names::OperAdd },
		{ MINUS,   4, Left, Standard, Names::OperSub },
		{ TIMES,   5, Left, Standard, Names::OperMul },
		{ DIVIDE,  5, Left, Standard, Names::OperDiv },
		{ MODULO,  5, Left, Standard, Names::OperMod },
		{ EXP,     6, Right, Standard, Names::OperExp },
	};

	for (auto& op : operators)
		if (op.token == k)
			return op;

	return Op { END_OF_FILE, -1, Left };
}

static inline CompareExp::Kind compareOp (int token)
{
	switch (token)
	{
	case LT: return CompareExp::Kind::Lt;
	case LE: return CompareExp::Kind::LtEq;
	case NE: return CompareExp::Kind::NotEq;
	case GE: return CompareExp::Kind::GrEq;
	case GR: return CompareExp::Kind::Gr;
	case EQ:
	default: return CompareExp::Kind::Eq;
	}
}
static inline LazyOpExp::Kind lazyOp (int token)
{
	if (token == KW_and)
		return LazyOpExp::Kind::And;
	else
		return LazyOpExp::Kind::Or;
}
static ExpPtr combine (ExpPtr lh, ExpPtr rh, const Op& op, const Span& span)
{
	ExpPtr res;
	switch (op.kind)
	{
	case Lazy:
		res = ExpPtr(new LazyOpExp(lh, lazyOp(op.token), rh));
		break;
	case Compare:
		res = ExpPtr(new CompareExp(lh, compareOp(op.token), rh));
		break;
	case Cons:
		res = Exp::cons(span, lh, rh);
		break;
	case Standard:
		res = methodCall(span, lh, op.str, { rh });
		break;
	default:
		res = nullptr;
	}
	res->span = span;
	return res;
}
static ExpPtr parseExp (Scanner& scan, int prec)
{
	if (prec >= MaxPrec)
		return parseTerm(scan);

	auto lh = parseExp(scan, prec + 1);
	Op oper;

	for (;;)
	{
		oper = getOp(scan.get().kind);
		if (oper.precedence != prec)
			break;

		auto span = scan.shift().span;

		if (oper.order == Right)
		{
			auto rh = parseExp(scan, prec);
			lh = combine(lh, rh, oper, span);
			break;
		}
		else
		{
			auto rh = parseExp(scan, prec + 1);
			lh = combine(lh, rh, oper, span);
		}
	}

	return lh;
}
ExpPtr parseExp (Scanner& scan)
{
	return parseExp(scan, 0);
}


/*
term:
	opUnary term
	prefixTerm {suffix} [typeHint]

prefixTerm:
	name
	constant
	tupleExp
	fieldExp
	condExp
	objectExp
	lambdaExp
	blockExp

tupleExp:
	"(" exp {"," exp} ")"
fieldExp:
	"." ID
*/
static ExpPtr parseTerm (Scanner& scan)
{
	ExpPtr res;
	auto span = scan.get().span;

	if ((res = parseConstant(scan)) == nullptr)
	switch (scan.get().kind)
	{
	case ID:
		res = ExpPtr(new VarExp(parseName(scan)));
		res->span = span;
		break;
	case LPAREN:
		{
			auto exps = commaList(scan, parseExp, LPAREN, RPAREN, true);
			if (exps.size() == 1)
				res = exps[0];
			else
			{
				res = ExpPtr(new TupleExp(exps));
				res->span = span;
			}
			break;
		}
	case LBRACK:
		{
			auto exps = commaList(scan, parseExp, LBRACK, RBRACK);
			res = ExpPtr(new ListExp(exps));
			res->span = span;
			break;
		}
	case DOT:
		{
			scan.shift();
			auto method = new MethodExp(scan.eat(ID).string);
			if (scan.get() == DOUBLECOLON)
			{
				scan.shift();
				method->hint = parseType(scan);
			}
			res = ExpPtr(method);
			res->span = span;
			break;
		}
	case KW_fn:
		res = parseLambda(scan);
		break;
	case KW_new:
		res = parseObject(scan);
		break;
	case KW_if:
		res = parseCond(scan, true);
		break;
	case KW_match:
		res = parseMatch(scan);
		break;
	case LCURL:
		res = parseBlockExp(scan);
		break;
	case MINUS:
		scan.shift();
		return methodCall(span, 
			parseTerm(scan), Names::Negate, {});
	case KW_not:
		scan.shift();
		return methodCall(span, 
			parseTerm(scan), Names::Not, {});
	default:
		throw SourceError("invalid expression", span);
	}
	res = parseSuffix(scan, res);
	return res;
}

/*
constant:
	["-"] numberConstant
	STRING
	CHAR
	"true"
	"false"
numberConstant:
	INT
	REAL
	LONG
*/
static ExpPtr parseNumber (Scanner& scan)
{
	switch (scan.get().kind)
	{
	case INT:  return ExpPtr(new NumberExp(scan.shift().val_int));
	case REAL: return ExpPtr(new NumberExp(scan.shift().val_real));
	case LONG: return ExpPtr(new NumberExp(scan.shift().val_long));
	default:   return nullptr;
	}
}
static ExpPtr parseConstant (Scanner& scan)
{
	ExpPtr res;
	auto span = scan.get().span;
	switch (scan.get().kind)
	{
	case INT: case REAL: case LONG:
		res = parseNumber(scan);
		break;
	case STRING:
		res = ExpPtr(new StringExp(scan.shift().string));
		break;
	case KW_true:
		scan.shift();
		res = ExpPtr(new BoolExp(true));
		break;
	case KW_false:
		scan.shift();
		res = ExpPtr(new BoolExp(false));
		break;
	case CHAR:
		res = ExpPtr(new CharExp(scan.shift().val_char));
		break;
	case MINUS:
		{
			// negative numbers don't require method calls!
			auto next = scan.get(1).kind;
			if (next == INT || next == REAL || next == LONG)
			{
				scan.shift();
				res = parseNumber(scan);
				((NumberExp*) res.get())->negate();
				break;
			}
		}
	default:
		return nullptr;
	}
	res->span = span;
	return res;
}

/*
suffix:
	callSuffix
	fieldSuffix
	propertySuffix
	sliceSuffix
	lambdaSuffix
callSuffix:
	"(" [exp {"," exp}] ")"
fieldSuffix:
	"." ID
propertySuffix:
	"." "(" ID ")"
sliceSuffix:
	"[" "," exp "]"
	"[" exp "," exp "]"
	"[" exp "," "]"
	"[" exp "]"
typeHint:
	":" type
*/
static ExpPtr parseSliceSuffix (Scanner& scan, ExpPtr e)
{
	ExpPtr res;
	auto span = scan.shift().span;

	if (scan.get() == COMMA)
	{
		scan.shift();
		auto to = parseExp(scan);
		res = ExpPtr(new MemberExp(MemberExp::SliceTo, e, to));
	}
	else
	{
		auto from = parseExp(scan);

		if (scan.get() == COMMA)
		{
			scan.shift();
			if (scan.get() != RBRACK)
			{
				auto to = parseExp(scan);
				res = ExpPtr(new MemberExp(e, from, to)); // Slice
			}
			else
				res = ExpPtr(new MemberExp(MemberExp::SliceFrom, e, from));
		}
		else
			res = ExpPtr(new MemberExp(e, from)); // Get
	}

	scan.eat(RBRACK);
	res->span = span;
	return res;
}
static ExpPtr parseSuffix (Scanner& scan, ExpPtr e)
{
	switch (scan.get().kind)
	{
	case LPAREN:
		{
			auto span = e->span;
			auto args = commaList(scan, parseExp, LPAREN, RPAREN, true);
			e = ExpPtr(new CallExp(e, args));
			e->span = span;
			break;
		}
	case DOT:
		scan.shift();
		scan.expect({ ID, LPAREN });
		if (scan.get() == ID)
		{
			auto span = scan.get().span;
			auto field = scan.eat(ID).string;
			e = ExpPtr(new FieldExp(e, field));
			e->span = span;
			break;
		}
		else
		{
			auto span = scan.shift().span;
			auto prop = scan.eat(ID).string;
			scan.eat(RPAREN);
			e = ExpPtr(new PropertyExp(e, prop));
			e->span = span;
			break;
		}
	case LBRACK:
		{
			e = parseSliceSuffix(scan, e);
			break;
		}
	case BAR:
		{
			auto span = e->span;
			auto lam = parseLambdaSuffix(scan);
			e = ExpPtr(new CallExp(e, { lam }));
			e->span = span;
			break;
		}
	case COLON:
		{
			auto span = scan.shift().span;
			auto type = parseType(scan);
			e = ExpPtr(new TypeHintExp(e, type));
			e->span = span;
			return e; // no more prefixes after this
		}
	default:
		return e;
	}
	return parseSuffix(scan, e);
}

/*
blockExp:
	"{" {stmt} [finalStmt] "}"

stmt:
	";"
	condStmt
	whileLoop
	forLoop
	letDecl
	exp [assignment]
*/
static ExpPtr parseBlockExp (Scanner& scan)
{
	auto span = scan.eat(LCURL).span;
	bool unit = false;
	ExpList exps;

	while (parseStmt(scan, exps, unit))
		;

	scan.eat(RCURL);

	if (exps.empty())
		unit = true;

	ExpPtr res(new BlockExp(exps, unit));
	res->span = span;
	return res;
}
static bool parseStmt (Scanner& scan, ExpList& exps, bool& unit)
{
	if (parseFinalStmt(scan, exps))
	{
		unit = false;
		return false;
	}

	switch (scan.get().kind)
	{
	case RCURL:
		return false;
	case SEMICOLON:
		unit = true;
		scan.shift();
		break;
	case KW_if:
		{
			auto cond = parseCond(scan, false);
			unit = cond->children.size() == 2;
			exps.push_back(cond);
			break;
		}
	case KW_while:
		unit = true;
		exps.push_back(parseWhileLoop(scan));
		break;
	case KW_let:
		unit = true;
		exps.push_back(parseLet(scan));
		break;
	default:
		exps.push_back(parseAssign(scan, unit));
		break;
	}
	return true;
}

/*
letDecl:
	"let" pat "=" exp
*/
static ExpPtr parseLet (Scanner& scan)
{
	auto span = scan.shift().span;
	auto pat = parsePat(scan);
	pat->rootPosition = true;
	scan.eat(EQUAL);
	auto init = parseExp(scan);
	auto res = ExpPtr(new LetExp(pat, init));
	res->span = span;
	return res;
}

/*
whileLoop:
	"while" exp blockExp
*/
static ExpPtr parseWhileLoop (Scanner& scan)
{
	auto span = scan.shift().span;
	auto cond = parseExp(scan);
	auto body = parseBlockExp(scan);
	auto res = ExpPtr(new WhileExp(cond, body));
	res->span = span;
	return res;
}

/*
assignment:
	"=" exp
*/
static ExpPtr parseAssign (Scanner& scan, bool& unit)
{
	auto lh = parseExp(scan);
	if (scan.get() == EQUAL)
	{
		unit = true;
		auto span = scan.shift().span;
		auto rh = parseExp(scan);
		auto res = ExpPtr(new AssignExp(lh, rh));
		res->span = span;
		return res;
	}
	else
	{
		unit = false;
		return lh;
	}
}

/*
finalStmt:
	"return" [exp]
*/
static bool parseFinalStmt (Scanner& scan, ExpList& exps)
{
	ExpPtr res;
	auto span = scan.get().span;

	switch (scan.get().kind)
	{
	case KW_return:
		{
			scan.shift();
			ExpPtr ret;
			if (scan.get() != RCURL)
				ret = parseExp(scan);
			else
			{
				ret = ExpPtr(new TupleExp({}));
				ret->span = span;
			}
			res = ExpPtr(new ReturnExp(ret));
			break;
		}

	default:
		return false;
	}
	res->span = span;
	exps.push_back(res);
	return true;
}

/*
condExp:
	"if" exp blockExp elseExp
elseExp:
	"else" condExp
	"else" blockExp

condStmt:
	"if" exp blockExp [elseStmt]
elseStmt:
	"else" condStmt
	"else" blockExp
*/
static ExpPtr parseCond (Scanner& scan, bool req_else)
{
	auto span = scan.eat(KW_if).span;
	auto cond = parseExp(scan);
	auto if_body = parseBlockExp(scan);
	ExpPtr res;

	if (req_else)
		scan.expect(KW_else);

	if (req_else || scan.get() == KW_else)
	{
		ExpPtr else_body = nullptr;

		scan.shift();
		if (scan.get() == KW_if)
			else_body = parseCond(scan, req_else);
		else if (scan.get() == LCURL)
			else_body = parseBlockExp(scan);
		else
			scan.expect({ KW_if, LCURL });

		res = ExpPtr(new CondExp(cond, if_body, else_body));
	}
	else
		res = ExpPtr(new CondExp(cond, if_body));

	res->span = span;
	return res;
}


/*
lambdaExp:
	"fn" "(" [lamVar {"," lamVar}] ")" blockExp
lamVar:
	ID [":" type]
lambdaSuffix:
	"|" [lamVar {"," lamVar}] "|" blockExp
*/
static ExpPtr parseLambda (Scanner& scan)
{
	auto span = scan.shift().span;
	auto args = commaList(scan, parseVarOpt, LPAREN, RPAREN);
	auto body = parseBlockExp(scan);
	auto res = ExpPtr(new LambdaExp(args, body));
	res->span = span;
	return res;
}
static ExpPtr parseLambdaSuffix (Scanner& scan)
{
	auto span = scan.get().span;
	auto args = commaList(scan, parseVarOpt, BAR, BAR);
	auto body = parseBlockExp(scan);
	auto res = ExpPtr(new LambdaExp(args, body));
	res->span = span;
	return res;
}

/*
objectExp:
	"new" type objectInits
objectInits:
	"{" [init {"," init}] "}"
init:
	ID "=" exp
*/
static ObjectExp::Init parseInit (Scanner& scan)
{
	auto name = scan.eat(ID).string;
	scan.eat(EQUAL);
	auto val = parseExp(scan);
	return ObjectExp::Init(name, val);
}
static ExpPtr parseObject (Scanner& scan)
{
	auto span = scan.shift().span;
	auto type = parseType(scan);
	auto inits = commaList(scan, parseInit, LCURL, RCURL);
	auto res = ExpPtr(new ObjectExp(type, inits));
	res->span = span;
	return res;
}

/*
matchExp:
	"match" exp matchBody
matchBody:
	"{" {matchCase} "}"
matchCase:
	pat "->" exp
	pat blockExp
*/
static MatchExp::Case parseMatchCase (Scanner& scan)
{
	auto pat = parsePat(scan);
	pat->rootPosition = true;
	ExpPtr res;

	scan.expect({ ARROW, LCURL });
	if (scan.get() == ARROW)
	{
		scan.shift();
		res = parseExp(scan);
	}
	else
		res = parseBlockExp(scan);

	return MatchExp::Case(pat, res);
}
static ExpPtr parseMatch (Scanner& scan)
{
	auto span = scan.shift().span;
	auto cond = parseExp(scan);
	std::vector<MatchExp::Case> cases;

	scan.eat(LCURL);

	do
	{
		cases.push_back(parseMatchCase(scan));
	} while (scan.get() != RCURL);

	scan.shift();

	auto res = ExpPtr(new MatchExp(cond, cases));
	res->span = span;
	return res;
}






/*
pat:
	prefixPat ["$" pat]
prefixPat:
	constant
	ID
	enumPat
	tuplePat
	listPat
enumPat:
	name "(" [pat {"," pat}] ")"
tuplPat:
	"(" [pat {"," pat}] ")"
listPat:
	"[" [pat {"," pat}] "]"
*/
PatPtr parsePat (Scanner& scan)
{
	auto span = scan.get().span;
	PatPtr res;

	switch (scan.get().kind)
	{
	case MINUS: case INT: case REAL: case LONG:
	case CHAR: case STRING: case KW_true: case KW_false:
		if (auto e = parseConstant(scan))
		{
			res = PatPtr(new ConstPat(e));
			break;
		}
		else
			goto fail;

	case LPAREN: // ( args... )
		{
			auto args = commaList(scan, parsePat, LPAREN, RPAREN);
			res = PatPtr(new EnumPat(EnumPat::Tuple, args));
			break;
		}

	case LBRACK:
		{
			auto args = commaList(scan, parsePat, LBRACK, RBRACK);
			res = PatPtr(new EnumPat(EnumPat::List, args));
			break;
		}

	case ID:
		// Mod::Ctor ( args... )
		if (scan.get(1).kind == DOUBLECOLON ||
				scan.get(1).kind == LPAREN)
		{
			auto name = parseName(scan);
			auto args = commaList(scan, parsePat, LPAREN, RPAREN);
			res = PatPtr(new EnumPat(name, args));
		}
		else // Var
			res = PatPtr(new BindPat(scan.shift().string));
		break;

	default:
		goto fail;
	}
	res->span = span;

	if (scan.get() == CONS)
	{
		auto consSpan = scan.shift().span;
		auto tail = parsePat(scan);
		return Pat::cons(consSpan, res, tail);
	}
	else
		return res;
fail:
	throw SourceError("invalid pattern", span);
}


}}
