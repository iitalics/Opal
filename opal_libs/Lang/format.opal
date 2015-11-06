module Lang



impl string { fn fmt (args : list[Lang::Show]) {
	match (self.find("{}"), args) {
		(-1, _) { self }
		(idx, x $ args') {
			self[,idx] +
			x.str() +
			self[idx + 2,].fmt(args')
		}
	}
}}

impl opt[#a(Lang::Show)] { fn str () {
	match self {
		Some(x) -> "Some(" + x.str() + ")"
		None() -> "None()"
	}
}}
impl list[#a(Lang::Show)] { fn str () {
	"[" + match self {
		[] -> ""
		x $ xs -> xs.foldl(x.str(), fn (z, y) { z + ", " + y.str() })
	} + "]"
}}


impl string {
	fn strip () {
		let i = 0;
		let j = self.len();
		while i < j and self[i].space?() {
			i = i.succ();
		}
		while j >= i and self[j.pred()].space?() {
			j = j.pred();
		}
		self[i,j]
	}
}