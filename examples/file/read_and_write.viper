message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi auctor lorem ex, sit amet vestibulum ligula vehicula placerat. Proin scelerisque lacus efficitur interdum pretium. Aliquam a mollis lectus, in tempor lorem. Aliquam feugiat non metus in accumsan. Quisque dolor neque, tincidunt id imperdiet cursus, pellentesque nec enim. Fusce lacus dolor, viverra sit amet porttitor at, aliquet eu nulla. Duis laoreet imperdiet hendrerit. Donec vehicula nulla nec ante posuere dapibus vel nec dui. Suspendisse potenti. Sed sed urna orci. Fusce facilisis erat sit amet purus sodales scelerisque.

Nunc placerat ut nisl a sagittis. Nulla accumsan eros non convallis cursus. Morbi tempus dignissim fermentum. Sed velit ipsum, pharetra sed pulvinar et, sagittis id dolor. Fusce nec nulla non nulla sollicitudin scelerisque. Aenean hendrerit euismod mi, id ultrices sem iaculis a. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Integer molestie gravida neque eu auctor. Morbi et varius sapien. Nulla ac est turpis. Cras in dolor dui. Aenean a enim vel risus tincidunt faucibus. Integer ullamcorper est id orci bibendum, et efficitur turpis pulvinar. Sed pellentesque sit amet urna eu cursus."

// Open file in write mode
print "Creating file..."
f = file("sample.txt", "w")
f.write(message)
f.close()
print "File closed: " + str(f.is_closed())


// Open file in read mode
print "Reading file..."
f = file("sample.txt")
print "File open: " + str(f.is_open())
print "File content: " + f.read()
f.close()
print "File open: " + str(f.is_open())

// Open file in write binary mode
print "Creating file in binary mode..."
f = file("sample.txt", "wb")
f.write(bytes([72, 69, 76, 76, 79, 10, 84, 72, 69, 82, 69]))
f.close()

// Open file in read binary mode
print "Reading file in binary mode..."
f = file("sample.txt", 'rb')
print "File content: " + str(f.read())
f.close()
