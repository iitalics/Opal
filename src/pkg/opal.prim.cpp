#include "../opal.h"
#include "../env/Loader.h"
#include "../runtime/Exec.h"
#include <cmath>
using namespace Opal;
using namespace Opal::Run;

/*  Purely boring stuff  */
///////////////////////////

static void int_add (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Int(a.dataInt + b.dataInt);
	th.push(c);
}
static void int_sub (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Int(a.dataInt - b.dataInt);
	th.push(c);
}
static void int_mul (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Int(a.dataInt * b.dataInt);
	th.push(c);
}
static void int_div (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	if (b.dataInt == 0)
		th.die("DivideByZero");

	auto c = Cell::Int(a.dataInt / b.dataInt);
	th.push(c);
}
static void int_mod (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Int(a.dataInt % b.dataInt);
	th.push(c);
}
static void int_succ (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Int(a.dataInt + 1);
	th.push(b);
}
static void int_pred (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Int(a.dataInt - 1);
	th.push(b);
}
static void int_neg (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Int(-(a.dataInt));
	th.push(b);
}
static void int_equal (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Bool(a.dataInt == b.dataInt);
	th.push(c);
}
static void int_to_real (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Real(Real_t(a.dataInt));
	th.push(b);
}
static void int_to_long (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Long(Long_t(a.dataInt));
	th.push(b);
}
static void int_to_char (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Char(Char_t(a.dataInt));
	th.push(b);
}
static void char_to_int (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Int(Int_t(a.dataChar));
	th.push(b);
}

static void real_add (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Real(a.dataReal + b.dataReal);
	th.push(c);
}
static void real_sub (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Real(a.dataReal - b.dataReal);
	th.push(c);
}
static void real_mul (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Real(a.dataReal * b.dataReal);
	th.push(c);
}
static void real_div (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	if (b.dataReal == 0.0)
		th.die("DivideByZero");
	auto c = Cell::Real(a.dataReal / b.dataReal);
	th.push(c);
}
static void real_mod (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Real(std::fmod(a.dataReal, b.dataReal));
	th.push(c);
}
static void real_succ (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Real(a.dataReal + 1.0);
	th.push(b);
}
static void real_pred (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Real(a.dataReal - 1.0);
	th.push(b);
}
static void real_neg (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Real(-(a.dataReal));
	th.push(b);
}
static void real_cmp (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto diff = a.dataReal - b.dataReal;
	auto c = Cell::Int(
		diff > 0 ? 1 :
		diff < 0 ? -1 : 0);
	th.push(c);
}
static void real_equal (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Bool(a.dataReal == b.dataReal);
	th.push(c);
}
static void real_to_int (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Int(Int_t(a.dataReal));
	th.push(b);
}
static void real_to_long (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Long(Long_t(a.dataReal));
	th.push(b);
}

static void long_add (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Long(a.dataLong + b.dataLong);
	th.push(c);
}
static void long_sub (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Long(a.dataLong - b.dataLong);
	th.push(c);
}
static void long_mul (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Long(a.dataLong * b.dataLong);
	th.push(c);
}
static void long_div (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	if (b.dataLong == 0)
		th.die("DivideByZero");
	auto c = Cell::Long(a.dataLong / b.dataLong);
	th.push(c);
}
static void long_mod (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Long(a.dataLong %b.dataLong);
	th.push(c);
}
static void long_succ (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Long(a.dataLong + 1.0);
	th.push(b);
}
static void long_pred (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Long(a.dataLong - 1.0);
	th.push(b);
}
static void long_neg (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Long(-(a.dataLong));
	th.push(b);
}
static void long_cmp (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto diff = a.dataLong - b.dataLong;
	auto c = Cell::Int(
		diff > 0 ? 1 :
		diff < 0 ? -1 : 0);
	th.push(c);
}
static void long_equal (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Bool(a.dataLong == b.dataLong);
	th.push(c);
}
static void long_to_int (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Int(Int_t(a.dataLong));
	th.push(b);
}
static void long_to_real (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Real(Real_t(a.dataLong));
	th.push(b);
}
static void bool_inv (Thread& th)
{
	auto a = th.pop();
	auto b = Cell::Bool(!a.dataBool);
	th.push(b);
}
static void bool_equal (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Bool(a.dataBool == b.dataBool);
	th.push(c);
}
static void bool_xor (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Cell::Bool(a.dataBool != b.dataBool);
	th.push(c);
}


static void loadPackage (Env::Package& pkg)
{
	pkg
	.put("int.add",    int_add)
	.put("int.sub",    int_sub)
	.put("int.mul",    int_mul)
	.put("int.div",    int_div)
	.put("int.mod",    int_mod)
	.put("int.succ",   int_succ)
	.put("int.pred",   int_pred)
	.put("int.neg",    int_neg)
	.put("int.cmp",    int_sub) // i'm lazy like that
	.put("int.equal",  int_equal)
	.put("int.to_real", int_to_real)
	.put("int.to_long", int_to_long)
	.put("int.to_char", int_to_char)
	.put("char.to_int", char_to_int)
	.put("real.add",   real_add)
	.put("real.sub",   real_sub)
	.put("real.mul",   real_mul)
	.put("real.div",   real_div)
	.put("real.mod",   real_mod)
	.put("real.succ",  real_succ)
	.put("real.pred",  real_pred)
	.put("real.neg",   real_neg)
	.put("real.cmp",   real_cmp)
	.put("real.equal", real_equal)
	.put("real.to_int", real_to_int)
	.put("real.to_long", real_to_long)
	.put("long.add",   long_add)
	.put("long.sub",   long_sub)
	.put("long.mul",   long_mul)
	.put("long.div",   long_div)
	.put("long.mod",   long_mod)
	.put("long.succ",  long_succ)
	.put("long.pred",  long_pred)
	.put("long.neg",   long_neg)
	.put("long.cmp",   long_cmp)
	.put("long.equal", long_equal)
	.put("long.to_int", long_to_int)
	.put("long.to_real", long_to_real)
	.put("bool.inv",   bool_inv)
	.put("bool.equal", bool_equal)
	.put("bool.xor",   bool_xor);
}
static Env::PackageLoad _1("opal.prim", loadPackage, { "Core" });
