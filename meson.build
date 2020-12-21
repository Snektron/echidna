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
    'src/net/serversocket.cpp',
    'src/net/socket.cpp',
    'src/utils/log.cpp',
    'src/utils/threadpool.cpp',
    'src/utils/argparse.cpp',
]

server_sources = [
    'src/server/clienthandler.cpp',
    'src/server/clientmanager.cpp',
    'src/server/climanager.cpp',
    'src/server/job.cpp',
    'src/server/jobqueue.cpp',
    'src/server/main.cpp',
    'src/server/server.cpp',
    'src/server/task.cpp',
]

client_sources = [
    'src/client/clerror.cpp',
    'src/client/client.cpp',
    'src/client/clutil.cpp',
    'src/client/device.cpp',
    'src/client/main.cpp',
    'src/client/renderer.cpp',
    'src/client/renderqueue.cpp',
]

cli_sources = [
    'src/cli/main.cpp'
]

add_project_arguments(
    [
        '-DCL_TARGET_OPENCL_VERSION=120',
        '-DECHIDNA_CONCURRENT_FRAMES=@0@'.format(get_option('concurrent_frames')),
    ],
    language: 'cpp'
)

include = include_directories('include')

client_deps = [
    dependency('OpenCL'),
    dependency('threads'),
    subproject('lodepng').get_variable('lodepng_dep'),
]

executable(
    'echidna-server',
    sources + server_sources,
    install: true,
    build_by_default: true,
    include_directories: include,
    dependencies: dependency('threads')
)

executable(
    'echidna-client',
    sources + client_sources,
    install: true,
    build_by_default: false,
    include_directories: include,
    dependencies: client_deps
)

executable(
    'echidna-cli',
    sources + cli_sources,
    install: true,
    build_by_default: true,
    include_directories: include,
    dependencies: dependency('threads')
)
