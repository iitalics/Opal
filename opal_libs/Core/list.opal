module Core

pub type list[#e] = Cons(#e, list[#e]) or Nil()
