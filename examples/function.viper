// Recursive Fibonacci Program

fn fibonaci(n) {
  if n == 0 or n == 1 {
    return n;
  } else {
    return fibonaci(n-1) + fibonaci(n-2)
  }
}

for i = 0; i <= 10; i=i+1 {
  print fibonaci(i);
}
