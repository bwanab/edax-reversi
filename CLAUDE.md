# Claude Code Preferences

## Project

Fork of [abulmo/edax-reversi](https://github.com/abulmo/edax-reversi) — a strong Othello/Reversi engine written in C17. This fork adds a **stateless C API shared library** for Python bindings (see [src/README_PYTHON_BINDINGS.md](src/README_PYTHON_BINDINGS.md)), consumed by the `ReversiSB3` project via ctypes.

## Repository / Remotes

- `origin` → `git@github.com:bwanab/edax-reversi.git` (my fork, push here)
- `upstream` → `https://github.com/abulmo/edax-reversi.git` (Richard Delorme's canonical repo, pull from here to sync)

To sync with upstream:
```sh
git fetch upstream
git merge upstream/master
```

## Layout

- [src/](src/) — engine sources (C17). Built by concatenation via `all.c`.
- [src/Makefile](src/Makefile) — top-level build. `make help` lists targets/archs.
- [src/all_lib.c](src/all_lib.c) — like `all.c` but excludes `main.c`; used to build the shared library for Python bindings.
- [src/edax_wrapper.c](src/edax_wrapper.c) / [src/edax_wrapper.h](src/edax_wrapper.h) — stateless C API exposed to Python.
- [bin/](bin/) — build output (gitignored). Also the runtime working directory.
- [data/](data/) — `eval.dat` (eval weights) and `book.dat` (opening book). Gitignored; must be fetched separately.
- [problem/](problem/) — solver benchmark suites (.obf files).

## Build

**Prerequisites**: `clang` (default), `make`. macOS: Xcode CLT. Linux: standard build tools.

### Engine binary (macOS, Apple Silicon)
```sh
mkdir -p bin
cd src
make build ARCH=native OS=osx        # fast: plain -O3 build
# or, for max speed (~2x slower to build):
make pgo-build ARCH=native OS=osx    # profile-guided optimization
cd ..
./bin/mEdax-native
```
Output binary is named `mEdax-<ARCH>` on macOS, `lEdax-<ARCH>` on Linux, `wEdax-<ARCH>.exe` on Windows.

### Engine binary (Linux)
```sh
cd src && make build ARCH=x86-64-v3 OS=linux
```

### Python-bindings shared library
See [src/README_PYTHON_BINDINGS.md](src/README_PYTHON_BINDINGS.md). Quick version:

macOS:
```sh
cd src
clang -std=c17 -O3 -march=native -mdynamic-no-pic \
      -D_GNU_SOURCE=1 -DNDEBUG -dynamiclib \
      -o ../bin/libedax.dylib all_lib.c -lm
```

Linux:
```sh
cd src
clang -std=c17 -O3 -march=native -D_GNU_SOURCE=1 -DNDEBUG \
      -shared -fPIC -o ../bin/libedax.so all_lib.c -lm
```

## Runtime data (must be downloaded — not in repo)

The engine needs `data/eval.dat` (~50 MB) to run. Get it from an upstream release:
```sh
cd data
curl -OL https://github.com/abulmo/edax-reversi/releases/download/v4.4/eval.7z
7z x eval.7z    # brew install sevenzip / apt install p7zip-full
```
Opening book (`data/book.dat`) is optional; the engine generates one on demand.

## Running

Edax expects to run **from the repo root** (paths like `data/eval.dat` are relative):
```sh
./bin/mEdax-native
```
At the prompt, `?` lists commands. `go` to have the engine play, `play e6` to make a move, `quit` to exit.

## Python integration

The Python side lives in a separate repo (`ReversiSB3`) and loads `bin/libedax.dylib` (or `.so`) via ctypes. See its `util/edax_engine.py`. The API is stateless: pass the two 64-bit bitboards (player, opponent) to `edax_get_move`.

## Notes / gotchas

- The upstream [.gitignore](.gitignore) has **unresolved merge conflict markers** checked in. Leave it alone unless doing a dedicated cleanup — many patterns still work because git tolerates the `*.o` etc. lines outside the conflict regions, but `bin/` and `data/` are inside a conflict block. If you commit generated files by accident, that's why.
- `ARCH=native` is fine for local dev; use explicit `x86-64-v3` / `armv8-a` etc. for release artifacts.
- The `mEdax`/`lEdax` binary is stateful and speaks Edax's own text protocol (plus GTP, NBoard, XBoard, GGS, Cassio). Python bindings bypass all of that.

## Commands cheat sheet

```sh
# Build engine
cd src && make build ARCH=native OS=osx && cd ..

# Build Python shared library (macOS)
cd src && clang -std=c17 -O3 -march=native -mdynamic-no-pic -D_GNU_SOURCE=1 -DNDEBUG -dynamiclib -o ../bin/libedax.dylib all_lib.c -lm && cd ..

# Run
./bin/mEdax-native

# Clean
cd src && make clean

# Sync from upstream
git fetch upstream && git merge upstream/master
```
