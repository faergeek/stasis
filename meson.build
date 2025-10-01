project(
    'stasis',
    'c',
    version: '0.0.1',
    license: 'MIT',
    meson_version: '>=1.8.0',
    default_options: ['c_std=c23', 'warning_level=3', 'werror=true'],
)

add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: 'c')

wayland = import('wayland')

executable(
    'stasis',
    install: true,
    sources: [
        'main.c',
        wayland.scan_xml(wayland.find_protocol('xdg-shell')),
        wayland.scan_xml('wlr-layer-shell-unstable-v1.xml'),
        wayland.scan_xml('wlr-screencopy-unstable-v1.xml'),
    ],
    dependencies: [dependency('wayland-client')],
)
