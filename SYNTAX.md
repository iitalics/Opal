Opal Syntax
===========

This text assumes that the reader has experience with other programming languages, such as C++ or Java.

* [Scope](#--scope)
* [Types](#--types)
* [Functions](#--functions)
* [Expressions](#--expressions)
* [Objects and Interfaces](#--objects-and-interfaces)

### Comments ###

    // double-forward-slash indicates a comment
    // comments will be used throughout this document to describe syntax & behavior

Scope
----------------------------------------

Code in Opal is broken into two "scopes": toplevel scope and function scope.

Toplevel scope contains declarations of types, functions and module includes.
Function scope is within functions and contains most of the Opal code

    // this is toplevel scope
    use Core

    // function declaration in 'toplevel scope'
    fn main () {
        // code in 'function scope'
        IO::println("Hello, world!")
    }

Types
----------------------------------------

Opal has a static type system, like in Java or Haskell. 
Types are broken up into two major categories:

* Concrete types
* Parameter types

### Concrete ###
Concrete types represent a specific kind of data. Concrete types may have any or no arguments.
Function types and tuple types have specific syntax.

    // syntax:
    //   <name>
    //   <name> [ <arguments> ]
    //   ( <elements> )
    //   fn ( <arguments> ) -> <return-type>

    // examples:
    int               // integer type
    string            // string type
    list[int]         // list of integers
    list[list[int]]   // list of lists of integers
    map[string, int]  // mapping from strings to integers
    (int, bool)       // two-element tuple
    (int, int, int)   // three-element tuple
    fn(int) -> real   // a function taking an integer and returning a real

The following types are used to handle primitive data: `int`, `long`, `real`, `bool`, `char`, `string`, `unit`

### Parameter ###
Parameter types represent a generic type, meaning some data of any single type.
Similar to "generics" in Java and "polymorphic types" in Haskell.
Parameter types are named, and are bound across the function or type in which they are defined (more on that later).
Concrete types may contain parameter types as arguments.

    // syntax:
    //   #<name>

    // examples:
    #a                  // some type
    list[#a]            // list of some type
    list[#b]            // list of another type
    fn(list[#a]) -> #a  // a function taking a list of some type and returning
                        //  data of that type
    fn(#a, #b) -> #a    // a function taking values of two different types
                        //  and returning data of the first type
    fn(#a, #a) -> #a    // a function taking values of the same type and returning
                        //  data of that type


Functions
----------------------------------------

Functions are defined in the toplevel scope using the `fn` keyword.
Although Opal supports type inference, the types of each argument to the function must be specified.
However, the return value will be automatically inferred.

    // syntax:
    //  function:
    //    fn <name> ( <arguments> ) { <body> }
    //  argument:
    //    <name> : <type>

    // examples:
    fn pi () { 3.1415926535 }          // returns the mathematical constant "pi"
    fn twice (x : int) { x + x }       // takes an integer, 'x', and returns its double
    fn identity (x : #a) { x }         // takes some data of any kind and returns it

The return value of a function is the result of the expression that immediately proceeds it, as shown
in the examples above.

### A note on variables and types ###

In Opal, variables and types occupy two very seperate *namespaces*.
This means that a function (or variable) and a type with the same name do not conflict. It is
therefore possible to name functions `int` or `char`, without causing any issues, even though
types the same name exist. 

Expressions
----------------------------------------

In Opal, most code in the "function scope" is made up of expressions.
An expression is a portion of code that evaluates to some resulting value.
This section will list the most common kinds of expression.

### Literals ###

    // numeric constants
        // integers
        184
        -3
        
        // long integers (64-bit)
        184L

        // real numbers (64-bit double)
        4.5
        .3

    // other constants
        // characters
        'c'
        '\n'            // newline

        // strings
        "Hello, world"

        // boolean
        true
        false

    // tuples
        ()              // called "unit"
        (1, 2, 3)
        (true, "yes")
        (false, "no")

Example code:
    
    fn pi () { 3.1415926535 }
    fn increment (x : int) { x + 1 }

### Blocks ###

A "block" is a sequence of expression enclosed by `{}` and optionally seperated with `;`.
The result of a block is simply the last expression in the block, and an empty block returns the value `()` (of type `unit`).
We've already seen blocks in function declarations.

    { 4 }           // = 4
    { 1 2 3 }       // = 3
    { 1; 2; 3 }     // = 3
    {}              // = ()
    { 1; }          // = ()

### Operators ###
    
Opal uses infix notation, and has 27 total operators ([reference](#--operator-reference)).
It supports all of the common operators: `+`, `-`, `*`, `/`, `%` *etc*.

    1 + 1          // = 2
    1 + 2 * 3      // = 7
    5 % 2          // = 1
    - (4 * 2)      // = -8
    not (true)     // = false
    1 < 3          // = true

It also supports slicing operators (see **Operator Methods**)

    "hello"[0]     // = 'h'
    "hello"[1,3]   // = "el"
    "hello"[1,]    // = "ello"
    "hello"[,2]    // = "he"

### Function calls ###
    
Function calls look like function calls in most popular languages (C, Python, Java, etc.).

    // syntax:
    //   <function> ( <arguments> )

    fn twice (x : int) { x + x }
    ...
    twice(6)                       // = 12

### Conditions ###

"If" conditions in Opal are expressions as well. The "else" part is required, unless the expression
is inside of a block.

    // syntax:
    //   if <expression> { <expressions> } else { <expressions> }
    //   if <expression> { <expressions> } else if ...
    //   if <expression> { <expressions> }               (when in block expressions)

    if true { ":)" } else { ":(" }         // = ":)"
    if false { ":)" } else { ":(" }        // = ":("

    fn sum (n : int) { // = 0 + 1 + 2 + ... + n
        if n <= 0 {
            0
        } else {
            n + sum(n - 1)                 // yep, Opal supports recursion
        }
    }

### Variables ###

Local variables can be defined inside blocks using `let` syntax. No type annotations are required; the expression's
type is automatically inferred.

    // syntax:
    //   let <name> = <expression>

    { let x = 3 ; x + 1 }              // = 4

`let` also supports "destructuring".

    { let (x, y) = (1, 2) ; x + y }    // = 3 

### Lists ###

Opal contains an internal linked list type, `list`. The `$` operator is used to combine a list head and tail.
The empty list is denoted as `[]`.

    // syntax:
    //   []
    //   [ <expressions> ]
    //   <expression> $ <expression>

    [1, 2, 3]                   // = [1, 2, 3]
    1 $ (2 $ (3 $ []))          // = [1, 2, 3]

    fn range (a : int, b : int) {
        if a > b {
            []
        } else {
            a $ range(a + 1, b)
        }
    }
    ...
    range(3, 7)                 // = [3, 4, 5, 6, 7]

Lists can also be destructed using `let`

    { let x $ xs = [1, 2, 3, 4] ; x }   // = 1
    { let x $ xs = [1, 2, 3, 4] ; xs }  // = [2, 3, 4]

### Lambdas ###

Opal is a functional programming language, meaning it supports first class functions, lambdas and closures.
Lambdas are defined using the `fn` keyword in an expression. Type annotations for arguments are optional; however
without context the type inference algorithm may sometimes fail to infer types.

    // syntax:
    //   lambda:
    //     fn ( <arguments> ) { <body> }

    fn zero (f : fn(int) -> #t) { f(0) }s
    ...
    zero(fn (x : int) { x + 2 })   // = 2
    zero(fn (x) { x + x })         // = 0

    let k = 3
    zero(fn (x) { x < k })         // = true

    // example where the type inferer fails
    let twice = fn (x) { x + x } ;
    // solution:
    let twice = fn (x : int) { x + x } ;

A lot of code requires passing a lambda as the single argument to another function. 
As a syntactic shorthand, you may use `||` after an expression to immediately create
and pass a lambda:

    // syntax:
    //   <expression> | <arguments> | { <body> }

    zero |x| { x + 1 }    eqv. to    zero(fn (x) { x + 1 })

Objects and Interfaces
----------------------------------------

### Structure types ###

New types may be defined in the type level using the `type` keyword.

    // syntax:
    //   type:
    //     type <name> { <fields> }
    //     type <name> [ <parameters> ] { <fields> }
    //   field:
    //     <name> : <type>

    type plant {
        species : string,
        height : real
    }

Objects of the newly defined type are created using the `new` keyword. Values for every
field of the new object are required, but the order does not matter.

    // syntax:
    //   object:
    //     new <type> { <inits> }
    //   init:
    //     <name> = <expression>

    let some_plant = new plant {
        species = "Sunflower",
        height = 4.0
    }

### Members ###

Members (or methods) of data can be accessed using `.`, such as in languages like Java, C++ and Python.

    // syntax:
    //   <expression> . <name>

    some_plant.species        // = "Sunflower"

### Methods ###
Methods are functions that are fields of a type, may be defined using `impl` syntax.
Methods add functionality that is specific to data of a certain type.

    // syntax:
    //   impl <name> : <type> { <functions> }

    impl x : int {
        fn twice () { x + x }
    }
    impl self : plant {
        fn grow () {
            self.height = self.height + 0.1 ;
            self.height
        }
    }

    4.twice()          // = 8
    some_plany.grow()  // = 4.1

### Operator Methods ###

Most operators in Opal (`+`, `-`, ...) are actually just syntactic *sugar* for method calls. Each
operator has a special name referring to the method it calls.

For instance, the code `x + y` is exactly equivalent to the code `x.add(y)`, and the code `not x` is
exactly equivalent to `x.inv()`. Below lists some common operators and their corresponding method names:

    - (unary)  =>  .neg
    +          =>  .add
    *          =>  .mul
    ^          =>  .exp

*entire reference: [here](#--operator-reference)*

Comparison operators work differently from operators such as those listed above. Equality comparison
operators call the `.equal` method, while ordered comparison operators call the `.cmp` method, as described below:

    Name        Method        Returns 'true' if...
    ==          .equal        <result> equals 'true'
    !=          .equal        <result> equals 'false'
    <           .cmp          <result> is less than integer 0
    >           .cmp          <result> is greater than integer 0
    <=          .cmp          <result> is less than or equal to integer 0
    ...

    // examples:
    'a' > 'z'         eqv. to         'a'.cmp('z') > 0
    x != y            eqv. to         x.equal(y) == false

The operators `and` and `or` are *lazy evaluated*, operate only on the `bool` type, and do not
call any methods. The operator `$` does not call any methods.

### Interfaces ###

*or, where the type system gets interesting*

Interfaces are used to "constrain" parameter types to a subset that is garunteed to contain
a specified set of methods. To demonstrate:

>  Suppose you want to write a generic function `max` that takes two inputs, and returns the larger
>  of the two. Concrete types are too specific; we don't want to have to define a different
>  function for every comparable type (`int`, `real`, `char`...). However, parameter type could
>  represent any data, which is certainly not garunteed to be of a type that can be "ordered".

The code

    fn max (x : #a, y : #a) {
        if x > y { x } else { y }
    }

fails to compile.

To solve this problem, we create an "interface" that specifies types that support the `.cmp` method.
Now, we change the parameter type `#a` to be *constained* by this new interface, which makes it only
valid to pass data into the function if that data correctly supports the `.cmp` method.

    // syntax:
    //   iface:
    //     iface <name> { <methods> }
    //     iface <self parameter> : <name> { <methods> }
    //   method:
    //     fn <name> ( <types> ) -> <return type>

    iface #t : Ordered {
        fn cmp (#t) -> int
    }
    // our new 'Ordered' interface requires types to have a '.cmp' method
    // e.g. for the type 'char' to be a valid 'Ordered' type, it has
    //  to have a method that takes an argument of type 'char' and
    //  returns type 'int'

Parameter type constraints are denoted within `()` in the **first** appearance
of the parameter type. 

    // syntax:
    //   parameter:
    //     #<name>
    //     #<name> ( <interfaces> )

    fn max (x : #a(Ordered), y : #a) {
        if x > y { x } else { y }
    }

    // note:
    fn max (x : #a(Ordered), y : #a(Ordered))
    // and
    fn max (x : #a, y ; #a(Ordered))
    // are both invalid

The [standard library](opal_libs/Lang/ifaces.opal) defines some interfaces for
common use.


Operator Reference
---------------------

    Operator  Precedence  Associativity  Method
    -----------------------------------------------
    -         0           unary          .neg
    not       0           unary          .inv
    [x]       1           unary          .get/.set
    [x,y]     1           unary          .slice
    [x,]      1           unary          .slice_from
    [,y]      1           unary          .slice_to
    ^         2           right          .exp
    *         3           left           .mul
    /         3           left           .div
    %         3           left           .mod
    +         4           left           .add
    -         4           left           .sub
    $         5           right          (N/A)
    <         6           left           .cmp
    >         6           left           .cmp
    <=        6           left           .cmp
    >=        6           left           .cmp
    ==        6           left           .equal
    !=        6           left           .equal
    and       7           left           (N/A)
    or        7           left           (N/A)
    <<        8           left           .lshift
    |<        8           left           .lblock
    <|        8           left           .lbind
    >>        8           left           .rshift
    >|        8           left           .rblock
    |>        8           left           .rbind
