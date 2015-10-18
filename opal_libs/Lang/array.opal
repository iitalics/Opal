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
	fn empty? () { self.len() == 0 }

	fn foldl (z : #e', f : fn(#e', #e) -> #e') {
		let i = 0;
		while i < self.len() {
			z = f(z, self[i]);
			i = i.succ();
		}
		z
	}
	fn foldr (z : #e', f : fn(#e, #e') -> #e') {
		let i = self.len();
		while i > 0 {
			i = i.pred();
			z = f(self[i], z);
		}
		z
	}
	fn each (f : fn(#e) -> unit) {
		let i = 0;
		while i < self.len() {
			f(self[i]);
			i = i.succ();
		}
	}
	fn eachi (f : fn(int, #e) -> unit) {
		let i = 0;
		while i < self.len() {
			f(i, self[i]);
			i = i.succ();
		}
	}
	fn copy () {
		let res = array();
		res.cap_set(self.cap());
		self.each(res.push);
		res
	}
	fn fill (elem : #e, n : int) {
		self.cap_set(self.cap() + n)
		while n > 0 {
			self.push(elem)
			n = n.pred()
		}
	}
	fn slice (a : int, b : int) {
		let res = array()
		if a < b {
			res.cap_set(b - a);
			let i = a;
			while i < b {
				res.push(self[i]);
				i = i.succ();
			}
		}
		; res
	}
	fn slice_from (a : int) { self.slice(a, self.len()) }
	fn slice_to (b : int) { self.slice(0, b) }

	fn to_list () {
		let res = [];
		let i = self.len();
		while i > 0 {
			i = i.pred();
			res = self[i] $ res;
		}
		; res
	}
	fn to_array () { self }
}
impl array[#e(Show)] { fn str () {
	let res = "["
	let i = 0;
	while i < self.len() {
		if i > 0 {
			res = res + ", ";
		}
		res = res + self[i].str();
		i = i.succ();
	}
	res + "]"
}}
pub fn array () extern("opal.list") "array" -> array[#e]
pub fn array_of (elem : #e, n : int) {
	let a = array();
	a.fill(elem, n);
	a
}

impl list[#e] {
	fn to_array () {
		let res = array();
		self.each(res.push);
		res
	}
}
