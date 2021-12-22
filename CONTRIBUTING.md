Contributing guide
==================

Thank you for your interest in contributing to SPTK.

Any kinds of contributions are welcome.


Development
-----------
For the development of SPTK, you may need to install the following programs:

- [Bison](http://ftp.gnu.org/gnu/bison/)
- [CMake](https://cmake.org/download/) 3.3+
- [flex](https://github.com/westes/flex/releases) 2.6.3
- Python 3.6+
- Valgrind

Then you can setup the development environment of SPTK by running
```sh
cd tools
make
```

### Unit testing
SPTK provides a unit testing framework.
Please perform unit testing before sending a pull request by running
```sh
make test
```

### Source code formatting
`ClangFormat` and `cpplint` are used to format source codes.
Please also perform the formatting before sending a pull request by running
```sh
make format
```

### Document generation
You can generate the SPTK reference manual as follows:
```sh
make doc
```
The generated manual is in `doc/_build/html/`.
