module Core

pub type list[#e] = Cons(#e, list[#e]) or Nil()

// list comprehension
impl list[#e] {
	fn len () {
		match self {
			[] -> 0
			x $ xs -> 1 + xs.len()
		}
	}
	fn get (i : int) {
		let x $ xs = self // beware
		if i == 0 {
			x
		} else {
			xs.get(i.pred())
		}
	}

	// folding
	fn foldr (z : #e', f : fn(#e, #e') -> #e') {
		match self {
			[] -> z
			x $ xs -> f(x, xs.foldr(z, f))
		}
	}
	fn foldr1 (f : fn(#e, #e) -> #e) {
		let x $ xs = self
		xs.foldr(x, f)
	}
	fn foldl (z : #e', f : fn(#e', #e) -> #e') {
		match self {
			[] -> z
			x $ xs -> xs.foldl(f(z, x), f)
		}
	}
	fn foldl1 (f : fn(#e, #e) -> #e) {
		let x $ xs = self
		xs.foldl(x, f)
	}

	// special cases
	fn each (f : fn(#e) -> unit) {
		match self {
			[] -> ()
			x $ xs { f(x); xs.each(f) }
		}
	}
	fn map (f : fn(#e) -> #e') {
		match self {
			[] -> []
			x $ xs -> f(x) $ xs.map(f)
		}
	}
	fn filter (f : fn(#e) -> bool) {
		match self {
			[] -> []
			x $ xs ->
				if f(x) { x $ xs.filter(f) }
				else { xs.filter(f) }
		}
	}
	fn any? (f : fn(#e) -> bool) {
		match self {
			[] -> false
			x $ xs ->
				if f(x) { true }
				else { xs.any?(f) }
		}
	}
	fn all? (f : fn(#e) -> bool) {
		match self {
			[] -> true
			x $ xs ->
				if f(x) { xs.any?(f) }
				else { false }
		}
	}
	fn add (other : list[#e]) {
		match self {
			[] -> other
			x $ xs -> x $ (xs + other)
		}
	}
}
