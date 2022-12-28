class A{
    fn method(){
        print "A method"
    }
}

class B (A) {
    fn method(){
        print "B method"
    }

    fn test(){
        super.method()
    }
}

b = B()

b.method()

b.test()
