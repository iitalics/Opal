module Lang
use Core

impl x : int {
	fn cmp (y : int) { 0 }
	fn equal (y : int) { true }
}


fn main () {
	1 < 3
	1 == 4
	
	1 == { break } // weird.
}