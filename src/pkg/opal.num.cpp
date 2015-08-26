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
	// no release
}
static void int_sub (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Int(a.dataInt - b.dataInt);
	th.push(c);
	// no release
}
static void int_mul (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Int(a.dataInt * b.dataInt);
	th.push(c);
	// no release
}
static void int_div (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	// if (b.dataInt == 0) throw ...
	auto c = Run::Cell::Int(a.dataInt / b.dataInt);
	th.push(c);
	// no release
}
static void int_mod (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Int(a.dataInt % b.dataInt);
	th.push(c);
	// no release
}
static void int_succ (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Int(a.dataInt + 1);
	th.push(b);
	// no release
}
static void int_pred (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Int(a.dataInt - 1);
	th.push(b);
	// no release
}

static void real_add (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Int(a.dataReal + b.dataReal);
	th.push(c);
	// no release
}
static void real_sub (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Int(a.dataReal - b.dataReal);
	th.push(c);
	// no release
}
static void real_mul (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Int(a.dataReal * b.dataReal);
	th.push(c);
	// no release
}
static void real_div (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	// if (b.dataReal == 0) throw ...
	auto c = Run::Cell::Int(a.dataReal / b.dataReal);
	th.push(c);
	// no release
}
static void real_mod (Run::Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	auto c = Run::Cell::Int(std::fmod(a.dataReal, b.dataReal));
	th.push(c);
	// no release
}
static void real_succ (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Int(a.dataReal + 1.0);
	th.push(b);
	// no release
}
static void real_pred (Run::Thread& th)
{
	auto a = th.pop();
	auto b = Run::Cell::Int(a.dataReal - 1.0);
	th.push(b);
	// no release
}


static void loadPackage (Env::Package& pkg)
{
	pkg
	.put("int.add", int_add)
	.put("int.sub", int_sub)
	.put("int.mul", int_mul)
	.put("int.div", int_div)
	.put("int.mod", int_mod)
	.put("int.succ", int_succ)
	.put("int.pred", int_pred)
	.put("real.add", real_add)
	.put("real.sub", real_sub)
	.put("real.mul", real_mul)
	.put("real.div", real_div)
	.put("real.mod", real_mod)
	.put("real.succ", real_succ)
	.put("real.pred", real_pred);

	std::cout << "load package '" << pkg.name() << "' is a success" << std::endl;
}
static Env::PackageLoad _1("opal.num", loadPackage);
