use Core
use Lang

impl list[#a(Ord)] { fn sort () {
	match self {
		[] -> []
		x $ [] -> [x]
		p $ xs {
			let left = xs.filter |x| { x < p }
			let right = xs.filter |x| { x >= p }
			left.sort() + [p] + right.sort()
		}
	}
}}

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
		tree.push([])
	}


	// add leaves to the tree
	items.each |x| {
		let stem = x / 10
		let leaf = x % 10
		let i = stem - min_stem

		tree[i] = leaf $ tree[i]
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
		[63, 67, 69, 82, 91, 98, 104, 104, 105, 107, 118,
	     120, 133, 134, 136, 138, 141, 143, 144, 147, 167,
	     181, 190, 221]
}



