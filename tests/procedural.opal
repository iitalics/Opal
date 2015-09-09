use Core
module Proc


fn main () {
	let sum = 0

	let i = 1
	while i <= 10 {
		sum = sum + i
		i = i.succ()
	}

	sum
}