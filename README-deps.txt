
---------------------
 Dependencies README
---------------------

This is the README for obtaining the dependencies for the source tarball
of blitwizard. Follow this guide if you want to STATICALLY link any
dependencies which is recommended if you consider distributing a blitwizard
binary. Also, it is the recommended way when building on Windows!

  Note on Shared libs on Unix systems:

   If you are building on Linux/Unix, you can also simply install the required
   libs in your system (including the dev packages for them). They will be
   linked as SHARED libraries as a result. You should install those libs:
   SDL2, libpng, zlib, libogg, libvorbis, Lua, Box2D, libFLAC, Speex, Ogre3D,
   PhysFS

Static libraries avoid dependency problems at runtime (if you run blitwizard
only for yourself locally, you shouldn't worry about this!) and they save you
the bunch of external dlls on Windows.

Before you start, extract the blitwizard archive (where this README is in)
to a folder.
All relative paths specified are relative to that extracted folder.

Note on FFmpeg: blitwizard can use FFmpeg, but doesn't depend on it.
If you want to use FFmpeg, please check README-build.txt for detals.

== Dependencies ==

Either get the deps here: http://www.blitwizard.de/download-files/deps.zip
(extract into the blitwizard folder root here).

That file contains:
    SDL 2 hg cloned/fetched sometime in July 2012
    libpng 1.5.12
    zlib 1.2.7
    libogg 1.3.0
    libvorbis 1.3.2
    libFLAC 1.2.1
    Lua 5.2.1
	Box2D 2.2.1
    Speex 1.2rc1
    OGRE 1.8.0
    PhysFS 2.1 (hg)
    bullet 2.81

WARNING: Those versions might be outdated and may contain bugs! Please
always check if there are newer versions available, and consider
using those instead:   

Alternatively, get them yourself in hand-picked, current versions:

Required (always):
 - drop the contents of a source tarball of a recent Lua 5.2 release into
    src/lua
   see http://www.lua.org/download.html
   (IMPORTANT: You should apply the patches aswell if you need a secure Lua!)

Required for null device audio, audio:
 - drop the contents of a source tarball of a recent libogg release into
    src/ogg
   see http://xiph.org/downloads/

Required for null device audio, audio:
 - drop the contents of a source tarball of a recent libvorbis release into
    src/vorbis
   http://xiph.org/downloads/

Required for null device graphics, or normal (2d/3d) graphics:
 - drop the contents of a source tarball of a recent libpng release into
    src/imgloader/png
   see http://libpng.org/pub/png/libpng.html

Required for null device graphics, or normal (2d/3d) graphics:
 - drop the contents of a source tarball of a recent zlib release into
    src/imgloader/zlib
   see http://zlib.net/

Required for physics simulation:
 - drop the contents of a source tarball of a recent Box2D release into
    src/box2d
   see http://box2d.org/

Required for 2d graphics, audio:
 - drop the contents of a source tarball of a recent SDL 2 into
    src/sdl/
   see http://www.libsdl.org/hg.php

Required for 3d graphics:
 - drop the contents of a source tarball of a recent OGRE into
    src/ogre/
   see http://www.ogre3d.org/

Required for additional FLAC audio format support (if not using FFmpeg):
 - drop the contents of a source tarball of a recent libFLAC release into
    src/flac/
   see http://sourceforge.net/projects/flac/

Required for additional high quality audio resampling support (you want this!):
 - drop the contents of a source tarball of a recent Speex release into
    src/speex/
   see http://speex.org/

Required for .zip and game embedded into binary support:
 - drop the contents of a source tarball of a recent PhysFS >= 2.1 release into
    src/physfs/
   see http://icculus.org/physfs/

A source tarball is simply an archive that contains a folder with all
the source code of the product in it. If you get your tarball as .tar.gz
or .tar.bz2 (instead of .zip) and you are on Microsoft Windows, get
the archiver http://www.7-zip.org/ which can extract those.

Please note you should NOT drop the whole main folder inside
of a tarball into the directories, but just what the main folder
*contains* (so enter the main folder in your archiver, then
mark all the things inside, and extract those).

If blitwizard cannot find a lib as STATIC library during compilation
(so only as SHARED or not at all), you most likely did something wrong
here and you should check back on this.

