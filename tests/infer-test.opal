module Test
use Core



// test 1  constants
//   Test::test1_0 -> Core::int
//   Test::test1_1 -> Core::real
//   Test::test1_2 -> Core::bool
//   Test::test1_3 -> Core::string
//   Test::test1_4 -> (Core::int, Core::bool)
//   Test::test1_5 -> Core::unit
//   Test::test1_6 -> Core::unit
pub fn test1_0 () { 0 }
pub fn test1_1 () { 0.0 }
pub fn test1_2 () { false }
pub fn test1_3 () { "zero" }
pub fn test1_4 () { (1, true) }
pub fn test1_5 () { () }
pub fn test1_6 () { }

// test 2  variables (global or local)
//   Test::test2_0 -> Core::int
//   Test::test2_1 -> fn(Core::int) -> Core::int
//   Test::test2_2 -> #a
//   Test::test2_3 -> fn(#0, #1) -> #0
pub fn test2_0 (x : int) { x }
pub fn test2_1 () { test2_0 }
pub fn test2_2 (x : #a, y : #b) { x }
pub fn test2_3 () { Test::test2_2 }

// test 3  fields of basic types
//   Test::test3_0 -> Core::int
//   Test::test3_1 -> #a
//   Test::test3_2 -> Core::int
//   Test::test3_3 -> Test::B[#a]
pub type A {
	a : int,
	b : real
}
pub type B[#a] {
	a : #a
}
pub fn test3_0 (obj : A) { obj.a }
pub fn test3_1 (obj : B[#a]) { obj.a }
pub fn test3_2 (obj : B[int]) { obj.a }
pub fn test3_3 (obj : B[B[#a]]) { obj.a }

// test 4  methods of basic types
//   Test::A.x -> Core::real
//   Test::test4_0 -> fn() -> Core::real
//   Test::A.y -> Test::A
//   Test::test4_1 -> fn() -> Test::A
//   Test::B.t -> Core::int
//   Test::test4_2 -> fn() -> Core::int
//   Test::B.s -> #b
//   Test::test4_3 -> fn(#0) -> #0
//   Test::B.u -> #a
//   Test::test4_4 -> fn() -> Core::int
//   Test::B.v -> #b
//   Test::test4_5 -> fn(#0) -> #0
//   Test::test4_6  FAIL (uncomment)
impl A {
	fn x () { self.b }
	fn y () { self }
}
impl B[int] {
	fn t () { self.a }
	fn s (x : #b) { x }
}
impl B[#a] {
	fn u () { self.a }
	fn v (x : #b) { x }
}
pub fn test4_0 (obj : A) { obj.x }
pub fn test4_1 (obj : A) { obj.y }
pub fn test4_2 (obj : B[int]) { obj.t }
pub fn test4_3 (obj : B[int]) { obj.s }
pub fn test4_4 (obj : B[int]) { obj.u }
pub fn test4_5 (obj : B[int]) { obj.v }
//pub fn test4_6 (obj : B[string]) { obj.t }

// test 5  function calls
//   Test::test5_0 -> Core::int
//   Test::test5_1 -> Core::int
//   Test::test5_2 -> Core::int
//   Test::test5_3 -> Core::bool
//   Test::test5_4_0 -> fn(Core::int) -> Core::int
//   Test::test5_4_1 -> #a
//   Test::test5_4 -> fn(Core::int) -> Core::int
//   Test::test5_5 -> #0
pub fn test5_0 (x : real ) { test1_0() }
pub fn test5_1 () { test2_0(99) }
pub fn test5_2 () { test5_0(99) }
pub fn test5_3 () { test2_2(true, 88) }
pub fn test5_4_0 (f : fn(int) -> int) { f }
pub fn test5_4_1 (x : #a) { x }
pub fn test5_4 () { test5_4_0(test5_4_1) }
pub fn test5_5 (x : int) { test5_5(x) }

// test 6  conditionals
//   Test::test6_0 -> Core::int
//   Test::test6_1 -> fn(Core::int) -> Core::int
//   Test::test6_2  FAIL (uncomment)
//   Test::test6_3  FAIL (uncomment)
pub fn test6_0 (x : bool) { if x { 1 } else { 2 } }
pub fn test6_1 () { if true { test2_0 } else { test5_4_1 } }
//pub fn test6_2 (x : bool) { if x { 1 } else { 1.5 } }
//pub fn test6_3 (x : #a) { if x { false } else { true } }