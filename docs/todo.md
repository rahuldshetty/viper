1) Extend `clox` to allow more than 256 local variables to be in scope at a time.

2) Many languages make a distinction between variables that can be reassigned and those that can’t. In Java, the final modifier prevents you from assigning to a variable. In JavaScript, a variable declared with let can be assigned, but one declared using `const` can’t. Swift treats let as single-assignment and uses `var` for assignable variables. Scala and `Kotlin` use `val` and var.

Pick a keyword for a single-assignment variable form to add to Lox. Justify your choice, then implement it. An attempt to assign to a variable declared using your new keyword should cause a compile error.

3) Our simple local array makes it easy to calculate the stack slot of each local variable. But it means that when the compiler resolves a reference to a variable, we have to do a linear scan through the array.

Come up with something more efficient. Do you think the additional complexity is worth it?


4) Long Constant Values

5) Run-length encoding for lines

6) Ternary Operator

7) multiple compilers running in parallel

8) Optimization by OP_POPN if sequence of stack items to be removed

9)  Tunable parameter - pick best

10) String interpolation ${}
11) Contextual Keywords - `async, await`, 
12) Optimize in-place stack binary operation

13)  The compiler adds a global variable’s name to the constant table as a 
string every time an identifier is encountered. 
It creates a new constant each time, even if that variable name is already
in a previous slot in the constant table. That’s wasteful in cases where the same 
variable is referenced multiple times by the same function. That, in turn, increases 
the odds of filling up the constant table and running out of slots since we allow only 
256 constants in a single chunk.

Optimize this. How does your optimization 
affect the performance of the compiler compared to the runtime? Is this the right trade-off?

14) Looking up a global variable by name in a hash table 
each time it is used is pretty slow, even with a good hash table.
 Can you come up with a more efficient
 way to store and access global variables without changing the semantics?

15) Dynamically grow Stack

16) Add support for Switch Construct

switch-Statement     → "switch" "(" expression ")"
                 "{" switchCase* defaultCase? "}" ;
switchCase     → "case" expression ":" statement* ;
defaultCase    → "default" ":" statement* ;
To execute a switch statement, first evaluate the parenthesized switch value expression. Then walk the cases. For each case, evaluate its value expression. If the case value is equal to the switch value, execute the statements under the case and then exit the switch statement. Otherwise, try the next case. If no case matches and there is a default clause, execute its statements.



17) Add support for break and continue statements.

A continue statement jumps directly to the top of the nearest enclosing loop, skipping the rest of the loop body. Inside a for loop, a continue jumps to the increment clause, if there is one. It’s a compile-time error to have a continue statement not enclosed in a loop.

Make sure to think about scope. What should happen to local variables declared inside the body of the loop or in blocks nested inside the loop when a continue is executed?

18) 