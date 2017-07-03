fn mod(a: i32, b: i32, n: i32) -> i32 {
    if (n*b < a) {
        if (a - (n + 1)*b < 0) {
            a - n*b
        } else {
            mod(a, b, n + 1)
        }
    } else { 0 }
}

fn prime(a: i32, n: i32) -> i32 {
    if (mod(a, n, 1) - 1 < 0) {
        if (n - 1 < a) {
            if (a - 1 < n) {
                prime(a, n + 1)
            } else { 0 }
        } else { 0 }
    } else {
        if (n < a) {
            if (a < n + 2) { 1 } else {
                prime(a, n + 1)
            }
        } else {
            prime(a, n + 1)
        }
    }
}

fn primesInRange(a: i32, b: i32) -> void {
    if (a < b + 1) {
        if (prime(a, 3) - 1 < 0) {} else {
            printf("%d ist prim.\n", a);
        };
        primesInRange(a + 1, b);
    } else {};
}

fn main() -> void {
    let a = 3;
    let b = 30;
    printf("2 ist prim.\n");
    primesInRange(a, b);
}
