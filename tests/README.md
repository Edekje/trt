# Testing Methodology:
- Each header/src file `foo` has its own unit test named `foo_test`.
- Unit tests consist of two (three) files:
    - A `foo_test.cpp` file with a main function, which produces expected output.
    - A corresponding bash file `foo_test.sh` which performs the test.
    - ( The compiled `foo_test.cpp` is stored in `builds/foo_test` ).
- Any additional files related to a test are named `foo_example_bar`.
- Execute all tests by executing the test script like so:
```
./test
```
- Execute a specific test, such as that of the module `config` like so:
```
./test config_test
```
- Execute test number 2 from config:
```
./test config_test 2
```
- Execute test number 2 from config, and view the given input and received output:
```
./test config_test 2 v
```

