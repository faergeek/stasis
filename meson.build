project(
    'still',
    'c',
    version: '0.0.2',
    license: 'MIT',
    meson_version: '>=0.58.0',
    default_options: ['c_std=c11', 'warning_level=3', 'werror=true'],
)

add_project_arguments(
    ['-D_POSIX_C_SOURCE=200809L', '-Wno-unused-parameter'],
    language: 'c',
)

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
    sources: ['main.c', xdg_shell, layer_shell, screencopy],
    dependencies: [dependency('wayland-client')],
)
