# Testing Methodology:
- Each header/src file `foo` has its own unit test named `foo_test`.
- Unit tests consist of two (three) files:
    - A `foo_test.cpp` file with a main function, which produces expected output.
    - A corresponding bash file `foo_test.sh` which performs the test.
    - ( The compiled `foo_test.cpp` is stored in `builds/foo_test` ).
- Any additional files related to a test are named `foo_example_bar`.
- A test may be performed by either running the corresponding test script, or executing `all_tests`, which first compiles, and then runs all tests.
