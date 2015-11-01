module Lang

pub type opt[#a] = Some(#a) or None()
pub type result[#a] = Ok(#a) or Error(string)
pub type box[#a] { val : #a }


// (<|) : M[a] -> (a -> b) -> M[b]
// (<<) : M[a] -> b -> M[b]
// (>>) : M[a] -> (a -> M[b]) -> M[b]

pub fn box (v : #a) { new box[#a] { val = v } }
pub fn box_copy (v : #a(Lang::Copy)) { new box[#a] { val = v.copy() } }
impl box[#a] {
	fn get () { self.(val) }
}
impl box[#a(Lang::Show)] { fn str () { self.(val).str() }}
impl box[#a(Lang::Step)] {
	fn incr () { self.(val) = self.(val).succ() }
	fn decr () { self.(val) = self.(val).pred() }
}

impl opt[#a] {
	fn get () { // DO NOT USE
		match self {
			Some(x) -> x
		}
	}
	fn get_or (y : #a) {
		match self {
			Some(x) -> x
			None() -> y
		}
	}
	fn any? () {
		match self {
			Some(_) -> true
			None() -> false
		}
	}
	fn to_result (what : string) {
		match self {
			Some(x) -> Ok(x)
			None() -> Error(what)
		}
	}
	fn apply (f : fn(#a) -> #b) {
		match self {
			Some(x) -> Some(f(x))
			None() -> None()
		}
	}
	fn each (f : fn(#a) -> unit) {
		match self {
			Some(x) -> f(x)
			None() -> ()
		}
	}

	fn lbind (f : fn(#a) -> #b) { self.apply(f) }
	fn rbind (f : fn(#a) -> unit) { self.each(f) }
}

impl result[#a] {
	fn get () {
		match self {
			Ok(x) -> x
			// Error(str) -> Core::throw(str) ??
		}
	}
	fn lbind (f : fn(#a) -> #b) {
		match self {
			Ok(x) -> Ok(f(x))
			Error(s) -> Error(s)
		}
	}
	fn lshift (y : #b) {
		match self {
			Ok(x) -> Ok(y)
			Error(s) -> Error(s)
		}
	}
	fn rshift (f : fn(#a) -> result[#b]) {
		match self {
			Ok(x) -> f(x)
			Error(s) -> Error(s)
		}
	}
}
