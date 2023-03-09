# Build
```
mkdir build && cd build
cmake ..
make -j8
```
# Run
Add compiled libraries into PATH
```
export LD_LIBRARY_PATH=/path/to/build/src:$LD_LIBRARY_PATH
```
Run `kernel_launcher`
```
/path/to/build/src/kernel_launcher <target executable> <program arguments>...
```
