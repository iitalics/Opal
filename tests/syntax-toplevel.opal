module ABC
use Core

// private/public types
type A { x : int, y : int }
pub type B { z? : bool }
pub type C { my_b : B }

// types with parameters
type foo[#a] { x : #a }

// ifaces
iface Foo { fn thing (A) -> B }
pub iface ToB { fn to_B () -> B }

// impl (named or automatic self-variable)
impl B { fn to_B () { self } }
impl a : A {
   fn to_A () { a }
   fn to_B () { new B { z? = x < y } }
}
impl C { fn to_B () { self.my_b } }

// specialized impl
impl foo[#t(ToB)] {
   fn to_B () { self.x.to_B() }
}
impl foo[A] {
   fn to_A () { self.x }
   fn thing (a : A) {
      B()
   }
}

// globals
fn A () { new A { x = 0, y = 0 } }
fn B () { new B { z? = false } }
fn C (x : #t(ToB)) {
   new C {
      my_b = x.to_B()
   }
}

// special types: tuple / function
fn foo (x : (A, B)) {}
fn bar (x : fn(A, B) -> C) {}

