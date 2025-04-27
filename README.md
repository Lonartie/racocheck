# RacoCheck

RacoCheck is a little framework designed to unit-test critical sections of code
to detect race conditions. It uses coroutines and checkpoints and runs all 
interleaving patterns of given functions and runs post-validation and invariant
checks. If any check is violated, it prints the path to the violation and the
state of the system at that point.

## Disclaimer
This is a research project and is not intended for production use. It doesn't
handle all edge cases and is not optimized for performance (it's terribly slow
for big functions). But it can give you fast approximate insights of the 
correctness of your code if used for small functions.

## Work-In-Progress
This is a work in progress and is not yet complete. The following features are
planned:
* Support for path pruning 
* Support for better printing with code lines

## Installation
It's just a simple cmake project. You can build it with:
```bash
mkdir build
cd build
cmake ..
make
```

## Usage
A running example can be found in `main.cpp`.

Creating testable methods:
```c++
#define RACO_TEST_ENABLED // <-- enables raco::fun<T> to be coroutines, 
                          // remove this to get normal functions
#include "source/raco.h"

raco::fun<int> add_5(int& v) {
   RACO_CHECKPOINT; // manual checkpoints
   int tmp = v;
   RACO_TEST(tmp = tmp + 5); // automatic checkpoints before expression
   RACO_RETURN RACO_TEST_INLINE(v = tmp); // inline checkpoints
}

raco::fun<int> sub_5(int& v) {
   RACO_TEST(int tmp = v);
   RACO_TEST(tmp = tmp - 5);
   RACO_TEST(v = tmp);
   RACO_RETURN v;
}
```
--> You don't need to checkpoint every line like I do. It's enough to checkpoint
every line that reads or writes to a shared/monitored variable.

Checking `add_5` and `sub_5` functions:
```c++
raco::check()
    .tasks([](state& s) {
        // create monitored variables
        auto& value = s.create<int>("value", 0);
        // setup functions to be tested
        return std::tuple {
            add_5(value), sub_5(value) // indexed by their order
        };
    })
    .invariant([](const state& s) {
        // check the invariant
        const auto& val = s.get<int>("value");
        return val == 0 || val == 5 || val == -5;
    })
    .post_condition([](const state& s) {
        // check the post-condition
        return s.get<int>("value") == 0;
    })
    .enable_path_pruning() // not working yet..
    .depth_limit(10) // depth limits supported, will print warnings
    .ignore_depth_limit_warning() // ignore depth limit warnings
    .deadlock_timeout(std::chrono::milliseconds(100)) // detect deadlocks via timeouts
    .info_stream_to(std::cout) // print info to cout
    .error_stream_to(std::cerr) // print errors to cerr
    .show_info(TIME_ESTIMATE) // set information levels to print
    .show_summary(PATH) // print paths in summary
    .continue_on_error() // continue on error
    .run(); // returns a boolean indicating success
```

## Example output
```c++
[ERROR]   : Post condition not satisfied!
=== STATE ===
raco::state {
  value = -5
  return #0 = 5
  return #1 = -5
}
=== PATH ===
{0,0,1,0,1,1}
```