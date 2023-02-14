# File Handling

In Viper, you can use the built-in `file()` method to create File Objects. These file objects support native functions to perform read/write operation on files.

The file method takes in two parameters. The first mandatory parameter is the file path which can be absolute or relative path to the file reference. The second parameter is an optional argument that  is used to specify in what mode does the file has to be opened (Default: read mode "r").

```
file(<File path>, <optional File mode>)
```

## Example

```read_and_write.viper
message = "...."

// Open file in write mode
print "Creating file..."
f = file("sample.txt", "w")
f.write(message)
f.close()
print "File closed: " + str(f.is_closed())


// Open file in read mode
print "Reading file..."
f = file("sample.txt")
print "File open: " + str(f.is_open())
print "File content: " + f.read()
f.close()
print "File open: " + str(f.is_open())
```
