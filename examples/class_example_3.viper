
class StringStack{
    fn StringStack(init){
        this.init = init
        this.count = len(init)
    }

    fn len(){
        return this.count
    }

    fn out(){
        return this.init
    }

    fn append(s){
        this.init = this.init + s
        this.count = this.count + len(s)
    }
}

s = StringStack("hello")
print s.len()

