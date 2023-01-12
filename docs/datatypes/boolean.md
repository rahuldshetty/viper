# Boolean <!-- {docsify-ignore-all} -->

Boolean data type represents one bit of information that is either true or false.

- Viper recognizes "true" and "false" as reserved keywords to generate Boolean values.
- All conditional expression in Viper evaluates to a Boolean value.

## Example

```boolean.viper
a = true
b = false
print "A: " + str(a)
print "B: " + str(b)
print "A != B: " + str(a != b)

one = 1
two = 2

print "ONE: " + str(one)
print "TWO: " + str(two)
print "ONE == ONE: " + str(one == one)
print "ONE == TWO: " + str(one == two)
print "ONE + ONE == TWO: " + str(one + one == two)
```


Output
```
A: true
B: false
A != B: true
ONE: 1
TWO: 2
ONE == ONE: true
ONE == TWO: false
ONE + ONE == TWO: true
```

