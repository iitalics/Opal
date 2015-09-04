use Core

type foo = Foo(int, int, int) or Bar(bool)

fn main () {
	match Foo(5, 12, 13) {
		Foo(x, y, z) -> x + y + z
		Bar(c) -> 0
	}
}
