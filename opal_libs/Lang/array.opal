use Core
module Lang

pub type array[#e] extern true

impl array[#e] {
	fn len ()                extern("opal.list") "array.len" -> int
	fn cap ()                extern("opal.list") "array.cap" -> int
	fn cap_set (n : int)     extern("opal.list") "array.cap_set" -> unit
	fn get (i : int)         extern("opal.list") "array.get" -> #e
	fn set (i : int, t : #e) extern("opal.list") "array.set" -> unit

	fn insert (i : int, t : #e) extern("opal.list") "array.insert" -> unit
	fn remove (i : int)         extern("opal.list") "array.remove" -> unit

	fn push (t : #e) {
		self[self.len()] = t
	}
	fn pop () {
		self.remove(self.len().pred())
	}

	fn foldl (z : #e', f : fn(#e', #e) -> #e') {
		let i = 0
		while i < self.len() {
			z = f(z, self[i])
			i = i.succ()
		}
		; z
	}
	fn foldr (z : #e', f : fn(#e, #e') -> #e') {
		let i = self.len()
		while i > 0 {
			i = i.pred()
			z = f(self[i], z)
		}
		; z
	}
	fn each (f : fn(#e) -> unit) {
		let i = 0
		while i < self.len() {
			f(self[i])
			i = i.succ()
		}
	}
	fn eachi (f : fn(int, #e) -> unit) {
		let i = 0
		while i < self.len() {
			f(i, self[i])
			i = i.succ()
		}
	}
	fn copy () {
		let res = array()
		res.cap_set(self.cap())
		self.each(res.push)
		; res
	}
	fn fill (x : #e, n : int) {
		self.cap_set(self.cap() + n)
		while n > 0 {
			self.push(x)
			n = n.pred()
		}
	}

	fn to_list () {
		let res = []
		let i = self.len()
		while i > 0 {
			i = i.pred()
			res = self[i] $ res
		}
		; res
	}
	fn to_array () { self }
}
pub fn array () extern("opal.list") "array" -> array[#e]

impl list[#e] {
	fn to_array () {
		let res = array()
		self.each(res.push)
		; res
	}
}
