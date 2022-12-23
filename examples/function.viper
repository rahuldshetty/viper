// Recursive Fibonacci Program

fn fibonacci(n) {
  if n == 0 or n == 1 {
    return n;
  } else {
    return fibonacci(n-1) + fibonacci(n-2)
  }
}

for i = 0; i <= 10; i=i+1 {
  print fibonacci(i);
}
