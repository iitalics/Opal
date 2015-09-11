use Core
module Monad

impl f : fn(#a) -> #b {
	fn rbind (x : #a) { f(x) }

	fn exp (g : fn(#a') -> #a) {
		fn (x) { f(g(x)) }
	}
}


pub type opt[#a] = Some(#a) or None()
impl opt[#a] {
	fn rshift (f : fn(#a) -> opt[#b]) {
		match self {
			Some(x) -> f(x)
			None() -> None()
		}
	}
	fn rblock (f : fn() -> #a) {
		match self {
			Some(x) -> x
			None() -> f()
		}
	}
}

fn lazy (x : #a) { fn () { x } }


fn main () {
	match Some(4) >> (Some ^ .succ) {
		Some(x) -> x
		None() -> 0
	}
}
