#include "../opal.h"
#include "../env/Loader.h"
#include "../runtime/Exec.h"

namespace OpalListPkg {
using namespace Opal;
using namespace Opal::Run;


struct Array : public GC::Object
{
	static Env::Type* type;

	Int_t len, cap;
	Cell* buffer;

	enum {
		InitialCap = 4
	};

	Array ()
		: len(0), cap(0), buffer(nullptr)
	{}
	virtual ~Array ()
	{
		for (Int_t i = 0; i < len; i++)
			buffer[i].release();
		delete[] buffer;
	}
	virtual void markChildren ()
	{
		for (Int_t i = 0; i < len; i++)
			buffer[i].mark();
	}

	void resize (Int_t to)
	{
		auto new_cap = cap;
		if (cap == 0)
			new_cap = InitialCap;
		while (new_cap < to)
			new_cap *= 2;

		if (new_cap == cap) return;

		auto new_buf = new Cell[new_cap];

		if (buffer)
		{
			for (Int_t i = 0; i < len; i++)
				new_buf[i] = buffer[i];
			delete[] buffer;
		}
		buffer = new_buf;
		cap = new_cap;
	}
	void set (Int_t idx, Cell val)
	{
		if (idx == len)
			resize(++len);
		else
			buffer[idx].release();

		buffer[idx] = val.retain();
	}
	Cell get (Int_t idx)
	{
		return buffer[idx];
	}
	void insert (Int_t idx, Cell val)
	{
		// len = 4
		// insert(0, v)
		//   set(4, get(3))
		//   set(3, get(2))
		//   set(2, get(1))
		//   set(1, get(0))
		//   set(0, v)
		for (Int_t i = len; i > idx; i--)
			set(i, get(i - 1));
		set(idx, val);
	}
	void remove (Int_t idx)
	{
		// let = 4
		// remove(2)
		//   set(2, get(3))
		//   len--
		for (Int_t i = idx; i < len - 1; i++)
			set(i, get(i + 1));
		len--;
	}
};
Env::Type* Array::type = nullptr;

/*
impl array[#e] {
	fn len () -> int
	fn cap () -> int
	fn cap_set (n : int) -> unit

	fn get (i : int) -> #e
	fn set (i : int, t : #e) -> unit

	fn push (t : #e) -> unit
	fn pop () -> #e

	fn insert (i : int, t : #e) -> unit
}
*/
void array_ctor (Thread& th)
{
	auto obj = new Array();
	th.push(Cell::Object(Array::type, obj));
}
void array_len (Thread& th)
{
	auto self = th.pop();
	auto obj = (Array*) (self.obj);
	th.push(Cell::Int(obj->len));
	self.release();
}
void array_cap (Thread& th)
{
	auto self = th.pop();
	auto obj = (Array*) (self.obj);
	th.push(Cell::Int(obj->cap));
	self.release();
}
void array_cap_set (Thread& th)
{
	auto a = th.pop();
	auto new_cap = a.dataInt;
	auto self = th.pop();
	auto obj = (Array*) (self.obj);
	if (new_cap >= 0)
		obj->resize(new_cap);
	th.push(Cell::Unit());
	self.release();
	a.release();
}
void array_get (Thread& th)
{
	auto a = th.pop();
	auto idx = a.dataInt;
	auto self = th.pop();
	auto obj = (Array*) (self.obj);
	if (idx < 0 || idx >= obj->len)
		th.die("OutOfRange");
	th.push(obj->get(idx));
	self.release();
	a.release();
}
void array_set (Thread& th)
{
	auto val = th.pop();
	auto a = th.pop();
	auto idx = a.dataInt;
	auto self = th.pop();
	auto obj = (Array*) (self.obj);
	if (idx < 0 || idx >= obj->len + 1)
		th.die("OutOfRange");
	obj->set(idx, val);
	th.push(Cell::Unit());
	self.release();
	a.release();
	val.release();
}
void array_insert (Thread& th)
{
	auto val = th.pop();
	auto a = th.pop();
	auto idx = a.dataInt;
	auto self = th.pop();
	auto obj = (Array*) (self.obj);
	if (idx < 0 || idx >= obj->len + 1)
		th.die("OutOfRange");
	obj->insert(idx, val);
	th.push(Cell::Unit());
	self.release();
	a.release();
	val.release();
}
void array_remove (Thread& th)
{
	auto a = th.pop();
	auto idx = a.dataInt;
	auto self = th.pop();
	auto obj = (Array*) (self.obj);
	if (idx >= 0 && idx < obj->len)
		obj->remove(idx);
	th.push(Cell::Unit());
	self.release();
	a.release();
}

static void loadPackage (Env::Package& pkg)
{
	auto modLang = Env::loadModule("Lang");
	if (!(Array::type = modLang->getType("array")))
		throw SourceError(pkg.name() + ":  no 'array' type defined");

	pkg
	.put("array.len", array_len)
	.put("array.cap", array_cap)
	.put("array.cap_set", array_cap_set)
	.put("array.get", array_get)
	.put("array.set", array_set)
	.put("array.insert", array_insert)
	.put("array.remove", array_remove)
	.put("array", array_ctor);
}

static Env::PackageLoad _1("opal.list", loadPackage, { "Lang" });

}
