module Lang



impl fmt : string { fn mod (args : list[Lang::Show]) {
	match (fmt.find("{}"), args) {
		(-1, _) { fmt }
		(idx, x $ args') {
			fmt[,idx] +
			x.str() +
			fmt[idx + 2,] % args'
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
