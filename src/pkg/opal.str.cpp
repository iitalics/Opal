#include "../opal.h"
#include "../env/Loader.h"
#include "../runtime/Exec.h"
#include <cmath>
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
	auto n = th.pop().dataInt;
	auto c = th.pop().dataChar;
	auto str =
		(n > 0) ? std::string(n, c)
		        : "";
	th.push(Cell::String(str));
}
static void string_len (Thread& th)
{
	auto str = pop_string(th);
	th.push(Cell::Int(str.size()));
}
static void string_add (Thread& th)
{
	auto str2 = pop_string(th);
	auto str1 = pop_string(th);
	th.push(Cell::String(str1 + str2));
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
static void string_sub (Thread& th)
{
	auto b = th.pop().dataInt;
	auto a = th.pop().dataInt;
	auto str = pop_string(th);

	if (b <= a || a >= Int_t(str.size()))
		th.push(Cell::String(""));
	else
		th.push(Cell::String(str.substr(a, b - a)));
}


static void int_to_str (Thread& th)
{
	auto n = th.pop().dataInt;
	std::ostringstream ss; ss << n;
	th.push(Cell::String(ss.str()));
}
static void real_to_str (Thread& th)
{
	auto n = th.pop().dataReal;
	std::ostringstream ss; ss << n;
	if (n == Int_t(n))
		ss << ".0";
	th.push(Cell::String(ss.str()));
}
static void long_to_str (Thread& th)
{
	auto n = th.pop().dataLong;
	std::ostringstream ss; ss << n;
	th.push(Cell::String(ss.str()));
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
	.put("string.sub", string_sub)
	//.put("string.to_int", string_to_int)
	//.put("string.to_real", string_to_real)
	//.put("string.to_long", string_to_long)
	.put("int.str", int_to_str)
	.put("real.str", real_to_str)
	.put("long.str", long_to_str);
}

static Env::PackageLoad _1("opal.str", loadPackage, { "Core" });
