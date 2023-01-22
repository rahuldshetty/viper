# List <!-- {docsify-ignore-all} -->

Lists in Viper are similar to Arrays but can store items of any data types. By annotation, it is a collection of things enclosed within square brackets[] and separated by comma. 

- List are mutable.
- Dynamically sized.
- Has Zero-index based addressing (First item of the list starts at index 0).
- Helper functions/operators to work with List objects.

## Example

```list.md
fn hello(){ print("hello"); }

var items = [ 1, 2, "hello", hello, 4 ]

items.push(5)

print "Popped element:" + str(items.pop(0))

for i=0 ; i < len(items); i = i + 1 {
    print items[i]
}

```

Output
```
Popped element:1
2
hello
<fn 'hello'>
4
5
```

## Note

- List elements are stored in consecutive locations in memory. 
- They are always created/expanded with additional buffer and this buffer limit is reached and a new element has to be inserted then, the existing list is copied onto a new dynamic with larger size.