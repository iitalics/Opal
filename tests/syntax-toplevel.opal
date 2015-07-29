module Geom
use Core

type vec2 {
	x : float,
	y : float
}

fn vec2 (x : float, y : float) {
	new vec2 {x = x, y = y}
}

impl a : vec2 {
	fn add (b : vec2) {
		new vec2 {
			x = a.x + b.x,
			y = a.y + b.y
		}
	}
}