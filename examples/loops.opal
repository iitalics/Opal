module Examples
use Lang
use Math

fn main () {
    let data = array();
    let count = IO::console().prompt("Enter count: ").to_int();

    // generate random numbers
    (0, count).range |i| {
        let num = (1, 200).rand_incl();
        data.push(num);
        IO::print("{} -> {}\n".fmt([i, num]));
    };

    // find the largest value
    let max = 0;
    data.each |num| {
        if num > max {
            max = num;
        }
    };

    // remove everything less than the threshold (half the largest)
    let threshold = max / 2;
    let i = 0;
    let removed = 0;
    while i < data.len() {
        if data[i] < threshold {
            data.remove(i);
            removed = removed + 1;
        } else {
            i = i + 1;
        }
    }

    IO::print("----------\n");
    IO::print("threshold: {}\n".fmt([threshold]));
    IO::print("removed {} items\n".fmt([removed]));
    IO::print("----------\n");

    // display the new list
    data.eachi |i, num| {
        IO::print("{} -> {}\n".fmt([i, num]));
    }
}