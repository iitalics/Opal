use Core

impl int {
	fn equal (x : int) { true }
	fn str () { }
}

fn println (what : int) { }

fn main (argc : int)
{
	let y = 
		if argc < 2 {
			println(0)
			true
		} else {
			let x = 4

			println(x)
			x = x + 1
			println(x)

			x != 5
		}
	let z = true

	println(
		if y and z { 3 }
		else { 4 })
}