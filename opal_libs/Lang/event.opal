module Lang

pub type event[#c] { callbacks : array[fn(#c) -> unit] }
impl event[#c] {
	fn bind (f : fn(#c) -> unit) {
		self.(callbacks).push(f)
	}
	fn fire (a : #c) {
		self.(callbacks).each(fn (f) {
			f(a)
		})
	}

	// operator versions of the above
	fn lbind (f : fn(#c) -> unit) { self.bind(f) }
	fn lshift (a : #c) { self.fire(a) ; self }
}
pub fn event () { new event[#_] { callbacks = array() } }
