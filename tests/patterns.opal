module AST
use Core

type exp =
	Add(exp, exp) or
	Num(real)


fn main () {
	let Add(Num(x), Num(y)) = Add(Num(4), Num(5))

	x + y
}
