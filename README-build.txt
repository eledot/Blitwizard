
--------------
 BUILD README
--------------

This is the README for building blitwizard.

If you compile for Windows or want a static build:
Please consult README-deps.txt first for getting the
required dependencies.

== Get required tools  ==

If you are on Unix (Linux or Mac), please open a bash terminal now.
Then cd to the root directory of blitwizard. Please note gcc has
to be installed!

If you are on Windows, install MinGW (you MUST include MSYS and a
C compiler for it to work).

Get MinGW/MSYS through mingw-get: http://www.mingw.org/wiki/Getting_Started

Then move the whole blitwizard folder (this extracted archive with
all the deps copied into it as specified above) into your MSYS root
dir, which should be at some place like C:\MinGW\MSYS\1.0\ (depends
on your choice during the MinGW installation).

E.g. move the blitwizard folder to C:\MinGW\MSYS\1.0\blitwizard

Then open the MSYS shell and type (please note the leading slash):
   cd /blitwizard/
to get into that folder.

== Configuration step ==

Type "./configure" (without the quotations)  into your Unix bash or MSYS shell.
If you use the git code (not an official release), you will need to
type "sh ./autogen.sh" FIRST before doing that.

If you get an autoreconf: no such file error during "sh ./autogen.sh",
then you miss autotools:
  - On Windows, install MinGW/MSYS again and make sure to tick all
    the MSYS stuff you can find so you end up with autotools.
  - On Linux, search autotools or autoconf in your package manager.
  - On Mac, install mac ports and search for autotools or autoconf.

Please note there are options you can change if you want to do so,
type "./configure --help" to view them (you don't need to however).
If you aren't sure what they mean, just leave them alone :-)

== Configure feature output ==

IMPORTANT: When configure finishes, it will print a large summary of all
features available and libraries used for Blitwizard.

Please check if that summary contains audio/video/physics or whatever
you might want to use!

If any feature you want is unavailable, you need to either install
the library system-wide (Linux) or consult README-deps.txt on how to
properly provide it (Windows or static builds in general).

== Compilation step ==

Type "make" (without quotations)  into your Unix bash or MSYS shell to
compile.
You should now have a blitwizard binary in your bin directory in
the blitwizard folder. Have fun!

You might want to install Blitwizard system-wide so simply typing
"blitwizard" will run it. To do that, type "sudo make install" now.

== Guide for FFmpeg support ==

Blitwizard can use FFmpeg to support a huge amount of audio formats
instead of just .ogg and .flac (see README.txt).

Provide Blitwizard with the FFmpeg source code before you do the
configure step or it won't have FFmpeg support at runtime.

Put the contents of a recent FFmpeg tarball (check http://ffmpeg.org/)
into src/ffmpeg/ - the tarball has a main folder named "ffmpeg-XX"
inside in which the contents are located. Extract the contents directly,
without the ffmpeg-XX folder being parent of them, into src/ffmpeg/.

Please note the FFmpeg version you use at this point is NOT that
important. If the FFmpeg version used at runtime differs, it usually
doesn't make any difference. If it ever does for you, please drop us
a note so we can add information about it here!

