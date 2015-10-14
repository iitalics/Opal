module Examples

fn main () {
    // get a object for the console
    let console = IO::console();

    // create an array to store positions
    let stream = Lang::array();

    // sine wave speed
    let freq = 0.1;

    // prompt the user for length / height
    let len = console.prompt("Enter length: ").to_int();
    let height = console.prompt("Enter height: ").to_int();

    // generate the sine wave
    (0, len).range |i| {
        let x = i.to_real() * freq;
        let y = Math::sin(x);
        let row = (1.0 - y) / 2 * height.to_real();
        stream[i] = row.to_int();
    };

    // display each row
    (0, height).range |row| {
        // display each collumn
        (0, len).range |i| {
            if stream[i] == row {
                console.write("o");
            } else {
                console.write(" ");
            }
        };
        console.write("\n");
    };
}

pub fn demo () {
    "it works!"
}