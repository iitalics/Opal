use Core

impl x : real {
   fn add (y : real)   extern("opal.num") "real.add" -> real
   fn sub (y : real)   extern("opal.num") "real.sub" -> real
   fn mul (y : real)   extern("opal.num") "real.mul" -> real
   fn div (y : real)   extern("opal.num") "real.div" -> real
   fn mod (y : real)   extern("opal.num") "real.mod" -> real
   fn succ ()          extern("opal.num") "real.succ" -> real
   fn pred ()          extern("opal.num") "real.pred" -> real
   fn cmp (y : real)   extern("opal.num") "real.cmp" -> int
   fn equal (y : real) extern("opal.num") "real.equal" -> bool

   fn to_int () extern("opal.num") "real.to_int" -> int
   fn to_real () { x }
   fn to_long () extern("opal.num") "real.to_long" -> long
}

fn iterate (n : real, z : #a, f : fn(real, #a) -> #a) {
   if n == 0 {
      z
   } else {
      f(n, iterate(n - 1, z, f))
   }
}

type e_approx {
   sum : real,
   denom : real
}

fn main () {
   let approx = new e_approx { sum = 0, denom = 1 }

   // e = sum[n=0 -> infty] 1 / n!
   //   = 1 + 1 + 1/2 + 1/6 + 1/24 + ...
   approx = iterate(100, approx,
      fn (n, approx) {
         new e_approx {
            sum = approx.sum + 1.0 / approx.denom,
            denom = approx.denom * n
         }
      })

   let E = approx.sum
   return E
}