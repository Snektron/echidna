project(
    'echidna',
    'cpp',
    version: '0.0.42-B',
    default_options: [
        'cpp_std=c++2a',
        'buildtype=debugoptimized',
        'default_library=static'
    ]
)

sources = [
    'src/net/socket.cpp',
    'src/main.cpp'
]

include = include_directories('include')

executable(
    'echidna',
    sources,
    install: true,
    build_by_default: true,
    include_directories: include
)