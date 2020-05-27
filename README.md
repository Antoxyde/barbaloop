# Barbloop, a status bar for DWM

Heavily inspired from [Dwmblocks](https://github.com/torrinfail/dwmblocks).

It mainly add the possibility to have blocks actualized with a C function instead of through a Popen call.

# Why
Popen calls sh which executes the given script/binary, which means mapping things into memory and performing many system calls and stuff. If you want to run a command every second or so, in my opinion its better for you and your battery to do it with a simple C function, if possible.
However, some informations are pretty hard to get in a few line of code, and would need a lots of code, so its convenient to still be able to get these through Popen.
