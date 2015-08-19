module Lang
use Core



impl x : int {
	fn cmp (y : int) { 0 }
	fn equal (y : int) { true }
	fn add (y : int) { x }
	fn sub (y : int) { x }

	fn succ () { x + 1 }
	fn pred () { x - 1 }
	fn zero? () { x == 0 }

	fn str () { "" }
}
impl string { fn str () { self } }


iface #t : Ord {
	fn cmp (#t) -> int
}
iface #t : Eq {
	fn equal (#t) -> bool
}
iface #t : Enum {
	fn succ () -> #t
	fn pred () -> #t
	fn zero? () -> bool
}
iface #t : Num {
	fn add (#t) -> #t
	fn sub (#t) -> #t
	fn mul (#t) -> #t
	fn div (#t) -> #t
	fn neg (#t) -> #t
}
iface Show { fn str () -> string }


fn max (x : #t(Ord), y : #t) {
	if x > y { x } else { y }
}
fn println (x : #t(Show)) { }


pub type vec2 {
	x : int,
	y : int
}
impl a : vec2 {
	fn add (b : vec2) {
		new vec2 {
			x = a.x + b.x,
			y = a.y + b.y
		}
	}
}
fn vec2 (x : int, y : int) {
	new vec2 { x = x, y = y }
}

fn main () {
	let x = 0
	let y = true
	let z = vec2(0, 0)

	z.x = 4

	if z.y > z.x {
		return false
	}
	
	println(max(x, z.x))
	true
}