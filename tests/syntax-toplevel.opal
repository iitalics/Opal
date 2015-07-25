type someObj {
	x : int,
	y : float,
	z : string
}

impl someObject {
	fn transform (f : func[int, float, string, int]) {
		self.x = f(self.x, self.y, self.z)
	}
}

fn main () {
	not not true;
	( 1, 2, 3 );
	- 4 + 5 * 3 - - 1;

	println("Hello, world!")
	foo(1, 2, 3)

	let list = [1, 2, 3, 4]
	let array : array[int]
	array[0] = list.head

	let obj = new someObj {
		x = 4,
		y = 4.5,
		z = "Test"
	}

	obj.transform(fn (x, y, z) {
		x + 1
	})
}