module InferTest
use Core


// result: Core::string
pub fn fn_a () {
	"hello"
}

// result: Core::int
pub fn fn_b (x : int) {
	x
}

pub type foo {
	x : int,
	y : real,
	z : string
}
// result: Core::int
pub fn fn_c (foo : foo) {
	foo.x
}

impl foo {
	// result: Core::real
	fn a () {
		5.1
	}
	// result: Core::string
	fn b (x : int) {
		self.z
	}
	// result: InferTest::foo
	fn c (y : real) { self }
}

// result: Core::fn[Core::real]
pub fn fn_d (foo : foo) {
	foo.a
}
// result: Core::string
pub fn fn_e (foo : foo) {
	foo.b(4)
}
// reuslt: Core::int
pub fn fn_f (foo : foo) {
	foo.c(0).x
}
