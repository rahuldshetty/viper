
class Point{
    fn Point(x, y){
        this.x = x
        this.y = y
    }

    fn get_x(){
        return this.x
    }

    fn get_y(){
        return this.y
    }

    fn out(){
        print "Point ("
        print this.x 
        print this.y
        print ")"
        print ""
    }

    fn squared_distance(point){
        x1 = this.x
        x2 = point.x

        y1 = this.y
        y2 = point.y

        x = (x2 - x1)
        y = (y2 - y1)

        return x*x + y*y
    }

}

p1 = Point(0, 0)
p1.out()

p2 = Point(3, 4)
p2.out()

print p1.squared_distance(p2)
print p2.squared_distance(p1)

