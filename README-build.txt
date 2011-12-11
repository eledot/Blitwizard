
--------------
 BUILD README
--------------

This is the README for building blitwizard.

If you have a git version, please consult README-gitdeps.txt
first for getting the required dependencies.
BUT if you have an official source code release, you should check
that file anyway:

The DEPS CAN BE OUTDATED (which are included in the release version).
It is up to you to check if the provided deps are new enough for you,
also regarding fatal/security-relevant bugs.

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

Type ./configure into your Unix bash or MSYS shell.
If you use the git code (not an official release), you will need to
type sh ./autogen.sh FIRST before doing that.

If you get an autoreconf: no such file error during sh ./autogen.sh,
then you miss autotools. On Windows, install MinGW/MSYS again and make
sure to tick all the MSYS stuff you can find so you end up with
autotools. On Linux, search autotools or autoconf in your package
manager. On Mac, install mac ports and search for autotools or
autoconf.

Please note there are options you can change if you want to do so,
type ./configure --help to view them (you don't need to however).
If you aren't sure what they mean, just leave them alone :-)

== Compilation step ==

Type make into your Unix bash or MSYS shell to compile.
You should now have a blitwizard binary in your bin directory in
the blitwizard folder. Have fun!

