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
iface #t : Add { fn add (#t) -> #t }
iface Show { fn str () -> string }


fn show (x : #t(Show)) { x.str() }
fn max (x : #t(Ord), y : #t) {
	if x > y { x } else { y }
}
fn println (x : #t(Show)) {
	()
}

fn sum (n : #t(Enum, Add)) {
	if n.zero?() {
		n
	} else {
		n + sum(n.pred())
	}
}



fn main () {
	println(sum(50))
}