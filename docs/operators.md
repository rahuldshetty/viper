# Operators <!-- docs/_sidebar.md -->

Operators are special symbol(s) reserved in Viper to perform actions to generate some result. Each operators have their own syntax rules defined in the compiler and inputs for the operation are processed by these rules.

- Input values used in operator expressions are called *operands*.

Viper supports mainly 4 types of operators.
 
### Unary Operators

- These operators work with single operand value.

| Operator | Description | Example |
| ------ | ----------- | ----------- |
| Unary Not ! | Negate Boolean value from True to False or vice versa. | !true |
| Unary Minus - | Number is multiplied by -1 to get negative value. | -1 |

### Arithmetic Operators

- These are binary operators that perform Mathematical operations. 
- Both the operands must be number data types.
- Result of arithmetic operations are always Number data type.

| Operator | Description | Example |
| ------ | ----------- | ----------- |
| Add + | Adds two number operands. | 4 + 10 |
| Minus - | Subtracts value of first operand by second operand. | 4 - 10 |
| Divide / | Divides value of first operand by the second to result in division quotient. | 64 / 32  |
| Multiply * | Multiplies first operand with the second operand value. | 32 * 2 |

### Conditional Operators

- These operators compare the magnitude of two operands.
- Result of arithmetic operations are always Boolean data type.

| Operator | Description | Example |
| ------ | ----------- | ----------- |
| Less than < | Compares whether left operand value is less than right operand value.  | 4 < 10 |
| Less than or Equal <= | Compares whether left operand value is less than or equal to right operand value.  | 10 <= 10 |
| Greater than > | Compares whether left operand value is greater than right operand value. | 64 > 32  |
| Greater than or Equal >= | Compares whether left operand value is greater than or equal to right operand value. | 32 * 2 >= 64 |
| Equals == | Compares whether two operands are equal. | 10 == 10  |
| Not Equals != | Compares whether two operands are not equal. | 1 != 1 |

### Logical Operators

- These operators are used to combine conditional expressions.

| Operator | Description | Example |
| ------ | ----------- | ----------- |
| Logical And | Compares whether two conditional expressions are true. | true and true |
| Logical Or | Compares whether either of the two conditional expressions are true. | false or true |

### Special Operators

| Operator | Description | Example |
| ------ | ----------- | ----------- |
| Assignment = | This operator is used to assign values to variables, list and map items.  | my_variable = 100 |
| String Concatenation + | Appends one string to the end of another string. | "hello" + " world!" |
| String Equality == | Compares equality of two string objects. | "hello" == "Hello"  |
| Print | Prints the value of given operand expression to stdout. | print "Hello" |
| Call () | Used to invoke a function or create class objects. | myFunction(Person("John Doe", 26)) |
| Ternary | This is a special form of conditional operator that is used to evaluate one of the two expression statement based on truth/false value of a condition. | result = (num % 2 == 0)? "even" : "odd" |

## Note

- String Equality operator performs direct memory address comparison as strings are interned in Viper.
- Operators like +, == perform different operations based on their operand types.
- Print Statement:  In many programming languages print would be implemented as a native function, but in Viper this has been been implemented prior to function implementation for easier testing of Viper programs.