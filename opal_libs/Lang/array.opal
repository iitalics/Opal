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
}
pub fn array () extern("opal.list") "array" -> array[#e]
