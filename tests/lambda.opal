use Core
use Lang

fn repeat (n : int, f : fn() -> unit) {
   if n > 0 {
      f()
      repeat(n - 1, f)
   }
}
impl array[#e] {
   fn each (f : fn(#e) -> unit) {
      let i = 0
      repeat(self.len(), fn () {
         f(self[i])
         i = i.succ()
      })
   }
}


type event[#a] {
   callbacks : array[fn(#a) -> unit]
}
impl event[#a] {
   fn bind (f : fn(#a) -> unit) {
      self.callbacks.push(f)
   }
   fn fire (arg : #a) {
      self.callbacks.each(fn (cb) {
         cb(arg)
      })
   }
}
fn event () { new event[#a] { callbacks = array() } }

fn main () {
   let ev = event()

   let x = 0

   ev.bind(fn (_) {
      x = x + 1
   })
   ev.bind(fn (k) {
      x = x + k
   })

   ev.fire(3) // x = 0 + 1 + 3 = 4
   ev.fire(4) // x = 4 + 1 + 4 = 9

   { x }
}
