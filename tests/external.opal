module Test
use Core


// number types. TODO: declare these in a dedicated file
impl x : int {
	fn add (y : int)   extern("opal.num") "int.add" -> int
	fn sub (y : int)   extern("opal.num") "int.sub" -> int
	fn mul (y : int)   extern("opal.num") "int.mul" -> int
	fn div (y : int)   extern("opal.num") "int.div" -> int
	fn mod (y : int)   extern("opal.num") "int.mod" -> int
	fn succ ()         extern("opal.num") "int.succ" -> int
	fn pred ()         extern("opal.num") "int.pred" -> int
	fn cmp (y : int)   extern("opal.num") "int.cmp" -> int
	fn equal (y : int) extern("opal.num") "int.equal" -> bool

	fn to_int () { x }
	fn to_real () extern("opal.num") "int.to_real" -> real
	fn to_long () extern("opal.num") "int.to_long" -> long
	fn to_char () extern("opal.num") "int.to_char" -> char
}
impl x : real {
	fn add (y : real)   extern("opal.num") "real.add" -> real
	fn sub (y : real)   extern("opal.num") "real.sub" -> real
	fn mul (y : real)   extern("opal.num") "real.mul" -> real
	fn div (y : real)   extern("opal.num") "real.div" -> real
	fn mod (y : real)   extern("opal.num") "real.mod" -> real
	fn succ ()          extern("opal.num") "real.succ" -> real
	fn pred ()          extern("opal.num") "real.pred" -> real
	fn cmp (y : real)   extern("opal.num") "real.cmp" -> int
	fn equal (y : real) extern("opal.num") "real.equal" -> bool

	fn to_int () extern("opal.num") "real.to_int" -> int
	fn to_real () { x }
	fn to_long () extern("opal.num") "real.to_long" -> long
}
impl x : long {
	fn add (y : long)   extern("opal.num") "long.add" -> long
	fn sub (y : long)   extern("opal.num") "long.sub" -> long
	fn mul (y : long)   extern("opal.num") "long.mul" -> long
	fn div (y : long)   extern("opal.num") "long.div" -> long
	fn mod (y : long)   extern("opal.num") "long.mod" -> long
	fn succ ()          extern("opal.num") "long.succ" -> long
	fn pred ()          extern("opal.num") "long.pred" -> long
	fn cmp (y : long)   extern("opal.num") "long.cmp" -> int
	fn equal (y : long) extern("opal.num") "long.equal" -> bool

	fn to_int () extern("opal.num") "long.to_int" -> int
	fn to_real () extern("opal.num") "long.to_real" -> real
	fn to_long () { x }
}
impl x : char {
	fn to_int () extern("opal.num") "char.to_int" -> int
}


// external 'foo' type
// "true" means "garbage collected"
pub type foo extern true
impl foo {
	fn advance ()
		extern("opal.test") "foo.advance" -> unit
	fn str ()
		extern("opal.test") "foo.str" -> string
}
fn foo () extern("opal.test") "foo" -> foo

// simple utility
fn repeat (n : int, f : fn() -> unit) {
	if n > 0 {
		f()
		repeat(n - 1, f)
	}
}

fn main () {
	let my_foo = foo()

	repeat(5, my_foo.advance)
	my_foo.str()
}