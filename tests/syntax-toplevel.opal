module List
use Util

iface Show {
	fn str () : string
}
iface Ord #t {
	fn cmp (other : #t) : int
}

fn foo (x : int) {
	(1 + 2) * 3 
	1 + 2 * 3
	1 + 4 == 5 * 1 and 0 == 5 - x
}