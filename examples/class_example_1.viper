class Point{
    fn set(x, y){
      this.x = x
      this.y = y
    }

    fn getx(){
      return this.x
    }

    fn gety(){
      return this.y
    }

    fn square_distance(point){
      x1 = this.x
      x2 = point.getx()

      y1 = this.y
      y2 = point.gety()

      x = (x2 - x1)
      y = (y2 - y1)

      return x*x + y*y
    }

}

p1 = Point()
p1.set(0, 0)

p2 = Point()
p2.set(3, 4)

print p1
print p2
print p1.square_distance(p2)
