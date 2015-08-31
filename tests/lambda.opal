use Core
use Lang

fn repeat (n : int, f : fn () -> unit) {
   if n > 0 {
      f()
      repeat(n - 1, f)
   }
}

impl array[int] {
   fn sum () {
      let sum = 0
      let i = 0

      repeat(self.len(), fn () {
         sum = sum + self[i]
         i = i.succ()
      })

      sum
   }
}

fn main () {
   let a = array()
   
   repeat(8, fn () {
      a.push(9)
   })

   a.sum()
}
