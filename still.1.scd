still(1)

# NAME

still - Freeze the screen of a Wayland compositor until a provided command exits

# SYNOPSIS

*still* [options...] -c <command>

# DESCRIPTION

still can be used to freeze the screen and run a command while it is frozen.

Make sure the command you provide gives you a way to quit in case it takes over
the keyboard input. If keyboard input is not taken over and you just run it from
the terminal you can still hit *Ctrl+C* as *still* just lets all input to be
passed through.

For example you can freeze the screen to select a region on the screen(s) with
_slurp_(1) and take a screenshot of that region with _grim_(1) like this:

*still* -c 'slurp | grim -g- -'

_swappy_(1) can also be used to annotate it right away:

*still* -c 'slurp | grim -g- -' | swappy -f -

# OPTIONS

*-h*
	Show help message and quit.

*-c* _command_
	Shell command, which will be executed via *sh -c <command>* while the screen
	is frozen.

*-p*
	Include a pointer (cursor) on a frozen screenshot.

# SEE ALSO

_grim_(1) _slurp_(1) _swappy_(1)

# AUTHORS

Written and maintained by Sergei Slipchenko <faergeek@gmail.com>.

Source code is available at https://github.com/faergeek/still
