# Wave Language

The wave programming language is a project that I wrote in my free time.
The language is designed for use in embedded systems and its writing style is strongly inspired by C.
Wave is functional as well as static and strongly typed.

## What .wave code looks like:

```swift
func sum(int x, int y) : int {
    return x + y;
}

entrypoint(uint argument_count, str[] arguments) {
    print_format("the sum of 1 and 2 is: %i", sum(1, 2));
    
    exit 0;
}
```
(this example does not work for now)

## Current State

Working examples can be found in the ``resources/scripts/`` folder.
The language is still in its early stages, so don't expect too much.

## Building Wave

as of now only CMake has been tested

## Features

Current list of planned and implemented functions (though not all):

- [x] entrypoint
- [x] expressions
- [x] globals variables
- [x] local variables
- [ ] str data type
- [x] functions
    - [x] parameters and default values _(partially)_
    - [x] return statement
    - [x] extern functions
    - [x] function calls _(partially)_
    - [ ] variadic function parameters
- [x] statements
    - [ ] goto / label statement
    - [ ] if / else statement
    - [ ] ternary if statement
    - [x] do statement
    - [ ] while / while do statement
    - [ ] for / for else statement
    - [ ] try catch statement
- [ ] complex data types
    - [ ] enumerations
    - [ ] arrays
    - [ ] structs
- [ ] preprocessor
    - [ ] define statement
    - [ ] define arguments
    - [ ] if / else statement
    - [ ] variadic define arguments
    - [ ] preprocessor functions
        - [ ] concat function
        - [ ] stringify function
- [ ] other
    - [ ] namespaces
    - [ ] typedef
    - [ ] typeof
    - [ ] sizeof

## Notes

Compile-time features like debugging options can be enabled and disabled in the ``src/common/defines.h`` header.
