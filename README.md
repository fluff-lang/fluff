# The Fluff programming language

Fluff is a programming language designed for type safety, low memory usage and speed. Think of it as a mix of Rust, C++ and Typescript, all bundled in a lovingly written bundle for the true :3c users.

```
class Animal {
    virtual func greet() -> string;
}

class Dog : Animal {
    func greet() -> string {
        return "woof!";
    }
}

let animal: Animal = Dog.new();
println(animal.greet()); // woof!

```

![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/fluff-lang/fluff/build.yml)
![Linux](https://img.shields.io/badge/platform-linux-white?logo=linux&logoColor=white)
![Unix](https://img.shields.io/badge/platform-unix-white?logo=freebsd&logoColor=white)

## Directory structure

- `src/` - Source files
    - `core/` - Language configuration and data structures 
    - `parser/` - Parser and IR generation code
- `include/` - Helper includes for the source code files

## Contributing

Fluff is, at it's core, an experimental toy language. You can get started by reading the source code from the bottom up using the comments.

The source is very straight forward and not difficult to navigate, however you can always open an issue in case any section needs more documentation.