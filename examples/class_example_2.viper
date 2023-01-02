
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

    fn str(){
        return "Point(" +  str( (this.x) ) + ", " + str( (this.y) ) + ")"
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
print str(p1)

p2 = Point(3, 4)
print str(p2)

print p1.squared_distance(p2)
print p2.squared_distance(p1)

