names = ["Vipers", "World!", ""]

class HelloPrinter {
    fn hello(name){
        print "Hello " + str(name)
    }
}

printer = HelloPrinter()

for (i = 0 ; i < len(names) ; i = i + 1){
    if ( names[i] != "") {
        printer.hello(names[i])
    }
}