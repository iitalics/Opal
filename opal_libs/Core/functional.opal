module Core

impl f : fn() -> #a {
	fn mod (g : fn(#a) -> #b) {
		fn () { g(f()) }
	}
}

impl f : fn(#a) -> #b {
	// (f ^ g)(x)  =  f(g(x))
	fn exp (g : fn(#c) -> #a) {
		fn (x) { f(g(x)) }
	}

	// (f % g)(x)  =  g(f(x))
	fn mod (g : fn(#b) -> #c) {
		fn (x) { g(f(x)) }
	}

	// f <| x  =  f |> x  =  f(x)
	fn lbind (x : #a) { f(x) }
	fn rbind (x : #a) { f(x) }
}

impl f : fn(#a, #b) -> #c {
	// f <| x <| y  =  f(x, y)
	fn lbind (x : #a) { fn (y) { f(x, y) } }

	// f |> x |> y  =  f(y, x)
	fn rbind (y : #b) { fn (x) { f(x, y) } }

	// (f % g)(x, y)  =  g(f(x, y))
	fn mod (g : fn(#c) -> #d) {
		fn (x, y) { g(f(x, y)) }
	}

	fn flip () {
		fn (x, y) { f(y, x) }
	}
}

impl f : fn(#a, #b, #c) -> #d {
	fn lbind (x : #a) { fn (y, z) { f(x, y, z) } }
	fn rbind (z : #c) { fn (x, y) { f(x, y, z) } }

	fn mod (g : fn(#d) -> #e) {
		fn (x, y, z) { g(f(x, y, z)) }
	}
}

impl f : fn(#a, #b, #c, #d) -> #e {
	fn lbind (x : #a) { fn (y, z, w) { f(x, y, z, w) } }
	fn rbind (w : #d) { fn (x, y, z) { f(x, y, z, w) } }
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