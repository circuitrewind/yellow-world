To compile this code, you'll need the latest version of "pvsneslib" installed and
configured properly on your machine. Once this is done, it should be as simple as
running "make" (or "gmake" for your FreeBSD friends) in this directory.

pvsneslib can be found at:
https://github.com/alekmaul/pvsneslib

This project is called "Yellow World" as it started from pvsneslib's "Hello World"
example, with the displayed text of "Hello World" being one of the first changes.
The name just sort of stuck, as I was more focused on getting everything else
working, and didn't care too much to think up a proper name. ;)

To use this code, put "yellow_world.sfc" on your flash cart of choice. I tested
this with a normal SNES EverDrive. When the code first starts executing, there will
be a counter in the top-left slowly counting from 00 to 0F (hex). After this
completes, a new counter will take its place that increments much faster. Once the
fast counter is visible, you can remove the flash cart from the console, and then
insert the Game Processor RAM Cassette. At this point, you can press "B" on the
Player 1 controller to initiate the transfer from the RP2040 wired into the Player
2 controller slot.
