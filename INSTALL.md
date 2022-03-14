Installation
------------

### Linux / macOS

If you do not care about build options, simply run:
```sh
make
```
in Terminal.

If you would like to customize build options, follow the below procedure:
```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release [OPTIONS]
make -j 4 install
```

### Windows

If you do not care about build options, simply run:
```sh
make.bat
```
in DOS prompt.

If you would like to customize build options, follow the below procedure:
```sh
mkdir build
cd build
cmake .. [OPTIONS]
MSBuild -maxcpucount:4 /p:Configuration=Release INSTALL.vcxproj
```
