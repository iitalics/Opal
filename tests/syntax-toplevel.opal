module Core

pub type list[#e] {}
pub type int {}
pub type string {}

pub iface #t : Ord { fn cmp (#t) : int }
pub iface #t : Show { fn str () : string }

pub fn max (a : #t(Ord), a : #t) {
	if a > b { a } else { b }
}

impl string { fn str () { self } }
impl int { fn str () { "<int>" } }



