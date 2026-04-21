project(
    'overlaylib',
    'cpp',
    version: '0.1.0',
    default_options: [
        'cpp_std=c++20',
        'warning_level=3'
    ],
)

inc = include_directories('include')

gtk_dep = dependency('gtk4')
layer_shell_dep = dependency('gtk4-layer-shell-0')

webkit_dep = dependency('webkitgtk-6.0', required: false)

executable(
    'overlaylib',
    [
        'src/main.cpp',
        'src/overlay.cpp',
    ],
    dependencies: [gtk_dep, layer_shell_dep, webkit_dep],
    include_directories: inc
)
