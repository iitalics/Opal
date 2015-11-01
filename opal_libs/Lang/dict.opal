module Lang

pub type dict[#k, #v] {
	keys : array[#k],
	vals : array[#v]
}

impl dict[#k(Eq), #v] {
	fn index (key : #k) {
		let i = 0;
		let len = self.(keys).len();
		while i < len {
			if self.(keys)[i] == key {
				return i
			} else {
				i = i.succ()
			}
		};
		-1
	}
	fn find (key : #k) {
		match self.index(key) {
			-1 { None() }
			idx { Some(self.(vals)[idx]) }
		}
	}
	fn get_or (key : #k, val : #v) {
		match self.index(key) {
			-1 { val }
			idx { self.(vals)[idx] }
		}
	}
	fn get (key : #k) {
		self.(vals)[self.index(key)]
	}
	fn set (key : #k, val : #v) {
		let idx = self.index(key);
		if idx == -1 {
			self.(keys).push(key);
			self.(vals).push(val);
		} else {
			self.(vals)[idx] = val;
		}
	}
}

pub fn dict () {
	new dict[#k, #v] { keys = array(), vals = array() }
}
impl list[(#k, #v)] {
	fn to_dict () {
		let res = dict();
		self.each |pair| {
			let (k, v) = pair;
			res.(keys).push(k);
			res.(vals).push(v);
		};
		res
	}
}