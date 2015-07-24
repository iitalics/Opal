module List
use Util
module Foo

iface Show {
	fn str () : string
}
iface Ord #t {
	fn cmp (other : #t) : int
}

impl a : list[#e] {
	fn foldl (z : #e', f : func[#e', #e, #e']) // ...
}
impl list[#e(Show)] {
	fn str () // ...
}

fn max (a : #t(Ord), b : #t(Ord)) // ...