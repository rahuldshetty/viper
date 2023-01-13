# Function

Functions are sequence of instruction grouped together as one executable unit for re-usability of logic.

- Function in Viper are first-class functions. 
- Provides support for closure *i.e* function defined within other functional block can persist all its environment scoped variables.
- They support Parameterization and Return statements. 
- You can call a function using call operator - (). 

## User-Defined Function

Functions defined in a script or program are called as User-Defined functions.

Functions are defined with *fn* keyword as follows:
```
fn <functionName>(<parameterNames-seperated-by-commas>){
    <function-body>
}
```

- Function body can have *return* statements which will handover the flow of control back to the parent block. Optionally, It can also return a value back to the parent caller.
- If no return type is specified then the function returns *null* by default.
- Viper supports recursive function definition where a function can call itself in its function body.

Function invocation is the process of assigning function parameter values and executing the function block.
It can be invoked with call operator:
```
<functionName>(<parameterValues-seperated-by-commas>)
```

### Example

```function.viper
// Recursive Fibonacci Program

fn fibonacci(n) {
  if n < 2 {
    return n
  } else {
    return fibonacci(n-1) + fibonacci(n-2)
  }
}

for i = 0; i <= 10; i=i+1 {
  print fibonacci(i);
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

## Native Function

These are the in-built C functions defined in Viper interpreter. 

- Developers can directly write C program and import them into Viper through its [Foreign Function Interface](https://en.wikipedia.org/wiki/Foreign_function_interface).

| Function | Description | Example |
| ------ | ----------- | ----------- |
| Length - len | Calculates length of given operand which can be of String, List and Map data types. | len([1, 2, 3]) |
| String - str | Converts given object value and returns its string representation. | str(100) |

## Note

- Function parameters have maximum limit of 255.