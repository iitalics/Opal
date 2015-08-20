use Core

impl int {
	fn equal (x : int) { true }
	fn str () { "<int>" }
}

fn println (what : string) { }

fn main (argc : int)
{
	let y = 
		if argc < 2 {
			println("not enough arguments")
			true
		} else {
			let x = 4

			println(x.str())
			x = x + 1
			println(x.str())

			x != 5
		}
	let z = true

	println(
		if y and z { "y and z!" }
		else { ":<" })
}