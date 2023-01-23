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
