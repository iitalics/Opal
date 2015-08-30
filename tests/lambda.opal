use Core

fn repeat (n : int, f : fn () -> unit) {
   if n > 0 {
      f()
      repeat(n - 1, f)
   }
}

fn main () {
   let E = 0.0
   let denom = 1.0
   let n = 0.0

   repeat(1000, fn () {
      E = E + 1.0 / denom
      n = n + 1
      denom = denom * n
   })

   E
}
