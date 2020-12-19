# Echidna

## Compiling

Echidna uses the Meson build system, and can be compiled as follows
```
$ mkdir build
$ cd build
$ meson ..
$ ninja

```

The client is not built by default, but can be compiled using its associated target
```
$ ninja echidna-client
```

## DAS-5 Setup

On the DAS-5, one must first set the environment appropriately. As Echidna requires a C++17 capable compiler, one must first load an appropriate GCC environment:
```
$ module load gcc/9.3.0
```

### Compiling the client

Because of some misconfigurations, the default opencl-nvidia module provided by the DAS5 still points to an old installation. Instead, the package config included in the das5 directory should be used when compiling the client. By default, the head node will have the PKG_CONFIG_PATH environment variable set due to another misconfiguration, to an opencl-intel installation containing an erroneous package config. This package config path needs to be removed, and then the proper package config needs to be set when building:
```
$ unset PKG_CONFIG_PATH
$ meson .. -Dpkg_config_path=../das5
$ ninja echidna-client
```
