---
applyTo: '**'
description: 'description'
---
Provide project context and coding guidelines that AI should follow when generating code, answering questions, or reviewing changes.

# Copilot Instructions for MLA Project

## Test File Structure

When creating test files for this project, follow these conventions:

### File Naming
- Test files should end with `_test.h`
- Place test files in `lib/core-os-test/` directory

### Required Includes
```cpp
#include "../core-os/utils/[module_name].h"
#include "../core-os-test-support/mla_test_executor.h"
```

### Test Function Format

- Use inline void functions for each test
- Name format: [Module][Function]Test()
- Use assert_true() for assertions with descriptive messages
- Test known values with tolerance ranges (e.g., > 0.99999 && < 1.00001)

### Test Registration Function

- Create a Register[Module]Tests(mla_test_executor_t &p_TestExecutor) function
- Use mla_test() to create test objects with: name, test_category, and test function
- Register each test with mla_test_executor_register_test(p_TestExecutor, test)

### Available Assertions

All assertions take a message parameter as the last argument:
- assert_fail(message) - Force test failure with message
- assert_true(condition, message) - Assert condition is true
- assert_false(condition, message) - Assert condition is false
- assert_equal(actual, expected, message) - Assert values are equal
- assert_struct_equal(Type, actual, expected, message) - Assert structs are equal (requires operator!=)
- assert_not_equal(actual, expected, message) - Assert values are not equal
- assert_null(pointer, message) - Assert pointer is null
- assert_not_null(pointer, message) - Assert pointer is not null

Assertion Examples
```cpp
assert_true(result > 0, "Result should be positive");
assert_equal(value, 42, "Value should be 42");
assert_struct_equal(my_struct_t, actual, expected, "Structs should match");
assert_not_null(pointer, "Pointer should be allocated");
```

### Header Guards
Use #ifndef [FILENAME]_H format matching the filename