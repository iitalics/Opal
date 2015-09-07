module AST
use Core

impl list[#e] {
	fn len () {
		match self {
			[] -> 0
			_ $ xs -> 1 + xs.len()
		}
	}

	fn foldl (z : #e', f : fn(#e', #e) -> #e') {
		match self {
			[] -> z
			x $ xs -> xs.foldl(f(z, x), f)
		}
	}
}


fn main () {
	let numbers = [1, 2, 3, 4]
	let sum = numbers.foldl(0, .add)
	let prod = numbers.foldl(1, .mul)

	sum + prod
}