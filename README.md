# still

Freeze the screen of a Wayland compositor until a provided command exits.

> [!IMPORTANT]
> Make sure the command you provide gives you a way to quit other than hitting
> `Ctrl+C` as it won't work because all input will effectively be blocked and
> with no way to quit you'll have to switch to a TTY to kill it (e.g.
> `Ctrl+Alt+F5`, login, `pkill still`).

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

## Building

Make sure you have [meson
installed](https://mesonbuild.com/Getting-meson.html) and then:

```sh
meson setup --buildtype release build
meson compile -C build
```
