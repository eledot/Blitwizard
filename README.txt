
- Blitwizard README -

You got blitwizard at your hands, a 2d engine that runs Lua scripts
you write to specify the behaviour of your game/multimedia application.

If you don't have a binary release, you need to build blitwizard first.
Please check out README-build.txt for this.
 (if there is no such file, you have a binary release)

If you have the binary release or succeeded in compilation, simply
enter the bin/ sub folder and run blitwizard in there to get the
sample browser to see a few simple examples on how to use blitwizard.

NOTE ON AUDIO:
  
  The blitwizard binary release is compiled with FFmpeg support.
  However, you need to provide libavcodec/libavformat if they are
  not present on the platform you use blitwizard on.

  Find libavcodec/libavformat binaries for Windows:

    See http://ffmpeg.zeranoe.com/builds/ and check out the
    32-bit Builds (Shared).

    You can open the .7z files with http://7-zip.org/

	You want libavformat.dll and libavcodec.dll. Put them
    into the same folder where blitwizard.exe is.

  Blitwizard will load FFmpeg at runtime and then support a huge
  amount of audio formats (.mp3, .wav, .mp4 and many others).
  If blitwizard fails to load FFmpeg, it will only support .ogg
  audio.

LEARN BLITWIZARD:

You can check out the example source code by opening the .lua files
of the examples which are located in examples/ in separate sub
folders. Feel free to modify those .lua files, then rerun the sample
browser and see how the examples changed!

Please check out docs/api.html for a complete documentation of the
provided functionality of blitwizard for advanced usage.

