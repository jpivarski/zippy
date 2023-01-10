# Zippy

Small, fast, dynamically typed programming language without a garbage collector.

- It's possible to run without a garbage collector because the objects are all eager and immutable, and hence cyclic references are impossible, so we can get away with pure reference counting.
- Immutable data often need to create many small objects rapidly. I conjectured that the bottleneck is dynamic memory allocation, since that's _O(n)_ when fragmented. Objects in Zippy are constructed entirely from fixed-size nodes, and can therefore be allocated from a slab allocator, which is _O(1)_.

Therefore, it uses a minimum of memory: (linked) lists and (binary tree) records have a factor of 2 overhead (to represent _n_ children, they need _2n_ fixed-size slabs in the slab allocator). Memory use is traded for indirection, but the pointer-hopping is all local, within a contiguous, preallocated buffer.

## Compilation:

```bash
cd `git rev-parse --show-toplevel`
gcc --std=c89 -O3 -Wall -I include src/*.c tests/test-data.c -o tmp-tests  &&  ./tmp-tests
```

(C89 for portability.)

Some compilation variables:

- `ZIPPY_POINTER_SIZE`, which can be 16, 32 (default), or 64.
- `ZIPPY_REFCNT_SIZE`, which can be 8 (default), 16, 32, or 64.
- `ZIPPY_REFCNT_ROLLOVER`, which makes makes nodes copy, rather than fail, when they reach a maximum reference count.

## Performance tests:

- Zippy takes 10 seconds (elapsed) to do a fragmentation-heavy workflow. (12 seconds with `-DZIPPY_REFCNT_SIZE=8 -DZIPPY_REFCNT_ROLLOVER`.)

```bash
gcc --std=c99 -O3 -Wall -DZIPPY_REFCNT_SIZE=32 -I include src/*.c tests/perf_records.c -o tmp-tests

systemd-run --user --scope -p MemoryMax=4M -p MemorySwapMax=4M time ./tmp-tests
```

- Python 3.11 takes 80 seconds (elapsed), using frozen dataclasses with slots and avoiding any Python code execution (so we're just comparing data management). Also needs a bit more memory.

```bash
g++ --std=c++11 -O3 -Wall tests/perf_records_python.cpp -shared -fPIC $(python -m pybind11 --includes) -o testy$(python3-config --extension-suffix)

systemd-run --user --scope -p MemoryMax=7M -p MemorySwapMax=7M time python -c 'import testy; testy.run()'
```

But without memory constraints (so that Python never garbage-collects), Python takes 20 seconds.

Interestingly enough, Python can run in 30 seconds with less memory using conventional classes (I changed the code: check the source for its current state).

```bash
# recompile with conventional classes

systemd-run --user --scope -p MemoryMax=6M -p MemorySwapMax=6M time python -c 'import testy; testy.run()'
```

- C++ with dynamically allocated, polymorphic `std::vector` takes 0.600 seconds. (`-O0`: 1.300 seconds, `-O1`: 0.650 seconds, `-O2`: 0.600 seconds, `-O3`: 0.600 seconds.)

```bash
g++ --std=c++11 -O3 -Wall tests/perf_records_stdcpp.cpp -o tmp-tests

systemd-run --user --scope -p MemoryMax=4M -p MemorySwapMax=4M time ./tmp-tests
```

So although object-building can be 2× to 8× faster than Python, it's still 16× slower than C++. Admittedly, the C++ can optimize around known types (the elements of the `std::vector` can only be one of 4 predefined classes), so it's not dynamic the way that Zippy and Python are, but the techniques attempted here—no garbage collection and exclusive reliance (in this test) on the slab allocator—give us data management that's still much closer to Python than it is to compiled code.

## Memory size

Zippy's records use at most `8 + n*2*8` bytes for `n` fields. Each fixed-size slot is 8 bytes (for 4-byte pointers). The first slot points to the class object and the data (binary) tree. The data tree has field values at the leaves and indirection in the nodes so that lookup is _O(log n)_. Since a (full) binary tree has one fewer internal nodes than leaves, this is a factor of 2 overhead.

Python's `dataclass(frozen=True, slots=True)` uses `32 + n*8` bytes for `n` fields. Clearly Python is optimizing this for memory use, and the memory is dynamic (size exactly fits the required number of pointers: 8 bytes per pointer). Good job Python!

Python's conventional classes use `56 + 288` for at least the first 6 fields. (It's a hashmap; the size jumps when it can't get any more full.)

Zippy's lists are just linked lists, so there's again a factor of 2 overhead (half of each slab has a value and the other half points to the next node). Unlike records, there's no constant term.

Python's lists are like `std::vector`: reallocate and copy when a preallocated buffer is full. The scaling is `56 + n*8` bytes (56 bytes of overhead, then 8-byte pointers).

Naturally, C++ uses exactly as much memory as is needed—no overhead (except for the vtable in the polymorphis I used in the test, which in principle could be as small as 2 bits because there were only 4 classes).

## Conclusion

Developing programming language data around a slab allocator seemed like a great idea, but we don't actually gain much in practice. The lesson for me is either that `malloc` is sufficiently well-tuned that its in-principle _O(n)_ allocation/free doesn't matter much or that the indirection and reference counting _does_ matter, even though it's all close to the CPU (L2).
