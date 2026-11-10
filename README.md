# CLI Lichess client

Command Line Interface chess client for Lichess backend written on modern C++.

## Dependencies

- **LLVM** 17.0.1
- **CMake** 3.28
- **Boost**: Asio, Beast, System
- **OpenSSL** 3.6.3

## Build

```bash
mkdir build
cmake -B build
cmake --build build -j$(nproc)
```

## Game

### I. Create game token on lichess.org with rights

- *challenge:read*
- *challenge:write*
- *board:play*

### II. Create *"game-token"* file and insert token there

```
echo '<lichess-token>' > game-token
```

### III. Start game

```
./build/clichess <opponent-name> 2>/dev/pts/<tty-for-output-logs>
```
