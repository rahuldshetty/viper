print "Program: Fibonacci Sequence"

f = 0
s = 1

n = 10

i = 2

print f
print s

while i <= n {
    t = f + s
    print t

    f = s
    s = t

    i = i + 1
}
