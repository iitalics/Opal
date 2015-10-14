module Lang

pub iface Show { fn str () -> string }
pub iface ToInt { fn to_int () -> int }
pub iface ToReal { fn to_real () -> real }
pub iface ToLong { fn to_long () -> long }
pub iface ToBool { fn to_bool () -> bool }
pub iface #t : Ord {
	fn cmp (#t) -> int
}
pub iface #t : Eq {
	fn equal (#t) -> bool
}
pub iface #t : Num {
	fn add (#t) -> #t
	fn sub (#t) -> #t
	fn mul (#t) -> #t
	fn div (#t) -> #t
	fn neg () -> #t
}
pub iface #t : Inv {
	fn inv () -> #t
}
pub iface #t : Step {
	fn succ () -> #t
	fn pred () -> #t
}
pub iface Each[#e] {
	fn each (fn(#e) -> unit) -> unit
}
pub iface #t : Copy {
	fn copy () -> #t
}
pub iface #t : Add {
	fn add (#t) -> #t
}