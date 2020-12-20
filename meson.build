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
    'src/utils/threadpool.cpp'
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
    'src/client/renderqueue.cpp'
]

client_test_sources = [
    'src/client/client.cpp',
    'src/client/clienttest.cpp',
    'src/client/renderqueue.cpp'
]

add_project_arguments(['-DCL_TARGET_OPENCL_VERSION=120'], language: 'cpp')

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
    sources + ['src/cli/main.cpp'],
    install: true,
    build_by_default: false,
    include_directories: include,
    dependencies: dependency('threads')
)

executable(
    'echidna-client-test',
    sources + client_test_sources,
    install: true,
    build_by_default: false,
    include_directories: include,
    dependencies: dependency('threads')
)