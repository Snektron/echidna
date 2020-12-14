project(
    'echidna',
    'cpp',
    version: '0.0.42-B',
    default_options: [
        'cpp_std=c++17',
        'buildtype=debugoptimized',
    ]
)

sources = [
    'src/net/socket.cpp',
    'src/net/serversocket.cpp'
]

include = include_directories('include')

opencl_dep = dependency('OpenCL')

executable(
    'echidna',
    sources + ['src/main.cpp'],
    install: true,
    build_by_default: true,
    include_directories: include,
    dependencies: dependency('threads')
)

executable(
    'echidna-client',
    sources + ['src/client.cpp'],
    install: true,
    build_by_default: true,
    include_directories: include,
    dependencies: opencl_dep
)
