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
Because of some misconfigurations,
the OpenCL package config provided by the opencl-intel module cannot be used. Instead, the package
config included in the das5/ directory should be used when compiling the client:
```
$ meson .. -Dpkg_config_path=../das5
$ ninja echidna-client
```
This will link to Intels OpenCL ICD loader and pick up CPU devices by default. To _also_ use Nvidia GPUs, one must simply load the cuda11.1 module:
```
$ module load cuda11.1
```
Note that the Intel OpenCL ICD loader is also capable of loading the nvidia ICD driver, and the above command makes the nvidia implementation visible to the Intel OpenCL ICD loader. The reverse case is not true.
