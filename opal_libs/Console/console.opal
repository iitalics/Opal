module Console



pub fn parseColorName (str) {
	match str[0,3] {
		"bla"  -> 0
		"red"  -> 1
		"gre"  -> 2
		"yel"  -> 3
		"blu"  -> 4
		"pin"  -> 5
		"cya"  -> 6
		"whi"  -> 7
	}
}

pub fn parseColor (str : string) {
	let bold = 
		(str[0,4] == "bold") and {
			str = str[4,].strip();
			true
		};

	let FG = 30;
	let BG = 40;

	let idx = str.find("/");

	if idx == 0 {
		let bg = parseColorName(str[1,]) + BG;
		"\x1b[" + bg.str() + "m"
	} else if idx < 0 {
		let fg = parseColorName(str) + FG;

		if bold {
			"\x1b[1;" + fg.str() + "m"
		} else {
			"\x1b[" + fg.str() + "m"
		}
	} else {
		let fg = parseColorName(str[,idx]) + FG;
		let bg = parseColorName(str[idx + 1,]) + BG;

		if bold {
			"\x1b[1;" + fg.str() + ";" + bg.str() + "m"
		} else {
			"\x1b[" + fg.str() + ";" + bg.str() + "m"
		}
	}
}

pub fn draw_ (buf : string, fmt : string, args : list[(Lang::Show, string)]) {
	match (fmt.find("{}"), args) {
		(-1, _) { buf + fmt }
		(idx, (text, color) $ args') {
			let buf' =
				buf + fmt[,idx] +
				parseColor(color) + text.str() + "\x1b[0m";

			draw_(buf', fmt[idx + 2,], args')
		}
	}
}

pub fn print (what : #a(Lang::Show)) {
	IO::console().write(what.str())
}
pub fn println (what : #a(Lang::Show)) {
	IO::console().write(what.str() + "\n")
}
pub fn draw (fmt, args) {
	IO::console().write(draw_("", fmt, args))
}
pub fn drawln (fmt, args) {
	IO::console().write(draw_("", fmt + "\n", args))
}

