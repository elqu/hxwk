fn fac(a: i32) -> i32 {
    if a < 2 {
        1
    } else {
        a * fac(a - 1)
    }
}

fn main() -> void {
    let x = 6;
    printf("Die Fakultaet von %d ist %d.\n", x, fac(x));
}
