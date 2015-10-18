#include "../opal.h"
#include "../env/Loader.h"
#include "../runtime/Exec.h"
#include <cmath>
#include <cstdlib>
#include <string>

using namespace Opal;
using namespace Opal::Run;

static std::string pop_string (Thread& th)
{
	auto a = th.pop();

	if (a.stringObj == nullptr)
		return "";
	else
	{
		std::string res(a.stringObj->string);
		a.release();
		return res;
	}
}



static void string_of (Thread& th)
{
	Cell res;
	auto n = th.pop().dataInt;
	auto c = th.pop().dataChar;
	auto str =
		(n > 0) ? std::string(n, c)
		        : "";
	th.push(res = Cell::String(str));
	res.release();
}
static void string_len (Thread& th)
{
	auto str = pop_string(th);
	th.push(Cell::Int(str.size()));
}
static void string_add (Thread& th)
{
	Cell res;
	auto str2 = pop_string(th);
	auto str1 = pop_string(th);
	th.push(res = Cell::String(str1 + str2));
	res.release();
}
static void string_get (Thread& th)
{
	auto idx = th.pop().dataInt;
	auto str = pop_string(th);
	if (idx < 0 || idx >= Int_t(str.size()))
		th.die("OutOfRange");
	th.push(Cell::Char(str[idx]));
}
static void string_cmp (Thread& th)
{
	auto str2 = pop_string(th);
	auto str1 = pop_string(th);
	Int_t result = 0;

	size_t min_len = std::min(str1.size(), str2.size());
	for (size_t i = 0; i < min_len; i++)
	{
		result = (str1[i] - str2[i]);
		if (result != 0)
			break;
	}
	if (result == 0)
		result = (str1.size() - str2.size());

	th.push(Cell::Int(
		result < 0 ? -1 :
		result > 0 ? 1 : 0));
}
static void string_equal (Thread& th)
{
	auto str2 = pop_string(th);
	auto str1 = pop_string(th);
	th.push(Cell::Bool(str1 == str2));
}
static void string_find (Thread& th)
{
	auto str2 = pop_string(th);
	auto str1 = pop_string(th);
	size_t pos = str1.find(str2);

	if (pos == std::string::npos)
		th.push(Cell::Int(-1));
	else
		th.push(Cell::Int(pos));
}
static void string_slice (Thread& th)
{
	auto b = th.pop().dataInt;
	auto a = th.pop().dataInt;
	auto str = pop_string(th);

	if (b < 0)
		b = 0;

	if (a < 0 || b <= a || a >= Int_t(str.size()))
		th.push(Cell::String(""));
	else
	{
		Cell res;
		th.push(res = Cell::String(str.substr(a, b - a)));
		res.release();
	}
}


static void int_to_str (Thread& th)
{
	Cell res;
	auto n = th.pop().dataInt;
	std::ostringstream ss; ss << n;
	th.push(res = Cell::String(ss.str()));
	res.release();
}
static void real_to_str (Thread& th)
{
	Cell res;
	auto n = th.pop().dataReal;
	std::ostringstream ss; ss << n;
	if (n == Int_t(n))
		ss << ".0";
	th.push(res = Cell::String(ss.str()));
	res.release();
}
static void long_to_str (Thread& th)
{
	Cell res;
	auto n = th.pop().dataLong;
	std::ostringstream ss; ss << n;
	th.push(res = Cell::String(ss.str()));
	res.release();
}


static void string_to_int (Thread& th)
{
	auto str = pop_string(th);
	auto n = strtoll(str.c_str(), nullptr, 0);
	th.push(Cell::Int(Int_t(n)));
}
static void string_to_long (Thread& th)
{
	auto str = pop_string(th);
	auto n = strtoll(str.c_str(), nullptr, 0);
	th.push(Cell::Long(Long_t(n)));
}
static void string_to_real (Thread& th)
{
	auto str = pop_string(th);
	auto n = strtold(str.c_str(), nullptr);
	th.push(Cell::Real(Real_t(n)));
}



static int chars_lookup[128] =
{
	1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 8, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0,
	10,10,10,10,10,10,10,10,10,10,0, 0, 0, 0, 0, 8,
	8, 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,0, 0, 0, 0, 8,
	0, 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,0, 0, 0, 0, 0
};
static inline bool char_lookup (Char_t c, int flag)
{
	if (c >= 128 || c < 0)
		return false;
	else
		return bool(chars_lookup[int(c)] & flag);
}

static void char_is_space (Thread& th)
{
	auto ch = th.pop().dataChar;
	th.push(Cell::Bool(char_lookup(ch, 1)));
}
static void char_is_digit (Thread& th)
{
	auto ch = th.pop().dataChar;
	th.push(Cell::Bool(char_lookup(ch, 2)));
}
static void char_is_alpha (Thread& th)
{
	auto ch = th.pop().dataChar;
	th.push(Cell::Bool(char_lookup(ch, 4)));
}
static void char_is_ident (Thread& th)
{
	auto ch = th.pop().dataChar;
	th.push(Cell::Bool(char_lookup(ch, 8)));
}



static void loadPackage (Env::Package& pkg)
{
	pkg
	.put("string_of", string_of)
	.put("string.len", string_len)
	.put("string.add", string_add)
	.put("string.get", string_get)
	.put("string.cmp", string_cmp)
	.put("string.equal", string_equal)
	.put("string.find", string_find)
	.put("string.slice", string_slice)
	.put("string.to_int", string_to_int)
	.put("string.to_real", string_to_real)
	.put("string.to_long", string_to_long)
	.put("int.str", int_to_str)
	.put("real.str", real_to_str)
	.put("long.str", long_to_str)
	.put("char.space?", char_is_space)
	.put("char.digit?", char_is_digit)
	.put("char.alpha?", char_is_alpha)
	.put("char.ident?", char_is_ident);
}

static Env::PackageLoad _1("opal.str", loadPackage, { "Core" });
