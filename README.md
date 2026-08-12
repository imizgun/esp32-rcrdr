# esp32-rcrdr

## Requirements

- [Nix](https://nixos.org/) with flakes enabled, and `direnv` (or run `nix develop` manually)
- An ESP32-based board connected over USB

## Environment

The ESP-IDF environment is defined in `flake.nix`. With `direnv`:

```sh
direnv allow
```

Or enter the environment manually:

```sh
nix develop
```

## Build

### With `idf.py`

```sh
idf.py build
```

### With CMake directly

`idf.py` is a wrapper around CMake + Ninja; the same build can be run without it:

```sh
mkdir -p build
cmake -B build -G Ninja -DIDF_TARGET=esp32 .
cmake --build build
```

## Flash

```sh
idf.py -p /dev/ttyUSB0 flash
```

Or, from a CMake build:

```sh
cmake --build build --target flash
```

The port may differ (e.g. `/dev/ttyACM0`) — list available ports with:

```sh
idf.py -p PORT flash monitor
```

## Serial monitor

```sh
idf.py -p /dev/ttyUSB0 monitor
```

Exit the monitor with `Ctrl+]`.
