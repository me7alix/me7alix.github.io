# me7aLang

__me7aLang__ (metaLang) is a general-purpose programming language created for educational purposes. The compiler is kind of toyish, written in **C**, with a small number of dependencies and a simple design focused on clarity and learning.

## Features
- Written entirely in C
- Minimal dependencies
- Simple architecture for educational use
- Supports a growing subset of language features

### Macro features
me7aLang supports object-like and function-like macros, like C, but with few extra features. Function-like macros are followed with brackets so it's much more convenient to write them.

You can pass blocks of code in brackets and it will be parsed as a single argument, so it's allowed to write such macros:
```
macro do_while(expr, body) {
    while true {
        block body

        if !(expr) {
            break
        }
    }
}
```

## Getting Started
Clone the repository:
```bash
git clone https://github.com/me7alix/me7aLang.git
cd me7aLang
```

Build the compiler:
```bash
make release -B
```

Compile and run an example:
```bash
./build/release/m7c -I ./stdlib -o ./build/fib ./examples/fib.m7
./build/fib
```

Add the environment variable `METALANG_HOME` with the path to the compiler. Now you don't have to provide the standard library path manually:
```bash
./build/release/m7c -o ./build/fib ./examples/fib.m7
./build/fib
```

## Examples
```
import "std.m7"

fn main() {
    printf("Hello, World!\n")
}
```
Check the [examples](https://github.com/me7alix/me7aLang/tree/main/examples) directory to see what's currently implemented.

## License

This project is released under the MIT License.
