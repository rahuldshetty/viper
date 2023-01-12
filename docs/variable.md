
# Variables <!-- {docsify-ignore-all} -->

**Viper** is dynamically typed programming language that means developers don't have to specify any datatypes during variable declaration.

- Identifier name should always start with alphabetical characters followed by alphabet/numerical/underscore characters. This rule follows throughout the language to define class/objects/functions or any other values.

- Variable binding to identifier can be done with variable keyword - *var* or just by assigning an identifier name with value.

    Syntax:
    ```viper
    var <identifier> = <value>
    <identifer> = <value>
    ```

## Example

Create a file called *[variable.viper](https://github.com/rahuldshetty/viper/tree/master/examples/variable.viper)*:

```variable.viper
// Variable
SEPERATOR = "--------------------------------------"

var breakfast = "beignets";
beverage = "cafe au lait"

what_i_had = "beignets with " + beverage;

print "What I had? " + what_i_had;

print SEPERATOR;

// List Example
items = [1, 2, "hello", "4"]

print "List Items: " + str(items);
print SEPERATOR;

```

Run the script with Viper:
```bash
$ sh viper-linux-amd64 ./examples/variable.viper
What I had? beignets with cafe au lait
--------------------------------------
List Items: [1, 2, hello, 4]
--------------------------------------
```

## Data Types

Variables in Viper can store some of the standard types of data objects: `Number, Boolean, String, List, Map`

Data within a variable can be manipulated using functions and operators available as part of Viper interpreter (more info on this in [Operators](/operators.md) section). 

Based on the order of dependency between some of the data types, they have been grouped into two buckets.

### Primitive Datatype:

These are the basic form of data types that is used to represent a single entity value.

- [Number](/datatypes/number.md)
- [Boolean](/datatypes/boolean.md)
- [String](/datatypes/string.md)

### Secondary Datatype:

Secondary Data types refer to a collection of elements where each element can be of primitive or secondary datatype. It is used to represent a grouped entity.

- [List](/datatypes/list.md)
- [Map](/datatypes/map.md)


## Note

- Variables store **null** value by default when declared without assigning any value.

    Example:
    ```viper
    >> var hello
    >> print hello
    null
    ```

- All variables defined in Viper are stored in its VM's constant memory space. When declaring variable with an identifier, the identifier value (RHS) is added to constant memory space which gets associated with the identifier name (LHS).

- There are certain tokens/words which are pre-defined and reserved in Viper language to provide functionality. These names shouldn't be used as an identifier.
    ```
    and                 for             or              this
    class               fn              print           true
    if                  else            return          var
    false               null            super           while
    len                 str
    ```
