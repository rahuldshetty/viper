# Inheritance <!-- {docsify-ignore-all} -->

Inheritance is a OOP's property by which an object of one class acquires the properties and behaviors of a parent class.

- Viper follows copy-down inheritance where the instance fields/methods are copied down from parent class to child class.
- Viper currently supports inheriting from only one parent class.
- You can use *super* keyword to refer to the parent class fields/methods from the child class.

### Example

```inheritance.viper
class Animal{
    fn Animal(name){
        this.name = name
    }

    fn eat(){
        print this.name + " eating..."
    }
}

class Bird (Animal) {
    fn Bird(name, type){
        super.Animal(name)
        this.type = type
    }

    fn fly(){
        print this.type + " flying..."
    }
}

parrot = Bird("Sam", "parrot")
parrot.eat()
parrot.fly()

```

Output:
```
Sam eating...
parrot flying...
```

## Note

