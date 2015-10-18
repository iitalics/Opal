module Core

impl string {
	fn len ()              extern("opal.str") "string.len" -> int
	fn add (x : string)    extern("opal.str") "string.add" -> string
	fn get (i : int)       extern("opal.str") "string.get" -> char
	fn cmp (x : string)    extern("opal.str") "string.cmp" -> int
	fn equal (x : string)  extern("opal.str") "string.equal" -> bool
	fn find (x : string)   extern("opal.str") "string.find" -> int
	fn slice (start : int, end : int)
	                       extern("opal.str") "string.slice" -> string
	fn to_int ()           extern("opal.str") "string.to_int" -> int
	fn to_long ()          extern("opal.str") "string.to_long" -> long
	fn to_real ()          extern("opal.str") "string.to_real" -> real

	fn empty? () { self.len() == 0 }
	fn slice_from (n : int) { self.slice(n, self.len()) }
	fn slice_to (n : int) { self.slice(0, n) }
	fn mul (n : int) {
		if n <= 0 { "" }
		else if n == 1 { self }
		else {
			let res = "";
			while n > 0 {
				res = res + self;
				n = n.pred();
			}
			res
		}
	}

	fn str () { self }
}
impl int {  fn str () extern("opal.str") "int.str"  -> string }
impl real { fn str () extern("opal.str") "real.str" -> string }
impl long { fn str () extern("opal.str") "long.str" -> string }
impl char { fn str () { string_of(self, 1) }}
impl bool { fn str () { if self { "true" } else { "false" } }}

pub fn string_of (c : char, n : int)
	extern("opal.str") "string_of" -> string


impl char {
	fn space? () extern("opal.str") "char.space?" -> bool
	fn digit? () extern("opal.str") "char.digit?" -> bool
	fn alpha? () extern("opal.str") "char.alpha?" -> bool
	fn ident? () extern("opal.str") "char.ident?" -> bool
	// TODO: add these
//	fn upper ()  extern("opal.str") "char.upper" -> char
//	fn lower ()  extern("opal.str") "char.loewr" -> char

	fn add (n : int) { (self.to_int() + n).to_char() }
	fn sub (c : char) { self.to_int() - c.to_int() }
	fn mul (n : int) { string_of(self, n) }
}
