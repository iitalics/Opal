#include "../opal.h"
#include "../env/Loader.h"
#include "../runtime/Exec.h"
#include <cmath>

namespace OpalMathPkg {
using namespace Opal;
using namespace Opal::Run;


static void math_sin (Thread& th)
{
	auto a = th.pop();
	th.push(Cell::Real(std::sin(a.dataReal)));
}
static void math_cos (Thread& th)
{
	auto a = th.pop();
	th.push(Cell::Real(std::cos(a.dataReal)));
}
static void math_tan (Thread& th)
{
	auto a = th.pop();
	th.push(Cell::Real(std::tan(a.dataReal)));
}
static void math_asin (Thread& th)
{
	auto a = th.pop();
	th.push(Cell::Real(std::asin(a.dataReal)));
}
static void math_acos (Thread& th)
{
	auto a = th.pop();
	th.push(Cell::Real(std::acos(a.dataReal)));
}
static void math_atan (Thread& th)
{
	auto a = th.pop();
	th.push(Cell::Real(std::atan(a.dataReal)));
}
static void math_atan2 (Thread& th)
{
	auto b = th.pop();
	auto a = th.pop();
	th.push(Cell::Real(std::atan2(a.dataReal, b.dataReal)));
}
static void math_exp (Thread& th)
{
	auto a = th.pop();
	th.push(Cell::Real(std::exp(a.dataReal)));
}
static void math_log (Thread& th)
{
	auto a = th.pop();
	th.push(Cell::Real(std::log(a.dataReal)));
}
static void math_log2 (Thread& th)
{
	auto a = th.pop();
	th.push(Cell::Real(log2(a.dataReal)));
}
static void math_sqrt (Thread& th)
{
	auto a = th.pop();
	th.push(Cell::Real(std::sqrt(a.dataReal)));
}


static void loadPackage (Env::Package& pkg)
{
	pkg
	.put("sin",    math_sin)
	.put("cos",    math_cos)
	.put("tan",    math_tan)
	.put("asin",   math_asin)
	.put("acos",   math_acos)
	.put("atan",   math_atan)
	.put("atan2",  math_atan2)
	.put("exp",    math_exp)
	.put("log",    math_log)
	.put("log2",   math_log2)
	.put("sqrt",   math_sqrt);
}
static Env::PackageLoad _1("opal.math", loadPackage, { "Core" });


};