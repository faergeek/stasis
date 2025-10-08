# still

Freeze the screen of a Wayland compositor until a provided command exits.

> [!IMPORTANT]
> Make sure the command you provide gives you a way to quit in case it takes
> over the keyboard input. If keyboard input is not taken over and you just run
> it from the terminal you can still hit `Ctrl+C` as `still` just lets all
> input to be passed through.

Works great with [grim](https://gitlab.freedesktop.org/emersion/grim) and
[slurp](https://github.com/emersion/slurp):

```sh
still -c 'slurp | grim -g- -'
```

As well as with [swappy](https://github.com/jtheoof/swappy) if you want to
annotate your screenshot right away:

```sh
still -c 'slurp | grim -g- -' | swappy -f -
```

Add `-p` if you want to include a cursor (or **p**ointer) on a frozen
screenshot:

```sh
still -p -c 'slurp | grim -g- -' | swappy -f -
```

## Installation

### Arch Linux

There's an [AUR package](https://aur.archlinux.org/packages/still).

### Building from source

#### Install dependencies

Dependencies:

- meson
- libwayland-client
- wayland-protocols
- pixman

##### Arch Linux

```sh
sudo pacman -S meson pixman wayland wayland-protocols
```

##### Debian/Ubuntu

```sh
sudo apt-get install meson libpixman-1-dev libwayland-dev wayland-protocols
```

#### Compile

```sh
meson setup --buildtype release build
ninja -C build
```

A binary will be at `./build/still`.
