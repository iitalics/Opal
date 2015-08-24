use Core

pub type rect {
	width : int,
	height : int
}
pub type pt {
	x : int, y : int
}

impl rect {
	fn perim () {
		self.width + self.width +
		self.height + self.height
	}
	fn draw (pt : pt) { ; }
}
impl (pt, rect) {
	fn draw () {
		let p = self.a
		let r = self.b
		r.draw(p)
	}
}

fn main () {
	let shape =
		(new pt { x = 0, y = 0 },
		 new rect { width = 100, height = 120 })

	shape.draw()
	shape.b.draw(new pt { x = 10, y = 10 })
}