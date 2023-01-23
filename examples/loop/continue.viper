count = 5

for i = 1; i <= 10; i = i + 1 {
    if count != 0 {
        count = count - 1
        continue
    }
    print i
}