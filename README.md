# stasis

Freeze the screen of a Wayland compositor until a provided command exits.

> [!IMPORTANT]
> Make sure the command you provide gives you a way to quit other than hitting
> `Ctrl+C` as it won't work because all input will effectively be blocked and
> with no way to quit you'll have to switch to a TTY to kill it (e.g.
> `Ctrl+Alt+F5`, login, `pkill stasis`).

Works great with [grim](https://gitlab.freedesktop.org/emersion/grim) and
[slurp](https://github.com/emersion/slurp):

```sh
stasis -c 'slurp -d | grim -g- -'
```

As well as with [swappy](https://github.com/jtheoof/swappy) if you want to
annotate your screenshot right away:

```sh
stasis -c 'slurp -d | grim -g- -l0 -' | swappy -f -
```

Add `-p` if you want to include a **p**ointer (a cursor, but, well, `-c` is
already taken by **c**ommand and looks similar to `sh -c`) on a frozen
screenshot:

```sh
stasis -p -c 'slurp -d | grim -g- -' | swappy -f -
```

## Building

Setup using meson (once):

```sh
meson setup --buildtype release build
```

Compile:

```sh
meson compile -C build
```
