0) Multi-line comments

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

18) Reading and writing the ip field is one of the most frequent operations inside the bytecode loop. Right now, we access it through a pointer to the current CallFrame. That requires a pointer indirection which may force the CPU to bypass the cache and hit main memory. That can be a real performance sink.

Ideally, we’d keep the ip in a native CPU register. C doesn’t let us require that without dropping into inline assembly, but we can structure the code to encourage the compiler to make that optimization. If we store the ip directly in a C local variable and mark it register, there’s a good chance the C compiler will accede to our polite request.

This does mean we need to be careful to load and store the local ip back into the correct CallFrame when starting and ending function calls. Implement this optimization. Write a couple of benchmarks and see how it affects the performance. Do you think the extra code complexity is worth it?

19) Native function calls are fast in part because we don’t validate that the call passes as many arguments as the function expects. We really should, or an incorrect call to a native function without enough arguments could cause the function to read uninitialized memory. Add arity checking.

20) Right now, there’s no way for a native function to signal a runtime error. In a real implementation, this is something we’d need to support because native functions live in the statically typed world of C but are called from dynamically typed Lox land. If a user, say, tries to pass a string to sqrt(), that native function needs to report a runtime error.

Extend the native function system to support that. How does this capability affect the performance of native calls?

21) Wrapping every ObjFunction in an ObjClosure introduces a level of indirection that has a performance cost. That cost isn’t necessary for functions that do not close over any variables, but it does let the runtime treat all calls uniformly.

Change clox to only wrap functions in ObjClosures that need upvalues. How does the code complexity and performance compare to always wrapping functions? Take care to benchmark programs that do and do not use closures. How should you weight the importance of each benchmark? If one gets slower and one faster, how do you decide what trade-off to make to choose an implementation strategy?

22) A famous koan teaches us that “objects are a poor man’s closure” (and vice versa). Our VM doesn’t support objects yet, but now that we have closures we can approximate them. Using closures, write a Lox program that models two-dimensional vector “objects”. It should:

Define a “constructor” function to create a new vector with the given x and y coordinates.

Provide “methods” to access the x and y coordinates of values returned from that constructor.

Define an addition “method” that adds two vectors and produces a third.

23) The Obj header struct at the top of each object now has three fields: type, isMarked, and next. How much memory do those take up (on your machine)? Can you come up with something more compact? Is there a runtime cost to doing so?

24) When the sweep phase traverses a live object, it clears the isMarked field to prepare it for the next collection cycle. Can you come up with a more efficient approach?

25) Mark-sweep is only one of a variety of garbage collection algorithms out there. Explore those by replacing or augmenting the current collector with another one. Good candidates to consider are reference counting, Cheney’s algorithm, or the Lisp 2 mark-compact algorithm.

26) 



