Contributing guide
==================

Thanks for your interest in contributing to SPTK.

SPTK is currently under construction.
Any kinds of contributions are not welcome at the moment.


Development
-----------
For the development of SPTK, you may need to install the following programs.

- CMake 3.3+
- Python 3.6
- Valgrind
- Virtualenv

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
Please also perform formatting before sending a pull request by running
```sh
make format
```

### Document generation
You can generate the SPTK reference manual as follows:
```sh
make doc
```
The generated manual is in `doc/_build/html/`.
