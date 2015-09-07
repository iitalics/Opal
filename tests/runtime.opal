use Core



fn sum (n : int) {
	if n > 0 {
		n + sum(n - 1)
	} else {
		0
	}
}

type counter {
	n : int,
	sum : int
}
impl counter {
	fn next () {
		let prev = self.n
		self.n = prev + 1
		self.sum = self.sum + prev
		prev
	}
	fn repeat (n : int) {
		if n > 0 {
			self.next()
			self.repeat(n - 1)
		}
	}
}
fn counter (start : int) {
	new counter { n = start, sum = 0 }
}

fn main () {
	let N = 5

	// use recursive algorithm
	let a = sum(N)

	// use counter object
	let cn = counter(1)
	cn.repeat(N)
	let b = cn.sum

	// check equality
	if a > b or a < b {
		"invalid!"
	} else {
		"correct answer!"
	}
}