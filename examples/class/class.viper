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
