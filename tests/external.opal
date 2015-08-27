module Test
use Core

// int types. TODO: declare these seperately
impl x : int {
	fn add (y : int) extern("opal.num") "int.add" -> int
	fn sub (y : int) extern("opal.num") "int.sub" -> int
	fn mul (y : int) extern("opal.num") "int.mul" -> int
	fn div (y : int) extern("opal.num") "int.div" -> int
	fn mod (y : int) extern("opal.num") "int.mod" -> int
	fn cmp (y : int) extern("opal.num") "int.sub" -> int
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