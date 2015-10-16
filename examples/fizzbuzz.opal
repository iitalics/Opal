module Examples

pub fn fizzbuzz (n : int) {
	match (n % 3, n % 5) {
		(0, 0) { "FizzBuzz" }
		(0, _) { "Fizz" }
		(_, 0) { "Buzz" }
		_      { n.str() }
	}
}

pub fn main () {
	(1, 100).range_incl |i| {
		IO::print(fizzbuzz(i) + "\n");
	}
}