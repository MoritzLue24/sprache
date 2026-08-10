
fn horner(x) {
    var a = 3 * x;
    var b = a + 2;
    var c = b * x;
    var result = c + 1;
    return result;
}

fn main() {
    var res = horner(7);
    return res;
}
