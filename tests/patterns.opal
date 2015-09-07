module AST
use Core

type exp =
	Add(exp, exp) or
	Mul(exp, exp) or
	Num(real)


impl exp {
	fn eval () {
		match self {
			Num(v) -> v
			Add(e1, e2) -> e1.eval() + e2.eval()
			Mul(e1, e2) -> e1.eval() * e2.eval()
		}
	}
}

fn main () {
	let exp = 
		Mul(Num(.5),
			 Add(Num(4),
				  Num(3)))

	exp.eval()
}
