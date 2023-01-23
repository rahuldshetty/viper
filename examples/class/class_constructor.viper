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
