<div align="center" style="display:flex; align-items:center;justify-content: center;background:#e1e1e1;color:#0f0f0f;padding:50px;">
    <img alt="talion logo" src="https://raw.githubusercontent.com/rahuldshetty/viper/master/docs/_media/logo.png" width="150">
</div>

<p align="center">
    <img alt="no-languages" src="https://img.shields.io/github/languages/count/rahuldshetty/viper?color=red&style=flat-square">
    <img alt="commit-activity" src="https://img.shields.io/github/commit-activity/w/rahuldshetty/viper?color=green&style=flat-square">
   <img alt="stars" src="https://img.shields.io/github/stars/rahuldshetty/viper?style=social">
</p>


# Viper, it Bytes  <!-- {docsify-ignore-all} -->


> *Viper, it Bytes* (pun intended) is a dynamically-typed, byte-interpreted programming language written in C. This project is based out of a language called *clox* inspired from the book [Crafting Interpreters](https://craftinginterpreters.com).

## Features

- Free & Open Source.
- Bytecode virtual machine written in C language.
- Easy to Code (Constructs similar to Python & JavaScript).
- High Level interpreted language.
- Dynamically-Typed.
- Support for Object Oriented Programming.
- Dynamic memory allocation & Garbage collector (Mark-Sweep Algorithm).

## Installation

Install Viper [guide](https://rahuldshetty.github.io/viper/#/guide?id=installation).

## Example

```intro.md
names = ["Vipers", "World!", ""]

class HelloPrinter {
    fn print(name){
        print "Hello " + str(name)
    }
}

printer = HelloPrinter()

for (i = 0 ; i < len(names) ; i = i + 1){
    if ( names[i] != "") {
        printer.print(names[i])
    }
}
```

## Contributing

If you would like to contribute to the project or report any bugs, feel free to raise this in the Github Issue [tracker](https://github.com/rahuldshetty/viper/issues).  