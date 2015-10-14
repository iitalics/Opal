module Lang

pub type opt[#a] = Some(#a) or None()
pub type err[#a] = Ok(#a) or Error(string)
pub type box[#a] { val : #a }


// (<|) : M[a] -> (a -> b) -> M[b]
// (<<) : M[a] -> b -> M[b]
// (>>) : M[a] -> (a -> M[b]) -> M[b]

pub fn box (v : #a) { new box[#a] { val = v } }
pub fn box_copy (v : #a(Lang::Copy)) { new box[#a] { val = v.copy() } }
impl box[#a] {
	fn get () { self.val }
	fn lbind (f : fn(#a) -> #a) {
		self.val = f(self.val)
		; self
	}
	fn lshift (val : #a) {
		self.val = val
		; self
	}
}
impl box[#a(Lang::Show)] { fn str () { self.val.str() }}
impl box[#a(Lang::Step)] {
	fn incr () { self.val = self.val.succ() }
	fn decr () { self.val = self.val.pred() }
}

impl opt[#a] {
	fn get () { // DO NOT USE
		match self {
			Some(x) -> x
		}
	}
	fn default (y : #a) {
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
	fn lbind (f : fn(#a) -> #b) {
		match self {
			Some(x) -> Some(f(x))
			None() -> None()
		}
	}
	fn lshift (y : #b) {
		match self {
			Some(x) -> Some(y)
			None() -> None()
		}
	}
	fn rshift (f : fn(#a) -> opt[#b]) {
		match self {
			Some(x) -> f(x)
			None() -> None()
		}
	}
}
impl opt[#a(Lang::Show)] { fn str () {
	match self {
		Some(x) -> "Some(" + x.str() + ")"
		None() -> "None()"
	}
}}

impl err[#a] {
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
	fn rshift (f : fn(#a) -> err[#b]) {
		match self {
			Ok(x) -> f(x)
			Error(s) -> Error(s)
		}
	}
}
