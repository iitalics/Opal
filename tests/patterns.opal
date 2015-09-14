use Core
use Lang
module Monad

fn less? (a : #a(Ord), b : #a) { a < b }

impl list[#a] {
	fn part (f : fn(#a) -> bool) {
		let xs = []
		let ys = []
		self.each <| fn (x) {
			if f(x) {
				xs = x $ xs
			} else {
				ys = x $ ys
			}
		}
		; (xs, ys)
	}
}
impl list[#a(Ord)] {
	fn sort () {
		match self {
			[] -> self
			x $ [] -> self
			p $ xs {
				let (left, right) = xs.part(less? |> p)

				left.sort() + [p] + right.sort()
			}
		}
	}
}

fn main () {
	[5, 1, 3, 8, 5, 6].sort()
}
