#include "../opal.h"
#include "../env/Loader.h"
#include "../runtime/Exec.h"
#include <fstream>

namespace OpalIOPkg {
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
static std::string read_line (std::istream& s)
{
	const int SIZE = 256;
	const char DELIM = '\n';
	char buffer[SIZE];

	std::ostringstream ss;
	for (;;)
	{
		if (s.eof())
			break;
		if (s.peek() == DELIM)
		{
			ss << DELIM;
			s.get();
			break;
		}

		s.get(buffer, SIZE, DELIM);
		ss << std::string(buffer);
	}

	return ss.str();
}


struct FileObject : GC::Object
{
	static Env::Type* in_type, *out_type;

	union {
		std::ifstream* ifs;
		std::ofstream* ofs;
		std::ofstream* fs;
	};
	bool is_input;

	FileObject (bool is_in) : is_input(is_in) {}

	virtual ~FileObject ()
	{
		if (is_input)
			delete ifs;
		else
			delete ofs;
	}
};
Env::Type* FileObject::in_type = nullptr;
Env::Type* FileObject::out_type = nullptr;


void console_read_line (Thread& th)
{
	th.drop();
	auto line = read_line(std::cin);
	auto str = Cell::String(line);
	th.push(str);
	str.release();
}
void console_eof (Thread& th)
{
	th.drop();
	th.push(Cell::Bool(std::cin.eof()));
}
void console_write (Thread& th)
{
	auto str = pop_string(th);
	th.drop();
	std::cout << str;
	std::cout.flush();
	th.push(Cell::Unit());
}

void file_exists (Thread& th)
{
	auto filename = pop_string(th);
	std::ifstream ifs(filename);
	th.push(Cell::Bool(ifs.good()));
}
void open_in (Thread& th)
{
	auto filename = pop_string(th);
	auto fs = new std::ifstream(filename);
	if (!fs->good())
	{
		delete fs;
		th.die("NoSuchFile");
	}
	auto file = new FileObject(true);
	file->ifs = fs;
	auto obj = Cell::Object(FileObject::in_type, file);
	th.push(obj);
	obj.release();
}
void open_out (Thread& th)
{
	auto filename = pop_string(th);
	auto fs = new std::ofstream(filename);
	auto file = new FileObject(false);
	file->ofs = fs;
	auto obj = Cell::Object(FileObject::out_type, file);
	th.push(obj);
	obj.release();
}

void in_file_read_line (Thread& th)
{
	auto obj = th.pop();
	auto file = (FileObject*) obj.obj;

	auto line = read_line(*(file->ifs));
	auto str = Cell::String(line);
	th.push(str);
	str.release();
	obj.release();
}
void in_file_eof (Thread& th)
{
	auto obj = th.pop();
	auto file = (FileObject*) obj.obj;

	bool eof = (file->ifs->peek() == EOF);
	th.push(Cell::Bool(eof));
	obj.release();
}
void out_file_write (Thread& th)
{
	auto what = pop_string(th);
	auto obj = th.pop();
	auto file = (FileObject*) obj.obj;

	*(file->ofs) << what;
	file->ofs->flush();
	th.push(Cell::Unit());
	obj.release();
}

static void loadPackage (Env::Package& pkg)
{
	auto modIO = Env::loadModule("IO");
	if (!(FileObject::in_type = modIO->getType("in_file")))
		throw SourceError(pkg.name() + ":  no 'in_file' type defined");
	if (!(FileObject::out_type = modIO->getType("out_file")))
		throw SourceError(pkg.name() + ":  no 'out_file' type defined");

	pkg
	.put("console.read_line", console_read_line)
	.put("console.eof?", console_eof)
	.put("console.write", console_write)
	.put("file_exists?", file_exists)
	.put("open_in", open_in)
	.put("open_out", open_out)
	.put("in_file.read_line", in_file_read_line)
	.put("in_file.eof?", in_file_eof)
	.put("out_file.write", out_file_write);
}

static Env::PackageLoad _1("opal.io", loadPackage, { "IO" });

}