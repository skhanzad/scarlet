// Fibonacci sequence example
function fibonacci(n: int): int {
    if (n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

function main(): void {
    print("Fibonacci sequence:");
    
    var i: int = 0;
    while (i < 10) {
        var fib: int = fibonacci(i);
        print("F(");
        print(i);
        print(") = ");
        print(fib);
        i = i + 1;
    }
    
    // Calculate sum of first 10 Fibonacci numbers
    var sum: int = 0;
    var j: int = 0;
    while (j < 10) {
        sum = sum + fibonacci(j);
        j = j + 1;
    }
    
    print("Sum of first 10 Fibonacci numbers: ");
    print(sum);
} 