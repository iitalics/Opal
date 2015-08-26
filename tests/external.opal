use Core

impl x : int {
	fn add (y : int) extern("opal.num") "int.add" -> int
	fn sub (y : int) extern("opal.num") "int.sub" -> int
	fn mul (y : int) extern("opal.num") "int.mul" -> int
	fn div (y : int) extern("opal.num") "int.div" -> int
	fn mod (y : int) extern("opal.num") "int.mod" -> int
	fn cmp (y : int) extern("opal.num") "int.sub" -> int
}

fn foo () extern("opal.test") "foo" -> unit

fn main () {
	foo()

	5 * 3 + 4 * 2
}