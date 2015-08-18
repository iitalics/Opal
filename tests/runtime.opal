use Core

fn print (x : int) {}

fn main (thing? : bool, thing : int, inc : int)
{
	if true {
		let x = if thing? { thing } else { 0 }

		x = x + inc
		print(x)
	}
}