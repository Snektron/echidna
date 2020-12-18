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

Because of some misconfigurations, the OpenCL package config provided by the opencl-intel module cannot be used. Instead, the package config included in the das5/ directory should be used when compiling the client. By default, nodes without a GPU will have the PKG_CONFIG_PATH environment variable overridden to the erroneous OpenCL package config (which will work on CPU nodes, but not on GPU nodes, as that directory is not available on GPU nodes). This package config path needs to be removed:
```
$ unset PKG_CONFIG_PATH
$ meson .. -Dpkg_config_path=../das5
$ ninja echidna-client
```
This will link to Intel's OpenCL ICD loader and pick up CPU devices by default. To _also_ use Nvidia GPUs, one must simply load the cuda11.1 module:
```
$ module load cuda11.1
```
Note that the Intel OpenCL ICD loader is also capable of loading the Nvidia ICD driver, and the above command makes the nvidia implementation visible to the Intel OpenCL ICD loader. The reverse case is not true, and the Nvidia loader will only able to see Nvidia devices.
