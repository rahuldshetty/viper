# Object Oriented Programming (OOP) <!-- {docsify-ignore-all} -->

Object Oriented Programming is programming paradigm built on the concepts of objects. Object compromises of two types of elements:
1. Instance Fields - Used to represent state of the object.
2. Methods/Procedures - Operation that can be performed on the object.

In Viper, you can build your own objects using classes. A class refers to a blueprint from which objects can be created.


- While lookup instance fields have higher priority than methods.
- You can use *this* operator to refer to the same object within the class. This lets you refer to instance fields or invoke other class methods.

## How to create a Class?

While creating a class in Viper, you need to specify the class name along with set of its function methods. 

Class Syntax:
```
class <className> {
    <functions-definitions>
}
```

Creating an object is similar to calling a function. You provide class name and pass initializer arguments if the class takes any.

Object Syntax:
```
<className>(<optional-initializer-arguments>)
```

## Example:

```class.viper
class Circle{
    fn set_radius(radius){
        this.radius = radius
    }

    fn area(){
        print "Area of circle: " + str(3.142 * this.radius * this.radius)
    }
}

c = Circle()
c.set_radius(2.3)
c.area()

```

Output:
```
Area of circle: 16.6212
```

## Note

