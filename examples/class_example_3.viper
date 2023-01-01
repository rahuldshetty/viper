
class StringStack{
    fn StringStack(init){
        this.init = init
    }

    fn len(){
        return len(this.init)
    }

    fn out(){
        return this.init
    }

    fn append(s){
        this.init = this.init + s
    }
}

s = StringStack("")
s.append("hello")

print s.out()
print len(s)

s.append(" world")
print s.out()
print len(s)
