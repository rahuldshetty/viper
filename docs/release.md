# Release Notes <!-- {docsify-ignore-all} -->

## Alpha: 1.1 [WIP]

- Added support for Escape Characters in String - \n \r \t \' \\
- Support for switch branching statements.
- Modulo Operator
- Single quoted string datatype.
- Shorthand expressions: += -= *= /= %=
- Support for File Handling operations.
- Additional support for Number datatypes:
    - Binary number format. Eg: 0b000001101
    - Octal number format. Eg: 0c716
    - Hexadecimal number format. Eg: 0x4adF
- Language internal changes:
    - Removed Scanner, Parser from global definition.
    - Dynamic growing value stack for real-time production application of Viper language.
- Added native method "bytes" to handle bytes data stream. 

## Alpha: 1.0

- Viper's first release.
- Variables & Various datatype support: Number, String, Boolean, List, Map
- Single & Multi-line Comments
- Operators:
    - Unary (Negation & Negative)
    - Binary (Arithmetic & Logical)
    - Special (Ternary, Call, Print, Equality etc.)
- Branching Statements: if/else
- Looping Statements: for, while
- C's Ternary Operator ?:
- Built-in & User-defined Functions
- Object Oriented Programming
    - Class members & methods
    - Constructor
    - Inheritance
    - Support for this & super keyword reference.