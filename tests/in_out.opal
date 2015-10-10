use Core
use Lang

impl array[#a] { fn swap (i : int, j : int) {
	let t = self[i]
	self[i] = self[j]
	self[j] = t
}}
impl array[#a(Ord)] {
	// in-place quicksort
	fn sort_some (a : int, b : int) {
		if b - a <= 1 {
			return
		}

		let p = a
		let i = a + 1
		while i < b {
			if self[i] < self[p] {
				if i > p + 1 {
					self.swap(i, p + 1)
				}
				self.swap(p, p + 1)
				p = p + 1
			}

			i = i + 1
		}

		self.sort_some(a, p)
		self.sort_some(p + 1, b)
	}
	fn sort () { self.sort_some(0, self.len()); self }
}

impl string { fn pre_pad (n : int) {
	if n > self.len() {
		(" " + self).pre_pad(n)
	} else {
		self
	}
}}

fn stem_leaf (items : list[int]) {
	if items.nil?() { return }
	let min_stem = 9999
	let max_stem = -9999

	// find the minimum and maximum stems
	items.each |x| {
		let stem = x / 10

		if stem < min_stem {
			min_stem = stem
		} else if stem > max_stem {
			max_stem = stem
		}
	}

	// create the tree and "branches"
	let tree = array()
	; (min_stem, max_stem).range_incl |stem| {
		tree.push(array())
	}

	// add leaves to the tree
	items.each |x| {
		let stem = x / 10
		let leaf = x % 10
		let i = stem - min_stem

		tree[i].push(leaf)
	}

	// display the tree
	IO::print("STEM | LEAF\n")
	tree.eachi |i, leafs| {
		let stem = i + min_stem

		IO::print("{} |" % [stem.str().pre_pad(4)])

		leafs.sort().each |leaf| {
			IO::print(" {}" % [leaf])
		}

		IO::print("\n")
	}
}

fn main () {
	stem_leaf <|
		[63, 67, 69, 82, 91, 98, 104, 105, 104, 107, 118,
		 120, 133, 134, 136, 138, 141, 143, 144, 147, 167,
		 181, 190, 221]
}



