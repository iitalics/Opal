module GameOfLife
use Lang

pub type GOL {
  w: int,
  h: int,
  rule: fn(int,int) -> int,
  arr: array[int]
}

impl GOL {
  fn get (p : (int,int)) {
    let (x,y) = p;
    if x >= 0 and y >= 0
        and x < self.(w) and y < self.(h) {
      self.(arr)[y * self.(w) + x]
    }
    else {
      0
    }
  }

  fn set (p : (int,int), v : int) {
    let (x,y) = p;
    if x >= 0 and y >= 0
        and x < self.(w) and y < self.(h) {
      self.(arr)[y * self.(w) + x] = v;
    }
  }

  fn neighbors (x, y) {
    [ (x-1, y-1), (x, y-1), (x+1, y-1),
      (x-1, y),             (x+1, y),
      (x-1, y+1), (x, y+1), (x+1, y+1) ]
    .map(self.get)
    .foldl(0, .add)
  }

  fn next_life () {
    let next = new GOL {
      w = self.(w),
      h = self.(h),
      rule = self.(rule),
      arr = array_of(0, self.(w) * self.(h))
    };

    (0, self.(h)).range |y| {
      (0, self.(w)).range |x| {
        next[(x,y)] = self.next_cell(x, y);
      }
    }

    next
  }

  fn next_cell (x, y) {
    let n = self.neighbors(x, y);
    self.(rule)(self[(x,y)], n)
  }

  fn str () {
    let s = "";
    (0, self.(h)).range |y| {
      (0, self.(w)).range |x| {
        if x == 0 {
          s = s + "\n >";
        }
        if self[(x,y)] > 0 {
          s = s + " #";
        }
        else {
          s = s + " .";
        }
      }
    }
    s + "\n"
  }
}

pub fn conway (cell, n) {
  match n {
    2 { cell }
    3 { 1 }
    _ { 0 }
  }
}

pub fn gol_example () {
  new GOL {
    w = 8, h = 6,
    rule = conway,
    arr =
    [ 0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,1,1,1,0,0,
      0,0,1,1,1,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0 ].to_array()
  }
}

pub fn main () {
  let gol = gol_example();
  let c = IO::console();
  let i = 1;
  let quit = false;
  while not quit {
    c.write("Iteration: {} {}\n".fmt([i, gol]))
    if c.read_line().find("q") == 0 {
      quit = true;
    }
    gol = gol.next_life();
    i = i + 1;
  }
}