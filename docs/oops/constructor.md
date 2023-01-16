# Constructor <!-- {docsify-ignore-all} -->

Constructors are class methods which has the same name as the class name and is invoked automatically by Viper at the first initialization of the object. 

- Viper supports optional constructor method.
- You can pass arguments to constructors.
- You cannot use return statements within a constructor.

### Example

```class_constructor.viper
class Circle{
    fn Circle(radius){
        this.radius = radius
    }

    fn area(){
        print "Area of circle: " + str(3.142 * this.radius * this.radius)
    }
}

c = Circle(2.3)
c.area()

```

Output:
```
Area of circle: 16.6212
```

## Note

