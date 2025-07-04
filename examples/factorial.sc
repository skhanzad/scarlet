// Factorial function example
function factorial(n: int): int {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

function main(): void {
    var n: int = 5;
    var result: int = factorial(n);
    
    print("Factorial of ");
    print(n);
    print(" is: ");
    print(result);
    
    // Test with different values
    var i: int = 1;
    while (i <= 5) {
        var fact: int = factorial(i);
        print("Factorial(");
        print(i);
        print(") = ");
        print(fact);
        i = i + 1;
    }
} 