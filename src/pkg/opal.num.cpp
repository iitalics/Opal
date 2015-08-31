#include "../opal.h"
#include "../env/Loader.h"
#include "../runtime/Exec.h"
#include <cmath>
using namespace Opal;

/*  Purely boring stuff  */
///////////////////////////

static void int_add (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Int(a.dataInt + b.dataInt);
	th.push(c);
}
static void int_sub (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Int(a.dataInt - b.dataInt);
	th.push(c);
}
static void int_mul (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Int(a.dataInt * b.dataInt);
	th.push(c);
}
static void int_div (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	if (b.dataInt == 0)
		th.die("DivideByZero");

	auto c = Run::Cell::Int(a.dataInt / b.dataInt);
	th.push(c);
}
static void int_mod (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Int(a.dataInt % b.dataInt);
	th.push(c);
}
static void int_succ (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Int(a.dataInt + 1);
	th.push(b);
}
static void int_pred (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Int(a.dataInt - 1);
	th.push(b);
}
static void int_neg (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Int(-(a.dataInt));
	th.push(b);
}
static void int_equal (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Bool(a.dataInt == b.dataInt);
	th.push(c);
}
static void int_to_real (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Real(Real_t(a.dataInt));
	th.push(b);
}
static void int_to_long (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Long(Long_t(a.dataInt));
	th.push(b);
}
static void int_to_char (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Char(Char_t(a.dataInt));
	th.push(b);
}
static void char_to_int (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Int(Int_t(a.dataChar));
	th.push(b);
}

static void real_add (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Real(a.dataReal + b.dataReal);
	th.push(c);
}
static void real_sub (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Real(a.dataReal - b.dataReal);
	th.push(c);
}
static void real_mul (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Real(a.dataReal * b.dataReal);
	th.push(c);
}
static void real_div (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	if (b.dataReal == 0.0)
		th.die("DivideByZero");
	auto c = Run::Cell::Real(a.dataReal / b.dataReal);
	th.push(c);
}
static void real_mod (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Real(std::fmod(a.dataReal, b.dataReal));
	th.push(c);
}
static void real_succ (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Real(a.dataReal + 1.0);
	th.push(b);
}
static void real_pred (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Real(a.dataReal - 1.0);
	th.push(b);
}
static void real_neg (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Real(-(a.dataReal));
	th.push(b);
}
static void real_cmp (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto diff = a.dataReal - b.dataReal;
	auto c = Run::Cell::Int(
		diff > 0 ? 1 :
		diff < 0 ? -1 : 0);
	th.push(c);
}
static void real_equal (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Bool(a.dataReal == b.dataReal);
	th.push(c);
}
static void real_to_int (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Int(Int_t(a.dataReal));
	th.push(b);
}
static void real_to_long (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Long(Long_t(a.dataReal));
	th.push(b);
}

static void long_add (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Long(a.dataLong + b.dataLong);
	th.push(c);
}
static void long_sub (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Long(a.dataLong - b.dataLong);
	th.push(c);
}
static void long_mul (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Long(a.dataLong * b.dataLong);
	th.push(c);
}
static void long_div (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	if (b.dataLong == 0)
		th.die("DivideByZero");
	auto c = Run::Cell::Long(a.dataLong / b.dataLong);
	th.push(c);
}
static void long_mod (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Long(a.dataLong %b.dataLong);
	th.push(c);
}
static void long_succ (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Long(a.dataLong + 1.0);
	th.push(b);
}
static void long_pred (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Long(a.dataLong - 1.0);
	th.push(b);
}
static void long_neg (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Long(-(a.dataLong));
	th.push(b);
}
static void long_cmp (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto diff = a.dataLong - b.dataLong;
	auto c = Run::Cell::Int(
		diff > 0 ? 1 :
		diff < 0 ? -1 : 0);
	th.push(c);
}
static void long_equal (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Bool(a.dataLong == b.dataLong);
	th.push(c);
}
static void long_to_int (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Int(Int_t(a.dataLong));
	th.push(b);
}
static void long_to_real (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Real(Real_t(a.dataLong));
	th.push(b);
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
	.put("long.to_real", long_to_real);

	std::cout << "load package '" << pkg.name() << "' is a success" << std::endl;
}
static Env::PackageLoad _1("opal.num", loadPackage, { "Core" });
