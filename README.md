# Zippy

Small, fast, dynamically typed programming language without a garbage collector.

Compilation:

```bash
cd git rev-parse --show-toplevel`
gcc --std=c89 -O3 -Wall -I include src/*.c tests/test-data.c -o tmp-tests  &&  ./tmp-tests
```

The _testing_ is being performed in C89 (plus stdint.h and stddef.h) to be sure that I'm using completely vanilla C syntax. I don't know what constraints compilers for, say, Z80 chips might apply.
