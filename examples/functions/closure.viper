var x = "global"

fn outer() {
  var x = "outer";
  fn inner() {
    print x;
  }
  inner();
}

outer();