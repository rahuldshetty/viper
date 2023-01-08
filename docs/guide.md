
# Quick Start <!-- {docsify-ignore} -->

## Installation

Download viper binary from the Github release page: [alpha v1.0](https://github.com/rahuldshetty/viper/releases/tag/v1.0)

Run the interpreter to start using viper:

```bash
$ sh viper-linux-amd64
>>
```

Alternatively, you can pass **.viper* files to run the script. You can follow below example to run a viper file. 

## Say 'Hello World' in viper

Create a simple file called *[hello-world.viper](https://github.com/rahuldshetty/viper/tree/master/examples/hello-world.viper)*:
```hello-world.viper
// Variables
name = "John Doe"
names = ["World", "Vipers", name]

// Simple Function
fn hello(name){
    print "Hello, " + name + "!"
}

for(i = 0; i < len(names) ; i = i + 1){
    hello(names[i])
}

```

Run the script with viper:
```bash
$ sh viper-linux-amd64 ./examples/hello-world.viper
Hello, World!
Hello, Vipers!
Hello, John Doe!
```
