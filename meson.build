project(
    'still',
    'c',
    version: '0.0.6',
    license: 'MIT',
    meson_version: '>=0.59.0',
    default_options: ['c_std=c11', 'warning_level=3', 'werror=true'],
)

add_project_arguments(
    ['-D_POSIX_C_SOURCE=200809L', '-Wno-unused-parameter'],
    language: 'c',
)

math = meson.get_compiler('c').find_library('m', required: false)
pixman = dependency('pixman-1')
wayland_client = dependency('wayland-client', version: '>=1.20')

wayland_scanner = find_program('wayland-scanner', native: true)

proto_c = generator(
    wayland_scanner,
    output: '@BASENAME@-protocol.c',
    arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
)

proto_h = generator(
    wayland_scanner,
    output: '@BASENAME@-client-protocol.h',
    arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
)

wayland_protocols = dependency('wayland-protocols', version: '>=1.25')
wayland_protocols_datadir = wayland_protocols.get_variable('pkgdatadir')

xdg_shell = [
    proto_c.process(wayland_protocols_datadir / 'stable/xdg-shell/xdg-shell.xml'),
]

wp_viewporter = [
    proto_c.process(
        wayland_protocols_datadir / 'stable/viewporter/viewporter.xml',
    ),
    proto_h.process(
        wayland_protocols_datadir / 'stable/viewporter/viewporter.xml',
    ),
]

layer_shell = [
    proto_c.process('wlr-layer-shell-unstable-v1.xml'),
    proto_h.process('wlr-layer-shell-unstable-v1.xml'),
]

screencopy = [
    proto_c.process('wlr-screencopy-unstable-v1.xml'),
    proto_h.process('wlr-screencopy-unstable-v1.xml'),
]

executable(
    'still',
    install: true,
    sources: [
        'array.c',
        'buffer.c',
        'capture.c',
        'image.c',
        'main.c',
        'overlay.c',
        xdg_shell,
        wp_viewporter,
        layer_shell,
        screencopy,
    ],
    dependencies: [math, pixman, wayland_client],
)

unity = subproject('unity', required: false)

if unity.found()
    unity_dep = unity.get_variable('unity_dep')

    test(
        'array',
        executable(
            'array.test',
            sources: ['array.c', 'array.test.c'],
            dependencies: [unity_dep],
        ),
    )

    test(
        'image',
        executable(
            'image.test',
            sources: ['image.c', 'image.test.c'],
            dependencies: [math, pixman, unity_dep],
        ),
    )
endif

scdoc = find_program('scdoc', required: get_option('man-pages'))

if scdoc.found()
    custom_target(
        'still.1',
        input: 'still.1.scd',
        output: 'still.1',
        command: scdoc,
        feed: true,
        capture: true,
        install: true,
        install_dir: get_option('mandir') / 'man1',
    )
endif
