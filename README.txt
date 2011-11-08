
--------
 README
--------
This is the README for building blitwizard.
Please note the dependency step is also required on Unix/Linux!

== Build guide ==
(1) Dependencies

For building this software, you first need to:

 - drop the contents of a source tarball of a recent SDL 1.3 into
    src/sdl/
   see http://www.libsdl.org/hg.php

 - drop the contents of a source tarball of a recent libogg release into
    src/ogg
   see http://xiph.org/downloads/

 - drop the contents of a source tarball of a recent libvorbis release into
    src/vorbis
   http://xiph.org/downloads/

 - drop the contents of a source tarball of a recent Lua 5 release into
    src/lua
   see http://www.lua.org/download.html
   (IMPORTANT: You should apply the patches aswell if you need a secure Lua!)

 - drop the contents of a source tarball of a recent libpng release into
    src/imgloader/png
   see http://libpng.org/pub/png/libpng.html

 - drop the contents of a source tarball of a recent zlib release into
    src/imgloader/zlib
   see http://zlib.net/

Please note you should NOT drop the whole main folder itself
of a tarball into the directories, but just what the main folder
*contains* (You will get a verbose error later if you did this
part wrong - so if you get errors about a "MISSING DEPENDENCY"
which you provided, you most likely did this part wrong).

(2) Appropriate tools

If you are on Unix (Linux or Mac), please open a bash terminal now.
Then cd to the root directory of blitwizard. Please note gcc has
to be installed!

If you are on Windows, install MinGW (you MUST include MSYS!).
Then move the whole blitwizard folder into your MSYS root dir,
e.g. C:\MinGW\MSYS\1.0\ (or differently depending on your choice
during the isntallation).

E.g. move the blitwizard folder to C:\MinGW\MSYS\1.0\blitwizard

Then open the MSYS shell and type (please note the leading slash):
   cd /blitwizard/
to get into that folder.

(3) Configuration step

Type ./configure into your Unix bash or MSYS shell.
If you use the git code (not an official release), you will need to
type sh ./autogen.sh FIRST before doing that.
Please note there are options you can change if you want to do so,
type ./configure --help to view them (you don't need to however).

(4) Compilation step

Type make into your Unix bash or MSYS shell to compile.
You should now have a blitwizard.exe in your bin directory in
the blitwizard folder. Have fun!
