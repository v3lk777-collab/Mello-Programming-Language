<p align="center">
    <img src="./logo.png" width="20%" alt="Mello Programming Language Logo"/>
</p>

<h1 align="center">Mello Programming Language</h1>

<p align="center">An OOP, high-performance, and indentation-based programming language and transpiler for streamlining embedded systems and IoT development.</p>

<p align="center">Mello compiles directly to native C++ — no runtime interpreter, no virtual machine. Write readable, Python-like code and get the exact binary footprint of hand-written Arduino C++.</p>

<p align="center">
    <img src="https://img.shields.io/badge/C%2B%2B-23-blue.svg" alt="C++23"/>
    <img src="https://img.shields.io/badge/CMake-3.25%2B-informational.svg" alt="CMake v3.25+"/>
    <img src="https://img.shields.io/badge/Python-3-informational.svg" alt="Python v3"/>
</p>

---

## Table of Contents

1.  [The Vision & Problem Statement](#1-the-vision--problem-statement)
2.  [Core Engineering Features](#2-core-engineering-features)
3.  [Under the Hood: Compiler Architecture](#3-under-the-hood-compiler-architecture)
4.  [Language Syntax & Reference](#4-language-syntax--reference)
5.  [Code Example](#5-code-example)
6.  [Getting Started](#6-getting-started)
7.  [Performance Benchmarks](#7-performance-benchmarks)
8.  [The Mello IDE](#8-the-mello-ide)
9.  [STEAM Evaluation Criteria](#9-steam-evaluation-criteria)
10. [Intellectual Property & Usage Policy](#10-intellectual-property--usage-policy)

---

## 1. The Vision & Problem Statement

Programming microcontrollers typically forces a choice between two extremes:

- **C/C++ (Arduino IDE):** Extremely fast and memory-efficient, but suffers from a steep learning curve, verbose syntax (semicolons, brackets), and manual hardware timer/debounce management.
- **MicroPython (v1.29.0) / CircuitPython (10.x / 11.x alpha):** Beautiful, readable syntax, but requires a resident runtime VM — 250,000+ bytes of Flash and 16,000+ bytes of RAM just to boot — which is simply incompatible with lower-end microcontrollers like the ATmega328P.

**The Mello Hypothesis:** By parsing an indentation-based, highly readable language into an Abstract Syntax Tree (AST) and transpiling it directly into strictly optimized, static C++ code *before* compilation, it's possible to achieve **100% of the native C++ performance footprint** while cutting code verbosity by **up to 46%**.

---

## 2. Core Engineering Features

- **Zero-Overhead Transpilation:** Mello code is converted into pure C++ at compile time. There is no runtime interpreter eating up RAM or CPU cycles.
- **Smart Micro-Threading (`every`):** Say goodbye to the blocking `delay()`. The `every` keyword automatically generates a non-blocking, `millis()`-based timer, allowing true cooperative multitasking.
- **Event-Driven Hardware (`on_press`):** Button debouncing is handled natively — the compiler expands a single line into an edge-detected, cooldown-based debounce check, with no hand-written state variables.
- **Auto-Configuration:** No manual `pinMode()` or `Serial.begin()` calls. The semantic analyzer scans the usage graph — every `turn_on()`, `toggle()`, `on_press`, and `serial.*` call — and generates the correct `setup()` routine automatically.
- **Smart Type Inference:** Every variable is assigned the tightest native C++ type at transpile time (`uint8_t` for small integers, `float` for decimals, `const char*` for text), and any variable that's never reassigned is automatically marked `constexpr`.
- **Indentation-Based:** Clean, Pythonic syntax with no curly braces `{}` or semicolons `;`.

---

## 3. Under the Hood: Compiler Architecture

The Mello compiler, built from scratch in C++23 and CMake 3.25, operates in five sophisticated phases:

### Phase A: Lexical Analysis (Lexer)
The `Lexer` scans the `.mello` file character by character, handling Python-like indent/dedent tracking via a stack-based algorithm, and tokenizes all 36 core keywords: `start`, `loop`, `use`, `wait`, `turn_on`, `turn_off`, `if`, `elif`, `else`, `write`, `read`, `scale`, `fn`, `return`, `and`, `or`, `not`, `every`, `while`, `for`, `repeat`, `on_press`, `toggle`, `break`, `continue`, `range`, `in`, `pass`, `save_memory`, `read_memory`, `millis`, `wait_ms`, `pin_mode`, `module`, `class`, `pulse_in`, `serial.*` family.

### Phase B: Syntax Analysis (Parser)
The `Parser` consumes the token stream and validates it against Mello's grammar using recursive-descent parsing, constructing a full Abstract Syntax Tree (AST) with properly nested logical blocks.

### Phase C: Semantic Analysis & Auto-Routing
During AST traversal, Mello resolves variable scoping and detects auto-pin assignments. A pin referenced by `turn_on()`, `turn_off()`, or `toggle()` is added to the `outputPins` set; a pin referenced by `on_press` is added to `inputPins` — both are used to auto-generate the correct `pinMode()` calls in `setup()`.

### Phase D: Code Generation (Transpiler)
Every AST node (`OnPressNode`, `EveryNode`, `VarAssignNode`, ...) implements its own `toCpp()` method, recursively emitting optimized, static C++ — no dynamic allocation, no hidden runtime cost.

### Phase E: Automated Pipeline
Mello runs `clang-format` (Google style) on the generated source, invokes `arduino-cli` against the `arduino:avr:uno`, `arduino:avr:nano`, or `esp32:esp32:esp32` board profile with `-O3 -flto` optimization to produce a `.hex` / `.bin` binary, and can flash it directly to a connected board by passing `--upload`.

---

## 4. Language Syntax & Reference

Mello is designed to be intuitive and highly readable.

### Variables, Arrays & Data Handling
Variables are dynamically inferred at transpile time but become strictly typed, native C++ in the output.

```python
name = "Mohammed"      # String variable
sensorPin = 5          # Integer variable
threshold = 10.5       # Float variable
isAMelloCode = true    # Boolean variable
character = 'A'        # Char variable (single quotes only)
PIN = 13               # Constant integer variable

array = [0, 1, 3, 4]   # Array
```

### Main Execution Blocks
Every hardware program requires a starting point and a continuous loop.

```python
fn start():
    serial.println("System Initialized and Ready.")

fn loop():
    # Your continuous logic goes here
```

### Hardware I/O Abstraction
Interacting with pins is simplified to natural-language commands.

```python
pin = 13
sensorPin = A0

fn loop():
    turn_on(pin)                # -> digitalWrite(pin, HIGH);
    wait(1s)                    # -> delay(1000);
    turn_off(pin)               # -> digitalWrite(pin, LOW);
    wait(1s)                    # -> delay(1000);

    value = read(sensorPin)     # "A"-prefixed pin -> analogRead(); otherwise digitalRead()
```

Or use the built-in `toggle` function to blink a pin — it keeps its own internal state and flips between `HIGH` and `LOW` on every call:

```python
PIN = 13                        # Create a constant var

fn loop():
    toggle(pin)                 # Flips HIGH/LOW every call via its own static flag
    wait(1s)
```

Other hardware and serial primitives:

```python
pin = 13
character = 'A'
value = 50

fn start():
    write(pin, HIGH)                # 0/1/HIGH/LOW -> digitalWrite(); any other value -> analogWrite()
    serial.write(character)         # Single argument -> Serial.write()
    scale(value, 0, 1023, 0, 255)   # -> map(value, 0, 1023, 0, 255)
```

### Advanced Event-Driven Structures
Mello shines in handling hardware events without blocking the CPU execution thread.

**The `every` loop (non-blocking timer):**

```python
pin = 13

fn loop():
    every 1s:
        turn_on(pin)
        serial.println("1 second passed, and the CPU wasn't blocked!")
```

**The `on_press` event (auto-debounced button):**

```python
buttonPin = 2

fn loop():
    on_press buttonPin:
        serial.println("Button on Pin 2 was pressed safely without bouncing.")
```

### Control Flow
Standard logical operators and three loop constructs are fully supported.

```python
LED_PIN = 13
sensorPin = A0

i = 0

fn loop():
    sensor_value = read(sensorPin)

    if sensor_value > 50:
        turn_on(LED_PIN)
    elif sensor_value == 50:
        serial.print("Stable")
    else:
        turn_off(LED_PIN)

    repeat 5:
        serial.println("This runs exactly five times")

    while is_active == 1:
        wait(100)               # Standard blocking delay if absolutely needed

    for i < 10:                 # Direction (++ / --) inferred from the comparison operator
        serial.println(i)
        i++

    for x in range(10):         # Equivalent to for x < 10, but with an explicit counter
        serial.println(x)

    for y in range(2, 10, 2):   # start, stop, and step are all customizable
        serial.println(y)
```

### Custom Functions
Define reusable blocks of code easily with the `fn` keyword.

```python
PIN = 13

fn blink_fast(pin_num):
    toggle(pin_num)
    wait(100)

fn loop():
    blink_fast(PIN)
```

You can explicitly specify a function's return type using the `->` return-type arrow. Use void for functions that do not return a value.

```python
PIN = 13

fn blink(led_pin) -> void:
    toggle(led_pin)
    wait(1s)

fn loop() -> void:
    blink(PIN)
```

### Break / Continue Control Transfer Statements
Use `break` and `continue` in while / for loops

```python
counter = 0

fn start():
    while counter <= 100:
        if counter == 50:
            break
        
        counter++
```

### EEPROM

```python
fn start():
    save_memory("number", 5)
    number = read_memory("number")

    serial.println(number)
```

### OOP (Classes)

```python
class Animal:
    fn sound(self, foo):
        serial.print(foo)

class Dog(Animal):
    fn sound(self, boo):
        serial.print(boo)

class Cat(Animal):
    fn sound(self, meow):
        serial.print(meow)

myDog = Dog()
myCat = Cat()

fn start():
    myDog.sound("Haw Haw Haw")
    myCat.sound("Meow Meow Meow")
```

### Standard Library (`use`)

Mello ships with a standard library written in Mello itself. Import a module with `use`:

```python
use math
use random

fn start():
    random.seed_random(42)

    c = math.clamp(15, 0, 10)
    randomNumber = random.random_range(1, 6)

    serial.println(c)              # 10
    serial.println(randomNumber)   # pseudo-random number from 1 to 6
```

#### `math`

| Function | Description |
|---|---|
| `absolute(x)` | Absolute value |
| `min_value(a, b)` / `max_value(a, b)` | Smaller / larger of two values |
| `clamp(value, min, max)` | Limit `value` to the range `[min, max]` |
| `square(x)` / `cube(x)` | `x²` / `x³` |
| `power(base, exponent)` | `base` raised to a non-negative integer exponent |
| `factorial(n)` | `n!` (returns `1.0` for `n <= 1`) |
| `lerp(a, b, t)` | Linear interpolation between `a` and `b` |
| `degrees_to_radians(v)` / `radians_to_degrees(v)` | Angle conversion |
| `isclose(a, b, epsilon)` | `true` if `a` and `b` differ by at most `epsilon` |
| `sin(x)` | Get the sine of x |
| `cos(x)` | Get the cosine of x |
| `tan(x)` | Get the tan of x |
| `sqrt(x)` | Get the sqrt of x |

#### `random`

A small, deterministic linear congruential generator (state in `0..4095`, light enough for an ATmega328P).

| Function | Description |
|---|---|
| `seed_random(seed)` | Set the generator state |
| `random_next()` | Next raw value in `0..4095` |
| `random_below(max_exclusive)` | Value in `0 .. max_exclusive-1` (returns `0` if `max_exclusive <= 0`) |
| `random_range(low, high)` | Value in `low .. high` inclusive (returns `low` if `high < low`) |

### Standard Protocols (`use`)

#### `i2c` Protocol

A software I2C (Bit-Banging) protocol written natively in Mello.

| Function | Description |
|---|---|
| `begin(sda_pin, scl_pin)` | Sends the I2C Start condition (pulls SDA low while SCL is high) |
| `stop()` | Sends the I2C Stop condition (releases SDA while SCL is high) |
| `write_byte(byte_val)` | Transmits 8 bits MSB-first and returns `1` if ACK is received, or `0` if NACK |
| `read_byte(send_ack)` | Reads 8 bits MSB-first and sends an ACK (`1`) or NACK (`0`) bit |

### Libraries (`use`)

#### `lcd`

An I2C character LCD driver (`LCD(sda, scl, lcd_rows, lcd_columns)`) for 16×2 and 20×4 displays via PCF8574.

| Method | Description |
|---|---|
| `print(text)` | Prints text at the current cursor position with automatic line wrapping |
| `println(text)` | Prints text and moves the cursor to the beginning of the next row |
| `set_cursor(col, row)` | Moves the cursor to a specific column and row |
| `clear()` / `home()` | Clears the display or resets the cursor to `(0, 0)` |
| `backlight_on()` / `backlight_off()` | Turns the LCD backlight on or off |

#### `oled`

A 128×64 I2C OLED display driver (`SSD1306(sda, scl)`) with a built-in 5×7 ASCII font.

| Method | Description |
|---|---|
| `print(text)` / `println(text)` | Prints text with automatic cursor tracking and page wrapping |
| `draw_char(c, column, page)` | Draws a single ASCII character at a specific column (`0..122`) and page (`0..7`) |
| `draw_line(page)` | Draws a horizontal divider line across the specified page (`0..7`) |
| `set_cursor(page, column)` | Sets the active page (`0..7`) and column (`0..127`) |
| `clear()` | Clears the entire display buffer and resets the cursor to `(0, 0)` |
| `on()` / `off()` | Turns the OLED panel on or off |
| `invert(enabled)` | Inverts display colors (`1` for inverted, `0` for normal) |
| `set_contrast(value)` | Adjusts screen brightness/contrast (`0..255`) |

#### `servo`

A software PWM servo motor driver (`Servo(pin)`).

| Method | Description |
|---|---|
| `write(angle)` | Clamps `angle` to `0..180` degrees and drives the servo to that position |
| `refresh(angle)` | Sends a single PWM refresh pulse for the given `angle` (`0..180`) |
| `read_angle()` | Returns the last written servo angle (`int`) |

#### `ultrasonic`

An HC-SR04 ultrasonic distance sensor driver (`Ultrasonic(trig, echo)`).

| Method | Description |
|---|---|
| `distance_cm()` | Measures distance in centimeters (`float`), or returns `-1.0` on timeout |
| `distance_inch()` | Measures distance in inches (`float`), or returns `-1.0` on timeout |
| `is_closer_than(limit_cm)` | Returns `true` if an object is detected within `limit_cm` centimeters |

#### `dht`

A temperature and humidity sensor driver (`DHT(pin, sensor_type)`) supporting both **DHT11** (`11`) and **DHT22** (`22`) single-wire protocols.

| Method | Description |
| :--- | :--- |
| `read_sensor()` | Polls the sensor, verifies the 40-bit checksum, and returns `true` on success or `false` on timeout/error |
| `get_temperature()` | Returns the last measured temperature in degrees Celsius (`float`) |
| `get_humidity()` | Returns the last measured relative humidity percentage (`float`) |

#### `motor`

A dual-driver motor control library providing `DCMotor(in1, in2, en)` for H-bridge drivers (L298N/L293D) and `StepperMotor(p1, p2, p3, p4)` for 4-wire stepper motors.

| Method | Description |
| :--- | :--- |
| `forward(speed)` | *(DCMotor)* Drives the motor forward at a clamped PWM speed (`0..255`) |
| `backward(speed)` | *(DCMotor)* Drives the motor in reverse at a clamped PWM speed (`0..255`) |
| `stop()` | *(DCMotor)* Coasts the motor to a stop by cutting power to all pins |
| `brake()` | *(DCMotor)* Actively brakes the motor by shorting both motor terminals |
| `set_speed_delay(delay_ms)` | *(StepperMotor)* Sets the inter-step delay in milliseconds (`> 1`) |
| `step(steps)` | *(StepperMotor)* Rotates the motor by a signed number of steps (positive for CW, negative for CCW) |
| `release()` | *(StepperMotor)* De-energizes all coils to prevent idle heating |

#### `mpu6050`

A 6-axis accelerometer and gyroscope driver (`MPU6050(sda, scl)`) built natively on top of Mello's `i2c` and `math` modules.

| Method | Description |
| :--- | :--- |
| `wake_up()` | Wakes the MPU6050 from sleep mode by clearing the `PWR_MGMT_1` register |
| `update()` | Reads and scales all 14 accelerometer and gyroscope bytes in a single I2C burst |
| `get_accel_x()` / `get_accel_y()` / `get_accel_z()` | Returns linear acceleration in **g** (`±2g` default range) |
| `get_gyro_x()` / `get_gyro_y()` / `get_gyro_z()` | Returns angular velocity in **degrees/sec** (`±250°/s` default range) |
| `get_tilt_x()` / `get_tilt_y()` | Returns approximate pitch and roll tilt angles in degrees (`-90.0` to `+90.0`) |

---

## 5. Code Example

A practical example showcasing multi-tasking and event handling in a Smart Room Controller — a non-blocking sensor poll running alongside a debounced button listener, neither one blocking the other.

```python
# Mello Smart Room Example
buttonPin = 2
lightPin = 13
tempSensor = A0
systemActive = true

fn start():
    serial.println("Smart Room OS Booting...")

fn loop():
    # Read temperature every 5 seconds without blocking the button
    every 5s:
        temp = read(tempSensor)
        serial.print("Current Temp: ")
        serial.println(temp)

    # Listen for button press to toggle the system
    on_press buttonPin:
        if systemActive == true:
            systemActive = false
            turn_off(lightPin)
            serial.println("System Deactivated")
        else:
            systemActive = true
            turn_on(lightPin)
            serial.println("System Activated")
```

---

## 6. Getting Started

### Option A — Prebuilt Windows Binary

A prebuilt `mello.exe` is committed directly to the repository at `cmake/mello.exe`, so Windows users can skip building entirely:

```bash
git clone https://github.com/v3lk777-collab/Mello-Programming-Language.git
cd Mello-Programming-Language/cmake
```

### Option B — Build from Source

The compiler is built with CMake 3.25 or later and requires a C++23 toolchain (Linux, macOS, or Windows).

```bash
git clone https://github.com/v3lk777-collab/Mello-Programming-Language.git
cd Mello-Programming-Language

mkdir cmake && cd cmake
cmake ..
cmake --build .
```

---

### Run Mello programming language

To compile the code

```bash
mello.exe path\to\main.mello
```

To compile and upload it to the board

```bash
mello.exe path\to\main.mello --upload
```

Supported board targets: `uno` (default), `nano`, and `esp32`

```bash
mello.exe path\to\main.mello nano --upload
```

```bash
mello.exe path\to\main.mello uno --upload
```

```bash
mello.exe path\to\main.mello esp32 --upload
```

If you only want to transpile without running the compiler backend

```bash
mello.exe path\to\main.mello --no-compile
```

If you want to keep the generated C++ sketch file after compilation use

```bash
mello.exe path\to\main.mello --save-code
```

If you don't want to see the generated C++ code use

```bash
mello.exe path\to\main.mello --no-output
```

> Prefer a dedicated editor? See [The Mello IDE](#-8-the-mello-ide) below.

---

## 7. Performance Benchmarks

Mello has been rigorously tested against standard alternatives on an ATmega328P:

| Framework | Flash | RAM | Compatibility |
|---|---|---|---|
| **Mello** | ~900 Bytes | 9 Bytes | Full 8-bit/32-bit parity |
| **Native Arduino C++** | ~900 Bytes | 9 Bytes | Full 8-bit/32-bit parity |
| **MicroPython VM (v1.29.0)** | > 250,000 Bytes | > 16,000 Bytes | Incompatible — fails to initialize |

- **Lines of Code (LoC):** Mello reduces required lines of code by **40–60%** compared to native Arduino C++, primarily through automatic timer state, debounce state, and `setup()` boilerplate.
- **Native Binary Output:** Because Mello transpiles straight to C++, its binaries are byte-for-byte comparable to hand-written Arduino code — there's no interpreter tax to pay.

---

## 8. The Mello IDE

A dedicated desktop IDE is in active development at **[Mello-IDE](https://github.com/v3lk777-collab/Mello-IDE)** (under development) — a native companion app for writing, managing, and compiling `.mello` files without a bare terminal.

- **Native desktop shell:** built with [Tauri](https://tauri.app) — a Rust backend paired with a TypeScript + Vite front-end, keeping the packaged app far lighter than a full Electron/Chromium bundle.
- **Bundled compiler:** the same CMake-built C++ compiler documented above ships directly with the IDE, so compiling is a local, offline operation.
- **Status:** early-stage, actively developed, not yet a versioned release.

---

## 9. STEAM Evaluation Criteria

This project is explicitly designed to meet high-level engineering evaluation standards:

- **Systems Engineering:** demonstrates a complete compiler architecture — lexical analysis, AST construction, semantic auto-routing, and native code generation.
- **Efficiency:** solves a real computational bottleneck — expressive syntax on hardware with kilobytes, not gigabytes, of memory.
- **Innovation:** introduces novel paradigms (`every`, `on_press`, auto-pin assignment, auto-`const` type inference) to hardware programming without sacrificing native performance.

---

## 10. Intellectual Property & Usage Policy

**© 2026 / 2027 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.**

Mello Programming Language and the Mello IDE are original works developed for scientific research and educational purposes.

- **Usage:** you are free to view, study, use Mello in embedded systems like Arduino and ESP32, and learn from this codebase.
- **Restrictions:** unauthorized use of this source code in any academic competition (e.g., ISEF, science fairs), research submission, or commercial product is **strictly prohibited** without prior written consent from the author.
- **Attribution:** if you find this project useful for learning, please attribute the work to the original author, Mohammed Tamer Mohammed Ahmed El-Azab.

*Interested in collaborating or seeking permission for specific use? Please reach out directly (v3lk777@gmail.com).* 