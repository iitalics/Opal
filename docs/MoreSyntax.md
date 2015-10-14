More Advanced Features
======================

* [Modules](#modules)
* [Standard Library](#standard-library)
* [Enum Types](#enum-types)
* [Pattern Matching](#pattern-matching)
* [Type Arguments](#type-arguments)

See also

* [Syntax and Features](Syntax.md)

## Modules ##

Opal supports a basic module system, for seperating namespaces.
Specify the module of the current file in the toplevel scope using the `module` syntax.
A file may only belong to a single module.

    // syntax:
    //   module <module name>

Export a function or type for use in other files by making it "public", using the `pub` keyword.
Non-public functions and types are not immediately accessable by any other files.

    // syntax:
    //   pub fn <name> ...
    //   pub type <type> ...

    module A
    pub fn thing () {}

Refer to exported functions and types from other modules using `::`.

    // syntax:
    //   <module name> :: <name>

    A::thing()

### Importing Modules ###

You can import every function and type from a module into the current file
using the `use` syntax.

    // syntax:
    //   use <module name>

    use A
    ...
    thing()

Modules are imported by looking in the [opal_libs](../opal_libs) folder and finding
a sub-folder with the same name as the module name, and then loading every source
code file (`.opal`) within that folder.

    // ./
    // opal_libs/
    //   MyModule/
    //     file1.opal
    // main.opal

    // "./opal_libs/MyModule/file1.opal"
    module MyModule
    pub fn do_a_thing (x : int) { x + 3 }

    // "./main.opal"
    fn main () {
        MyModule::do_a_thing(4)
    }

## Standard Library ##

The standard library for Opal is broken up into four main modules

There currently isn't full documentation for the standard library, but that
will likely change soon. For now, see [examples](../examples)

### `Core` ###

The `Core` module defines primitive types, and methods for manipulating these types.
It also has functions for functional programming and list comprehension.

    // examples
    use Core

    "hello".len()                // = 5
    4.to_real()                  // = 4.0
    5.2.str()                    // = "5.2"
    [1, 2, 3].map |x| { x * 2 }  // = [2, 4, 6]

### `Lang` ###

The `Lang` module defines some convienient types and interfaces.

    // examples
    let x = 1;
    let ev = Lang::event();
    ev.bind |arg| { x = x + arg };
    ev.fire(4)       // x = 5
    ev.fire(2)       // x = 7

    let a = Lang::array();
    a.fill('a', 4);
    a.insert(1, 'b');
    a.push('c');
    a.to_list()      // ['a', 'b', 'a', 'a', 'a', 'c']

### `IO` ###

The `IO` module provides an interface to the filesystem, as well standard in/out.

    {
        let ofs = IO::open_out("./test.txt");
        ofs.write("hello");
    }
    let ifs = IO::open_in("test.txt");
    let line = ifs.read_line();
    IO::console().write("data: '" + line + "'\n")

### `Math` ###

The `Math` module contains some common mathematical functions.

    Math::sin(0.0)        // = 0.0
    Math::cos(Math::PI()) // = -1.0
    Math::rand()          // = {random: 0.0 <= x < 1.0}
    (1, 12).rand_incl()   // = {random: 1 <= x <= 12}

## Enum Types ##

Opal supports **enums** (also called *sum types*).
An enum is defined by a number of different constructor functions. Data in
an enum is immutable, and can only be accessed through [pattern matching](#pattern-matching).
Enums are very useful constructs for functional programming.

    // syntax:
    //   enum:
    //     type <type> = <ctors> or ...
    //   ctor:
    //     <name> ( <types>, ... )

    type shape =
           Line(real)       // length
        or Circle(real)     // radius
        or Rect(real, real) // width, height

    ...
    let shape1 = Rect(3.0, 4.0)
    let shape2 = Line(5.0)

## Pattern Matching ##

The `match` expression is something like a greatly more convenient version of
the `select` construct in C-like languages.

    // syntax:
    //   match:
    //     match <expression> { <cases> }
    //   case:
    //     <pattern> { <expressions> }

    fn message (place : int) {
        match place {
            1  { "First place" }
            2  { "Second place" }
            3  { "Third place" }
            _  { "Honorable mention" }
        }
    }

`match` uses a rich pattern syntax. Patterns can be used to compare data
or destructure data such as lists, tuples and enums.
We've already seen patterns in [let destructuring](Syntax.md#variables).

    // syntax:
    //   pattern:
    //     <name>
    //     <constant>
    //     <pattern> $ <pattern>
    //     ( <patterns>, ... )
    //     <ctor name> ( <patterns>, ... )

    impl self : shape {
        fn area () {
            match self {
                Line(len)   { 0.0 }
                Circle(rad) { 3.1415926535 * rad * rad }
                Rect(w, h)  { w * h }
            }
        }
    }

## Type Arguments ##

Declared types may take any number of parameter type arguments:

    type map[#k, #v] {
        keys : list[#k],
        vals : list[#v]
    }
    type list'[#e] = Nil'() or Cons'(#e, list'[#e])

Interfaces may also take argumnets:

    iface Factory[#out] {
        fn make () -> #out
    }
    fn getThreeShapes (factory : #f(Factory[Shape])) {
        [factory.make(),
         factory.make(),
         factory.make()]
    }
