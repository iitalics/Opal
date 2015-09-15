#include "../opal.h"
#include "../env/Loader.h"
#include "../runtime/Exec.h"

using namespace Opal;
using namespace Opal::Run;


void string_len (Thread& th)
{
	auto a = th.pop();
	Int_t result;
	if (a.stringObj)
		result = a.stringObj->string.size();
	else
		result = 0;
	th.push(Cell::Int(result));
	a.release();
}
void string_add (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	th.push(Cell::String(
		a.stringObj->string +
		b.stringObj->string));
	a.release();
	b.release();
}
void string_get (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto str = a.stringObj->string;
	auto idx = b.dataInt;
	if (idx < 0 || idx >= Int_t(str.size()))
		th.die("OutOfRange");
	th.push(Cell::Char(str[idx]));
	a.release();
}
void string_cmp (Thread& th)
{
	Int_t result = 0;
	auto b = th.pop();
	auto a = th.pop();
	auto len1 = a.stringObj ? a.stringObj->string.size() : 0;
	auto len2 = b.stringObj ? b.stringObj->string.size() : 0;

	for (size_t i = 0; i < len1 && i < len2; i++)
	{
		result = (a.stringObj->string[i] - b.stringObj->string[i]);
		if (result != 0)
			break;
	}
	if (result == 0)
		result = Int_t(len1 - len2);

	th.push(Cell::Int(result));
	a.release();
	b.release();
}
void string_equal (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	bool result;

	if (a.stringObj && b.stringObj)
		result =
			(a.stringObj->string == b.stringObj->string);
	else
		result = (a.stringObj == b.stringObj);

	th.push(Cell::Bool(result));
	a.release();
	b.release();
}
void string_find (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	Int_t result;

	if (b.stringObj == nullptr)
		result = 0;
	else if (a.stringObj)
	{
		auto pos = a.stringObj->string.find(b.stringObj->string);

		if (pos == std::string::npos)
			result = (-1);
		else
			result = Int_t(pos);
	}
	else
		result = (-1);

	th.push(Cell::Int(result));
	a.release();
	b.release();
}
void string_sub (Thread& th)
{
	auto c = th.pop();
	auto b = th.pop();
	auto a = th.pop();

	auto min = b.dataInt;
	auto max = c.dataInt;

	std::string result;

	if (max <= min || a.stringObj == nullptr)
		result = "";
	else
		result = a.stringObj->string.substr(min, max - min);

	th.push(Cell::String(result));
	a.release();
}



static void loadPackage (Env::Package& pkg)
{
	pkg
	.put("string.len", string_len)
	.put("string.add", string_add)
	.put("string.get", string_get)
	.put("string.cmp", string_cmp)
	.put("string.equal", string_equal)
	.put("string.find", string_find)
	.put("string.sub", string_sub);
}

static Env::PackageLoad _1("opal.str", loadPackage, { "Core" });
