# Control Statements

These statements are used to control the flow of the execution in a program. Viper program are executed in the top-to-bottom order in which the instructions are written, but with control statements we can change the flow of this behaviour. 

Viper supports two main types of control statements:
1) Decision Making Statements
2) Loop Statements

## Decision Making Statements

These statement let you control whether a block of code has to be executed or not based on some decision/condition.

### If-Else Statements

The most basic form of control statement is the if-else statements. These let you control whether a piece of code called as "if-block" has to be executed based on condition. In case the condition fails, we can have an optional block of statement that can be triggered which is called "else-block".

- if-block will be triggered only when the condition evaluates to a truth value (Boolean true or non-null values).
- else-block is optional.

Syntax:
```
if <condition> {
    <if-block>
}

if <condition> {
    <if-block>
} else {
    <else-block>
}
```

#### Example:

```if_exammple.viper
number = 10

if  number >= 10   {
    print "Number greater than or equal 10"
} else {
    print "Number is less than 10"
}

```

Output
```
Number greater than or equal 10
```


### Switch statement

This is a special form of if/else ladder where you have one condition expression with multiple case branches. Based on the value of condition only one of the case statement is taken for execution. 

- You can have a default case statement which will be selected when none of the switch cases are selected.
- Case statement has its own block level scoping.

Syntax:
```
switch <case-condition> {
    case <possible-case-value-1>: <case-statement1>
    case <possible-case-value-2>: <case-statement2>
    .
    .
    .
    case <possible-case-value-n>: <case-statementN>
    default: <default-case-statement>
}

```

#### Example:

```switch.viper
i = 1
switch i {
    case 1: {
        print "one"
    }
    case 2: print "two"
    case 3: print "three"
    default: print "default"
}
```

Output
```
one
```

## Loop Statements

These statement lets you to repeatedly execute block of statements called as "loop-body" while some condition is met. These are similar to decision making but unlike moving the control to next line in if-else, we go back to the condition and evaluate whether the loop-body has to be re-executed or not.

Viper supports two main types of looping statements.

### While Loop

While loop are the most basic form of looping statements where we iterate and run the loop-body as long as the condition is true.

Syntax:
```
while <condition>{
    <loop-body>
}
```

#### Example

```while_loop.viper
print "Program: Fibonacci Sequence"

f = 0
s = 1

n = 10

i = 2

print f
print s

while i <= n {
    t = f + s
    print t

    f = s
    s = t

    i = i + 1
}

```

Output
```
Program: Fibonacci Sequence
0
1
1
2
3
5
8
13
21
34
55
```


### For Loop

For loop are similar to while-loop in nature but can include initialization and increment-decrement operations as part of its for-loop statement definition. The initialization lets you to define variables that are required during the loop execution and increment-decrement operation lets you to modify variables to progress through the loop. Control flow will be inside the for-loop statement as long as the condition is met.

Syntax:
```
for <initialization>; <condition>; <increment/decrement expression> {
    <loop-body>
}
```

#### Example

This is the same example for Fibonacci program but with for-loop.

```for_loop.viper
// Fibonacci Program

f = 0
s = 1

print f
print s


for i = 2 ; i <= 10 ; i = i + 1  {
    t = f + s
    print t;
    f = s
    s = t
}

```

Output
```
0
1
1
2
3
5
8
13
21
34
55
```

## Note
