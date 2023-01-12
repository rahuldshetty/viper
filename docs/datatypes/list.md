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

for i=0 ; i < len(items); i = i + 1 {
    print items[i]
}

```

```Output
1
2
hello
<fn 'hello'>
4
```

## Note

- List elements are stored in consecutive location in memory. They are created with additional buffer and whenever a list is completed occupied, the existing list is copied onto a new dynamic list with larger size.
- 