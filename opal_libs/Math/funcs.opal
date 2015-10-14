module Math
use Core

pub fn sin (x : real) extern("opal.math") "sin" -> real
pub fn cos (x : real) extern("opal.math") "cos" -> real
pub fn tan (x : real) extern("opal.math") "tan" -> real
pub fn asin (x : real) extern("opal.math") "asin" -> real
pub fn acos (x : real) extern("opal.math") "acos" -> real
pub fn atan (x : real) extern("opal.math") "atan" -> real
pub fn atan2 (x : real, y : real) extern("opal.math") "atan2" -> real

pub fn exp (x : real) extern("opal.math") "exp" -> real
pub fn log (x : real) extern("opal.math") "log" -> real
pub fn log2 (x : real) extern("opal.math") "log2" -> real
pub fn sqrt (x : real) extern("opal.math") "sqrt" -> real

pub fn PI ()  { 3.141592653589793 }
pub fn PI2 () { 6.283185307179586 }
pub fn E ()   { 2.71828182846 }

