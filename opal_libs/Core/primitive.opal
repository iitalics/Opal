module Core

impl x : int {
   fn add (y : int)   extern("opal.prim") "int.add" -> int
   fn sub (y : int)   extern("opal.prim") "int.sub" -> int
   fn mul (y : int)   extern("opal.prim") "int.mul" -> int
   fn div (y : int)   extern("opal.prim") "int.div" -> int
   fn mod (y : int)   extern("opal.prim") "int.mod" -> int
   fn succ ()         extern("opal.prim") "int.succ" -> int
   fn pred ()         extern("opal.prim") "int.pred" -> int
   fn neg ()          extern("opal.prim") "int.neg" -> int
   fn cmp (y : int)   extern("opal.prim") "int.cmp" -> int
   fn equal (y : int) extern("opal.prim") "int.equal" -> bool

   fn to_int () { x }
   fn to_real () extern("opal.prim") "int.to_real" -> real
   fn to_long () extern("opal.prim") "int.to_long" -> long
   fn to_char () extern("opal.prim") "int.to_char" -> char
}
impl x : real {
   fn add (y : real)   extern("opal.prim") "real.add" -> real
   fn sub (y : real)   extern("opal.prim") "real.sub" -> real
   fn mul (y : real)   extern("opal.prim") "real.mul" -> real
   fn div (y : real)   extern("opal.prim") "real.div" -> real
   fn mod (y : real)   extern("opal.prim") "real.mod" -> real
   fn succ ()          extern("opal.prim") "real.succ" -> real
   fn pred ()          extern("opal.prim") "real.pred" -> real
   fn neg ()           extern("opal.prim") "real.neg" -> real
   fn cmp (y : real)   extern("opal.prim") "real.cmp" -> int
   fn equal (y : real) extern("opal.prim") "real.equal" -> bool

   fn to_int () extern("opal.prim") "real.to_int" -> int
   fn to_real () { x }
   fn to_long () extern("opal.prim") "real.to_long" -> long
}
impl x : long {
   fn add (y : long)   extern("opal.prim") "long.add" -> long
   fn sub (y : long)   extern("opal.prim") "long.sub" -> long
   fn mul (y : long)   extern("opal.prim") "long.mul" -> long
   fn div (y : long)   extern("opal.prim") "long.div" -> long
   fn mod (y : long)   extern("opal.prim") "long.mod" -> long
   fn succ ()          extern("opal.prim") "long.succ" -> long
   fn pred ()          extern("opal.prim") "long.pred" -> long
   fn neg ()           extern("opal.prim") "long.neg" -> long
   fn cmp (y : long)   extern("opal.prim") "long.cmp" -> int
   fn equal (y : long) extern("opal.prim") "long.equal" -> bool

   fn to_int () extern("opal.prim") "long.to_int" -> int
   fn to_real () extern("opal.prim") "long.to_real" -> real
   fn to_long () { x }
}
impl x : char {
   fn to_int () extern("opal.prim") "char.to_int" -> int
}
impl x : bool {
   fn add (y : bool) { if x { x } else { y } }
   fn mul (y : bool) { if x { y } else { x } }
   fn to_int ()      { if x { 1 } else { 0 } }
   fn to_long ()     { if x { 1L } else { 0L } }
}
