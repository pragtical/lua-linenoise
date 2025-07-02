project('linenoise',
    ['c', 'cpp'],
    version: '0.10',
    license: 'MIT',
    default_options : ['cpp_std=c++11']
)

linenoise_src = files(
    'linenoise/ConvertUTF.cpp',
    'linenoise/wcwidth.cpp',
    'linenoise/linenoise.cpp',
    'linenoise.c',
)

linenoise_include = include_directories('linenoise')

if get_option('jit')
    lua_dep = dependency('luajit', required : true,
        default_options: ['default_library=static']
    )
else
    lua_dep = dependency('lua', required : true,
        default_options: ['default_library=static']
    )
endif

linenoise_lib = library('linenoise',
    linenoise_src,
    dependencies: lua_dep
)

linenoise_dep = declare_dependency(
    link_with: linenoise_lib,
    include_directories: linenoise_include
)
