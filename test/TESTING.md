# QGroundControl Unit Testing

## Quick Start

```bash
# Configure and build with tests enabled
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DQGC_BUILD_TESTING=ON
cmake --build build

# Run all tests via CTest
cd build && ctest --output-on-failure

# Run specific test
./QGroundControl --unittest:ParameterManagerTest
```

## Running Tests

### Via CTest

```bash
ctest -L Unit              # Fast tests
ctest -L Integration       # Tests requiring MockLink
ctest -LE Flaky            # Exclude flaky tests
ctest --parallel $(nproc)  # Parallel execution
```

### Convenience Targets

```bash
ninja check           # All tests
ninja check-unit      # Unit tests only
ninja check-ci        # CI-safe (excludes flaky)
```

## Writing Tests

```cpp
#include "UnitTest.h"

class MyTest : public UnitTest {
    Q_OBJECT
private slots:
    void _myTestCase();
};

UT_REGISTER_TEST(MyTest)
```

### Register in CMakeLists.txt

```cmake
add_qgc_test(MyTest LABELS Unit)
add_qgc_test(MyIntegrationTest LABELS Integration SERIAL)
```

## MultiSignalSpy

```cpp
MultiSignalSpy spy;
spy.init(myObject);

// String-based API (recommended)
QVERIFY(spy.checkSignal("valueChanged"));           // Emitted exactly once
QVERIFY(spy.checkOnlySignal("valueChanged"));       // Only this signal fired
QVERIFY(spy.waitForSignal("valueChanged", 5000));   // Wait with timeout
spy.clearAllSignals();

// Mask-based API for multiple signals
QVERIFY(spy.checkSignalsByMask(spy.mask("signal1", "signal2")));
```

## Code Coverage

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DQGC_ENABLE_COVERAGE=ON
cmake --build build
ninja coverage
```

## JUnit XML Output

```bash
./QGroundControl --unittest --unittest-output:results.xml
```

## CTest Labels

| Label | Description |
|-------|-------------|
| `Unit` | Fast tests, no vehicle |
| `Integration` | Requires MockLink |
| `Slow` | Long execution time |
| `Flaky` | May occasionally fail |
