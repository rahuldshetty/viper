// Fibonacci Program
// in language "Viper, it Bytes".

f = 0
s = 1

print f
print s


for i = 2 ; i <= 10 ; i = i + 1  {
    t = f + s
    print t;
    f = s
    s = t
}

