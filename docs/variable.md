
# Variables <!-- {docsify-ignore-all} -->

**Viper** is dynamically typed programming language that means developers don't have to specify any datatypes during variable declaration.

- Identifier name should always start with alphabetical characters followed by alphabet/numerical/underscore characters. This rule follows throughout the language to define class/objects/functions or any other values.

- Variable binding to identifier can be done with variable keyword - *var* or just by assigning an identifier name with value.

    Syntax:
    ```viper
    var <identifier> = <value>
    <identifer> = <value>
    ```

### Example

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
