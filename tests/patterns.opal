use Core

type foo = Foo(int, int, int) or Bar(bool)

fn foo () { Foo(5, 12, 13) }

fn main () {
	let Foo(x, y, z) = foo()

	x + y + z
}