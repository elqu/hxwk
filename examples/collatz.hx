fn odd(n: i32) -> bool {
    (n / 2) * 2 < n
}

fn collatz(n: i32) -> void {
    printf("%d ", n);

    if n < 2 {
    } else {
        let next = if odd(n) {3 * n + 1} else {n / 2};
        collatz(next)
    }
}

fn main() -> void {
    let x = 39086;
    collatz(x);
    printf("\n");   
}
