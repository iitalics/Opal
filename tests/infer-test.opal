module InferTest
use Core


// result: Core::string
fn fn_a () {
	"hello, world"
}


//	// result: Core::int
//	fn fn_b (x : int) {
//		x
//	}
//	
//	pub type foo {
//		x : int,
//		y : real,
//		z : string
//	}
//	// result: Core::int
//	fn fn_c (foo : foo) {
//		foo.x
//	}
//	
//	impl foo {
//		// result: Core::real
//		fn a () {
//			5.1
//		}
//		// result: Core::string
//		fn b (x : int) {
//			self.z
//		}
//		// result: InferTest::foo
//		fn c (y : real) { self }
//	}
//	
//	// result: Core::@fn(0)[Core::real]
//	fn fn_d (foo : foo) {
//		foo.a
//	}
//	// result: Core::string
//	fn fn_e (foo : foo) {
//		foo.b(4)
//	}
//	// reuslt: InferTest::foo
//	fn fn_f (foo : foo) {
//		foo.c(0)
//	}
