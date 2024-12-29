<center>
<h1>The Fluff programming language</h1>

![Static Badge](https://img.shields.io/badge/version-0.1.0-blue) [![License](https://img.shields.io/badge/license-MIT-green)](https://github.com/fluff-lang/fluff/blob/master/LICENSE.md) ![Commit activity](https://img.shields.io/github/commit-activity/w/fluff-lang/fluff?logo=github&label=commits)

</center>

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

## Build status

| Platform | Status |
| -- | -- |
|  ![Linux build status](https://img.shields.io/badge/platform-Linux-white?logo=linux&logoColor=white) | ![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/fluff-lang/fluff/ubuntu.yml)  |
|  ![MacOS build status](https://img.shields.io/badge/platform-macOS-white?logo=apple&logoColor=white) | ![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/fluff-lang/fluff/macos.yml)  |
|  ![FreeBSD build status](https://img.shields.io/badge/platform-FreeBSD-white?logo=freebsd&logoColor=white) | ![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/fluff-lang/fluff/freebsd.yml)  |
|  ![Windows build status](https://img.shields.io/badge/platform-Windows-white?logo=windows&logoColor=white) | ![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/fluff-lang/fluff/windows.yml)  |

## Directory structure

- `src/` - Source files
    - `core/` - Language configuration and data structures 
    - `parser/` - Parser and IR generation code
- `include/` - Helper includes for the source code files

## Contributing

Fluff is, at it's core, an experimental toy language. You can get started by reading the source code from the bottom up using the comments.

The source is very straight forward and not difficult to navigate, however you can always open an issue in case any section needs more documentation.