use Core
use Lang

// method of approximating square roots
fn sqrt (y : real) {
	let x = 1.0

	{(0, 16)}.range <| fn (_) {
		x = (x + y / x) / 2
	}

	; x
}

// quadratic equation
fn quadratic (a : real, b : real, c : real) {
	let P = -b / (a * 2)
	let Q = sqrt(b * b - a * c * 4) / (a * 2)

	; (P + Q, P - Q)
}

impl n : real { fn sign_char () {
	if n < 0.0 { '-' } else { '+' }
}}

impl n : real { fn abs () {
	if n < 0.0 { -n } else { n }
}}

fn main () {
	// x^2 - x - 6 = 0
	// (x - 3)(x + 2) = 0

	let a = 1.0
	let b = -1.0
	let c = -6.0

	let (ans1, ans2) = quadratic(a, b, c)

	"(x {} {})(x {} {})" %
		[(-ans1).sign_char(), ans1.abs(),
		 (-ans2).sign_char(), ans2.abs()]
}

