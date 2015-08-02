module Lang

pub type list[#e] {}
pub type int {}
pub type unit {}
pub type string {}

pub iface #t : Ord { fn cmp (#t) : int }
pub iface Show { fn str () : string }

impl x : int {
	fn succ () { x + 1 }
	fn pred () { x - 1 }
	fn cmp (y : int) { x - y }
}

fn repeat (n : int, f : fn(int) -> unit) {
	if n > 0 {
		repeat(n - 1, f)
		f(n)
	}
}

impl list[#a] {
	fn foldl (z : #b, f : fn(#b, #a) -> #b) {
		// ...
	}
}

