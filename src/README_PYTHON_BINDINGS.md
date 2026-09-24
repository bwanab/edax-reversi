# Python Bindings for Edax

This directory contains Python bindings for the Edax Reversi engine.

## Files

- `edax_wrapper.h` - C API header for Python bindings
- `edax_wrapper.c` - C implementation providing Python-friendly interface
- `all_lib.c` - Build configuration for shared library (excludes main.c)

## Building

### macOS
```bash
clang -std=c17 -O3 -march=native -mdynamic-no-pic \
      -D_GNU_SOURCE=1 -DNDEBUG -dynamiclib \
      -o ../bin/libedax.dylib all_lib.c -lm
```

### Linux
```bash
clang -std=c17 -O3 -march=native -D_GNU_SOURCE=1 -DNDEBUG \
      -shared -fPIC -o ../bin/libedax.so all_lib.c -lm
```

## Output

- `../bin/libedax.dylib` (macOS) or `../bin/libedax.so` (Linux)
- Size: ~555KB
- Exports: `edax_create`, `edax_destroy`, `edax_get_move`, `edax_get_score`, `edax_get_nodes`

## API

```c
// Create engine with search depth
EdaxEngine* edax_create(int level);

// Get best move for position (stateless)
// Returns 0-63, or -1 if no legal moves
int edax_get_move(EdaxEngine *engine, uint64_t player_bits, uint64_t opponent_bits);

// Get evaluation score (centidisks)
int edax_get_score(EdaxEngine *engine);

// Get nodes searched
uint64_t edax_get_nodes(EdaxEngine *engine);

// Clean up
void edax_destroy(EdaxEngine *engine);
```

## Configuration

The wrapper automatically:
- Initializes all Edax subsystems (edge stability, eval weights, search tables)
- Loads eval.dat from standard locations
- Silences verbose search output (options.noise = 100, verbosity = 0)
- Sets up clean stateless operation (no move history required)

## Usage from Python

See `ReversiSB3/util/edax_engine.py` for Python ctypes bindings.

## Notes

- The wrapper provides a stateless API - just pass the current board position
- No GTP protocol overhead (~1000x faster than subprocess approach)
- Thread-safe: each EdaxEngine instance is independent
- Silent by default: no search output during operation
