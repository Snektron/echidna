# Echidna

## Compiling

Echidna uses the Meson build system, and can be compiled as follows
```
$ mkdir build
$ cd build
$ meson ..
$ ninja

```

This compiled the client (echidna-client), server (echidna-server) and the cli (echidna-cli) by default.

Echidna renders multiple frames concurrently by using a slot mechanism. By default, each GPU has 2 slots. This number can be overridden at compiletime by passing `-Dconcurrent_frames=<amt>` to Meson.

## Usage

There are three basic parts to Echidna: The server, client and cli. The client represents a worker that can accept rendering tasks from the server, and the cli can be used to upload tasks to the server. The server then schedules this across the available clients in order to complete the rendering task. To set this up, the server should be started first:
```
$ ./echidna-server
```
This executable takes no parameters, and binds to 0.0.0.0:4242 by default.

Next, a set of clients should be connected to the server:
```
$ ./echidna-client host 4242
```
This will start a client on the machine the command is executed, and will make its GPUs available for render tasks. Note that the server will accept jobs without any clients connected (they will simply wait on an internal queue), and extra clients can be connected at any moment.

Finally, a job can be launched using the cli:
```
$ ./echidna-cli --host host --submit kernel.ocl --frames 100 --dim 1920x1080
```
The status of jobs scheduled by the server can also be queried using the cli:
```
$ ./echidna-cli --host host --query
```

## DAS-5 Setup

On the DAS-5, one must first set the environment appropriately. As Echidna requires a C++17 capable compiler, one must first load an appropriate GCC environment:
```
$ module load gcc/9.3.0
```

Futhermore, the project requires Ninja and Meson in order to be compiled. Meson can simply be installed via pip:
```
$ module load python/3.6.0
$ pip install --user meson
```

Ninja can be obtained as a static executable from https://ninja-build.org/.

Finally, the cuda 11 environment (which also contains the appropriate OpenCL libraries) should be loaded:
```
$ module load cuda11.1
```

### Compiling the client

Because of some misconfigurations, the default opencl-nvidia module provided by the DAS5 still points to an old installation. Instead, the package config included in the das5 directory should be used when compiling the client. By default, the head node will have the PKG_CONFIG_PATH environment variable set due to another misconfiguration to an opencl-intel installation containing an erroneous package config. This package config path needs to be removed, and then the proper package config needs to be set when building:
```
$ unset PKG_CONFIG_PATH
$ meson .. -Dpkg_config_path=../das5
$ ninja echidna-client
```
