module Core

impl string {
	fn len ()                  extern("opal.str") "string.len" -> int
	fn add (x : string)        extern("opal.str") "string.add" -> string
	fn get (i : int)           extern("opal.str") "string.get" -> char
	fn cmp (x : string)        extern("opal.str") "string.cmp" -> int
	fn equal (x : string)      extern("opal.str") "string.equal" -> bool
	fn find (x : string)       extern("opal.str") "string.find" -> int
	fn sub (a : int, b : int)  extern("opal.str") "string.sub" -> string
//	fn to_int ()               extern("opal.str") "string.to_int" -> int
//	fn to_real ()              extern("opal.str") "string.to_real" -> real
//	fn to_bool ()              extern("opal.str") "string.to_bool" -> bool

	fn str () { self }
}
//pub fn string_of (c : char, n : int)
//	extern("opal.str") "string_of" -> string
