module RPN
use Lang

pub type rpn_cmd
	=  CmdConst(int)
	or CmdDrop()
	or CmdDup()
	or CmdSwap()
	or CmdFn1(fn(int) -> int)
	or CmdFn2(fn(int, int) -> int)

impl array[int] { fn exec (cmd : rpn_cmd) {
	match cmd {
		CmdConst(n) {
			self.push(n)
		}
		CmdDrop() {
			self.pop()
		}
		CmdDup() {
			self.push(self[self.len() - 1])
		}
		CmdSwap() {
			let i = self.len() - 1;
			let j = self.len() - 2;
			let a = self[i];
			let b = self[j];
			self[i] = b;
			self[j] = a;
		}
		CmdFn1(f) {
			let a = self[self.len() - 1];
			self.pop();
			self.push(f(a));
		}
		CmdFn2(f) {
			let a = self[self.len() - 2];
			let b = self[self.len() - 1];
			self.pop();
			self.pop();
			self.push(f(a, b));
		}
	}
}}

impl string {
	fn triml () {
		let i = 0;
		while i < self.len() and self[i].space?() {
			i = i + 1;
		}
		self[i,]
	}
}


pub fn parseAll (str : string) {
	str = str.triml();
	if str.empty?() {
		[]
	} else {
		let (cmd, str') = parse(str);
		cmd $ parseAll(str')
	}
}
pub fn parse (str : string) {
	if str[0].digit?() {
		let (num, str') = parseNumber(str);
		(CmdConst(num), str')
	} else {
		let (id, str') = parseCmdName(str);
		(cmdByName(id), str')
	}
}
fn parseNumber (str : string) {
	let i = 0;
	while i < str.len() and str[i].digit?() {
		i = i + 1;
	}
	(str[0, i].to_int(), str[i,])
}
fn parseCmdName (str : string) {
	let i = 0;
	while i < str.len() and not str[i].space?() {
		i = i + 1;
	}
	(str[0, i], str[i,])
}
fn cmdByName (name : string) {
	match name {
		"drop" -> CmdDrop()
		"dup" -> CmdDup()
		"swap" -> CmdSwap()
		"neg" -> CmdFn1(.neg)
		"+" -> CmdFn2(.add)
		"-" -> CmdFn2(.sub)
		"*" -> CmdFn2(.mul)
		"/" -> CmdFn2(.div)
	}
}

pub fn main () {
	let console = IO::console();
	let stack = array();
	let going = true;

	while going {
		console.write("\n");
		if stack.empty?() {
			console.write("(none)");
		} else {
			stack.each |x| {
				console.write(x.str() + " ");
			}
		}
		console.write("\n");

		let input = console.prompt("# ");
		if input.empty?() {
			going = false;
		} else {
			parseAll(input).each(stack.exec);
		}
	}
	console.write("\n")
}
