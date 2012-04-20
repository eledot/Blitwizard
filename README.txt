
- Blitwizard README -

You got blitwizard at your hands, a 2d engine that runs Lua scripts
you write to specify the behaviour of your game/multimedia application.

LICENSE:

  The license of blitwizard is zlib (open e.g. src/main.c and read at the top).
  The examples are public domain including graphics and the blitwizard logo,
  which was designed by uoou - thanks :-)

  If you link libs statically by following the guide in README-deps.txt,
  check README-libs.txt and their source code for license remarks.
  
HOW TO USE:

  Run blitwizard to see a few example applications:
  Run on Windows:
    Double-click "Run-blitwizard.bat"
  Run on Linux/BSD/Mac:
    Open a terminal
    Change directory to the blitwizard directory
    Type: cd bin/ && ./blitwizard

  If you don't have a binary release, you need to build blitwizard first.
  Please check out README-build.txt for this.
   (if there is no such file, you have a binary release)

LEARN BLITWIZARD:

You can check out the example source code by opening the .lua files
of the examples which are located in examples/ in separate sub
folders. Feel free to modify those .lua files, then rerun the sample
browser and see how the examples changed!

Please check out docs/api.html for a complete documentation of the
provided functionality of blitwizard for advanced usage.

To start your own game project, copy all contents of the folder where
this README.txt is inside into a new empty folder named after your game
(e.g. a folder named "mygame"), then add a new game.lua into your new
game folder (you can write it with any text editor, e.g. Notepad) and
simply double-click "Run-Blitwizard.bat" inside your game folder to
run it.
(On Windows, for other systems simply run the appropriate blitwizard
binary from the bin folder).

To ship your game project with just the required files, check out
Ship-your-game.txt.

MORE AUDIO FORMATS THROUGH FFMPEG:

Blitwizard supports FFmpeg, however it does not ship with it.
FFmpeg supports a wide range of mulltimedia formats (e.g. mp3).

However, many other programs install FFmpeg and blitwizard does
automatically attempt to load and locate it if that is the case.

A possibly incomplete list of places checked for FFmpeg:
 - Valve's Steam usually comes with FFmpeg which
   blitwizard attempts to detect and load (on Windows)
 - Blender also comes with FFmpeg which litwizard should
   find (on Windows)
 - On Linux, /usr/lib is scanned for a system-wide FFmpeg
   installation

When FFmpeg is found and loaded, the additional formats will
be automatically available to your blitwizard program.

