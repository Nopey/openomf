
# Linux

```
xbps-install unzip automake autoconf flex libtool

cmake -S. -Bbuild --toolchain=../vcpkg/scripts/buildsystems/vcpkg.cmake -G "Ninja Multi-Config"

cmake --build build && build/Debug/openomf
```

# Windows

```
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
```

