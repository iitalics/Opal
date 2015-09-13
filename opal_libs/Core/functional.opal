module Core

impl f : fn(#a) -> #b {
	// (f ^ g)(x)  =  f(g(x))
	fn exp (g : fn(#c) -> #a) {
		fn (x) { f(g(x)) }
	}

	// f <| x  =  f(x)
	fn lbind (x : #a) { f(x) }
}

impl f : fn(#a, #b) -> #c {
	// f <| x <| y  =  f(x, y)
	fn lbind (x : #a) { fn (y) { f(x, y) } }

	fn flip () {
		fn (x, y) { f(y, x) }
	}
}

impl f : fn(#a, #b, #c) -> #d {
	fn lbind (x : #a) { fn (y, z) { f(x, y, z) } }
}

impl f : fn(#a, #b, #c, #d) -> #e {
	fn lbind (x : #a) { fn (y, z, w) { f(x, y, z, w) } }
}

impl f? : fn(#a) -> bool {
	// (not f)(x)  =  not (f(x))
	fn inv () { fn (x) { not f?(x) } }
}


impl (int, int) {
	fn range (f : fn(int) -> unit) {
		let (a, b) = self
		while a < b {
			f(a)
			a = a.succ()
		}
	}
	fn range_incl (f : fn(int) -> unit) {
		let (a, b) = self
		while a <= b {
			f(a)
			a = a.succ()
		}
	}
}