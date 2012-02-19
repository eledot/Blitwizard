
- Blitwizard README -

You got blitwizard at your hands, a 2d engine that runs Lua scripts
you write to specify the behaviour of your game/multimedia application.

Run on Windows:
  Double-click "Run-blitwizard.bat"
Run on Linux/BSD/Mac:
  Open a terminal
  Change directory to the blitwizard directory
  Type: cd bin/ && ./blitwizard

If you don't have a binary release, you need to build blitwizard first.
Please check out README-build.txt for this.
 (if there is no such file, you have a binary release)

If you have the binary release or succeeded in compilation, simply
enter the bin/ sub folder and run blitwizard in there to get the
sample browser to see a few simple examples on how to use blitwizard.

NOTE ON AUDIO:
  
  The blitwizard binary release is compiled with FFmpeg support.
  In particular, win32 blitwizard comes with FFmpeg, which allows
  blitwizard to load and play much more audio files.

  Read Ship-your-game.txt to find out how to ship with or without
  FFmpeg and for important legal notes.

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

