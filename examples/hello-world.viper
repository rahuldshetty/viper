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