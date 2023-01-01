fn hello(name){
    print "Hello " + name
}

size = 5

// List data structure
items = [1, 2, "hello", hello, 5, 6]

for(i = 0; i <= size; i = i + 1){
    print items[i];
}

// Map data structure
collection = {
    "1": 1,
    "2": 2,
    3: 3,
    4: hello,
    5: [1, 2, 3]
}

collection[5][0] = 0;

print collection

// call function
collection[4]("Viper")
