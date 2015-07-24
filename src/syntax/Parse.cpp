#include <syntax/Parse.h>

using namespace AST;
using namespace Tokens;

namespace Parse {
;

static void parseUse (Toplevel& top, Scanner& scan);
static void parseModule (Toplevel& top, Scanner& scan);
static void parseFuncDecl (Toplevel& top, Scanner& scan);
static void parseTypeDecl (Toplevel& top, Scanner& scan);
static void parseIFaceDecl (Toplevel& top, Scanner& scan);
static void parseImpl (Toplevel& top, Scanner& scan);



// utility to easily create
//  obj.method(a, b, c, ...)
static inline ExpPtr methodCall (const Span& span,
		ExpPtr obj, const std::string& method,
		const ExpList& args)
{
	ExpPtr mem(new FieldExp(obj, method));
	ExpPtr call(new CallExp(mem, args));
	mem->span = span;
	call->span = span;
	return call;
}

static ExpPtr parseBlockExp (Scanner& scan);




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
	auto name = scan.eat(ID).string;
	scan.eat(COLON);
	auto type = parseType(scan);
	return Var { name, type };
}

/*
name:
	ID "::" ID
	ID
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
fn_decl:
	"fn" ID "(" [var {"," var}] ")" fn_body

fn_body:
	"=" exp
	block_exp
*/
static ExpPtr parseFuncBody (Scanner& scan)
{
	scan.expect({ EQUAL, LCURL });

	if (scan.get() == EQUAL)
	{
		scan.shift();
		return parseExp(scan);
	}
	else
		return parseBlockExp(scan);
}
static FuncDeclPtr parseFuncDecl (Scanner& scan)
{
	auto span = scan.shift().span;
	auto name = scan.eat(ID).string;
	auto args = commaList<Var>(scan, parseVar, LPAREN, RPAREN);
	auto body = parseFuncBody(scan);

	FuncDeclPtr fn(new FuncDecl(name, args, body));
	fn->span = span;
	return fn;
}
static void parseFuncDecl (Toplevel& top, Scanner& scan)
{
	top.funcs.push_back(parseFuncDecl(scan));
}

/*
type_decl:
	"type" type "=" type
	"type" type "{" [var {"," var}] "}"
*/
static bool validSignatureType (TypePtr ty)
{
	auto conc = dynamic_cast<ConcreteType*>(ty.get());

	if (conc == nullptr)
		return false;

	for (auto sub : conc->subtypes)
		if (!sub->is<ParamType>())
			return false;

	return true;
}
static void parseTypeDecl (Toplevel& top, Scanner& scan)
{
	scan.shift();
	auto sig = parseType(scan);

	if (!validSignatureType(sig))
		throw SourceError("invalid signature type",
			{"expected concrete-type with param-type arguments"}, sig->span);

	auto name = ((ConcreteType*) sig.get())->name.name;
	auto args = ((ConcreteType*) sig.get())->subtypes;

	if (scan.get() == LCURL)
	{
		auto mems = commaList(scan, parseVar, LCURL, RCURL);
		top.types.push_back(std::make_shared<TypeDecl>(name, args, mems));
	}
	else if (scan.get() == EQUAL)
	{
		scan.shift();
		auto alias = parseType(scan);
		top.types.push_back(std::make_shared<TypeDecl>(name, args, alias));
	}
	else
		scan.expect({ LCURL, EQUAL });
}

/*
iface:
	"iface" ID [POLYID] "{" {fn_iface} "}"
fn_iface:
	"fn" ID "(" [var {"," var}] ")" ":" type
*/
static void parseIFaceFunc (IFaceDeclPtr iface, Scanner& scan)
{
	auto span = scan.shift().span;
	auto name = scan.eat(ID).string;
	auto args = commaList(scan, parseVar, LPAREN, RPAREN, true);
	scan.eat(COLON);
	auto ret = parseType(scan);
	iface->funcs.push_back({ name, args, ret, span });
}
static void parseIFaceDecl (Toplevel& top, Scanner& scan)
{
	auto span = scan.shift().span;
	auto name = scan.eat(ID).string;
	TypePtr self = nullptr;

	scan.expect({ POLYID, LCURL });
	if (scan.get() == POLYID)
	{
		self = parseType(scan);
		if (!((ParamType*) self.get())->ifaces.empty())
			throw SourceError("invalid self type",
				{"expected param-type with no ifaces"},
				self->span);
	}

	IFaceDeclPtr iface(new IFaceDecl(name, self));
	iface->span = span;
	top.ifaces.push_back(iface);

	scan.eat(LCURL);
	while (scan.get() != RCURL)
	{
		scan.expect({ RCURL, KW_fn });
		parseIFaceFunc(iface, scan);
	}
	scan.shift();
}

/*
impl:
	"impl" [ID ":"] type "{" {fn_decl} "}"
*/
static void parseImpl (Toplevel& top, Scanner& scan)
{
	scan.shift();
	Var impl { "self", nullptr };

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
		auto fn = parseFuncDecl(scan);
		fn->impl = impl;
		top.funcs.push_back(fn);
	}

	scan.shift();
}

/*
language:
	{toplevel}

toplevel:
	module
	fn_decl
	type_decl
	let_decl
	impl
	iface
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
	case KW_type:
		parseTypeDecl(top, scan);
		return true;
	case KW_fn:
		parseFuncDecl(top, scan);
		return true;
	case KW_impl:
		parseImpl(top, scan);
		return true;
	case KW_iface:
		parseIFaceDecl(top, scan);
		return true;
	default:
		return false;
	}
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
type_ifaces:
	"(" name {"," name} ")"
type_args:
	"[" type {"," type} "]"
*/
TypePtr parseType (Scanner& scan)
{
	auto span = scan.get().span;
	TypePtr ty;

	if (scan.get() == ID)
	{
		auto name = parseName(scan);
		std::vector<TypePtr> args;

		if (scan.get() == LBRACK)
			args = commaList(scan, parseType, LBRACK, RBRACK, false);
		else if (scan.get() == LPAREN) // (a)
			scan.expect(LBRACK);

		ty = TypePtr(new ConcreteType(name, TypeList(args)));
	}
	else if (scan.get() == POLYID)
	{
		auto name = scan.eat(POLYID).string;
		std::vector<Name> ifaces;

		if (scan.get() == LPAREN)
			ifaces = commaList(scan, parseName, LPAREN, RPAREN, false);
		else if (scan.get() == LBRACK) // (b)
			scan.expect(LPAREN);

		ty = TypePtr(new ParamType(name, ifaces));
	}
	else
		throw SourceError("expected type", span);

	// (a) and (b) warn about using the wrong kind of parenthesis/brackets
	//  to denote arguments/ifaces
	// should this be happening or should we delegate this syntax error?

	ty->span = span;
	return ty;
}






static ExpPtr parseTerm (Scanner& scan);
static ExpPtr parseExp (Scanner& scan, int prec);


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
	MaxPrec = 5,

	Compare = 0, Cons, Lazy, Standard
};
static std::vector<Op> operators {
	{ KW_and,  0, Left, Lazy },
	{ KW_or,   0, Left, Lazy },
	{ LT,      1, Left, Compare },
	{ LE,      1, Left, Compare },
	{ EQ,      1, Left, Compare },
	{ NE,      1, Left, Compare },
	{ GE,      1, Left, Compare },
	{ GR,      1, Left, Compare },
	{ CONS,    2, Right, Cons },
	{ PLUS,    3, Left, Standard, "add" },
	{ MINUS,   3, Left, Standard, "sub" },
	{ TIMES,   4, Left, Standard, "mul" },
	{ DIVIDE,  4, Left, Standard, "div" },
	{ MODULO,  4, Left, Standard, "mod" },
};

static Op getOp (int k)
{
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
		res = ExpPtr(new ConsExp(lh, rh));
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

ExpPtr parseExp (Scanner& scan)
{
	return parseExp(scan, 0);
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


/*
term:
	op_unary term
	prefix_term {term_suffix}
prefix_term:
	name
	INT
	REAL
	LONG
	STRING
	bool_exp
	tuple_exp
	lambda_exp
	object_exp
	cond_exp
	block_exp
term_suffix:
	"(" [exp {"," exp}] ")"
	"." ID
	"[" exp "]"
*/
static ExpPtr parseTerm (Scanner& scan)
{
	ExpPtr res;
	auto span = scan.get().span;
	switch (scan.get().kind)
	{
	case ID:
		res = ExpPtr(new VarExp(parseName(scan)));
		break;
	case INT:
		res = ExpPtr(new IntExp(scan.shift().val_int));
		break;
	case REAL:
		res = ExpPtr(new RealExp(scan.shift().val_real));
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
	case LPAREN:
		{
			auto exps = commaList(scan, parseExp, LPAREN, RPAREN, true);
			if (exps.size() == 1)
			{
				res = exps[0];
				span = res->span;
			}
			else
				res = ExpPtr(new TupleExp(exps));
			break;
		}
	case LCURL:
		res = parseBlockExp(scan);
		break;
	default:
		throw SourceError("expected expression", span);
	}

	res->span = span;
	return res;
}

/*
block_exp:
	"{" {stmt} [final_stmt] "}"
stmt:
	";"
	let_decl
	cond_if [cond_else]
	loop
	exp [assignment]
final_stmt:
	"return" [exp]
	"break"
	"continue"
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
			if (scan.get() != LCURL)
				ret = parseExp(scan);
			else
				ret = ExpPtr(new TupleExp({}));
			res = ExpPtr(new ReturnExp(ret));
			break;
		}

	case KW_break:
		scan.shift();
		res = ExpPtr(new GotoExp(GotoExp::Break));
		break;
	case KW_continue:
		scan.shift();
		res = ExpPtr(new GotoExp(GotoExp::Continue));
		break;
	default:
		return false;
	}
	res->span = span;
	exps.push_back(res);
	return true;
}
static ExpPtr parseBlockExp (Scanner& scan)
{
	auto span = scan.eat(LCURL).span;
	ExpList exps;
	bool unit = false;

	while (scan.get() != RCURL)
	{
		if (scan.get() == SEMICOLON)
		{
			unit = true;
			scan.shift();
		}
		else if (parseFinalStmt(scan, exps))
		{
			break;
		}
		else
		{
			unit = false;
			exps.push_back(parseExp(scan));
		}
	}

	scan.eat(RCURL);

	ExpPtr res(new BlockExp(exps, unit));
	res->span = span;
	return res;
}

}