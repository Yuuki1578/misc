# Ced, simple text editor for UNIX-like machine

## 1. Building
Make sure you had `gcc`, `binutils` and `make`

1. `git clone https://github.com/Yuuki1578/ced.git`
2. `cd ced`
3. `make check`/`make -f StaticLib.mk check`, optional dependency checking
4. `make`, if you want the static library `make -f StaticLib.mk`
5. `./build/ced` or `gcc <file> -L./build -lced`
