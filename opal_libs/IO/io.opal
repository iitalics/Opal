module IO

pub type console {}
pub type in_file extern true
pub type out_file extern true



impl console {
	fn read_line ()       extern("opal.io") "console.read_line" -> string
	fn eof? ()            extern("opal.io") "console.eof?" -> bool
	fn write (x : string) extern("opal.io") "console.write" -> unit

	fn prompt (s : string) {
		self.write(s);
		self.read_line()
	}
}
pub fn console () { new console {} }


impl in_file {
	fn read_line () extern("opal.io") "in_file.read_line" -> string
	fn eof? ()      extern("opal.io") "in_file.eof?" -> bool
}
pub fn open_in (filename : string)  extern("opal.io") "open_in" -> in_file


impl out_file {
	fn write (what : string) extern("opal.io") "out_file.write" -> unit
}
pub fn open_out (filename : string) extern("opal.io") "open_out" -> out_file



pub iface InStream {
	fn read_line () -> string
	fn eof? () -> bool
}
pub iface OutStream {
	fn write (string) -> unit
}

pub fn file_exists? (filename : string)
	extern("opal.io") "file_exists?" -> bool

pub fn print (what : #a(Lang::Show)) {
	console().write(what.str())
}
