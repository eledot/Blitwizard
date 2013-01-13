#!/bin/sh

rm -rf ./src/ogre/Plugins/EXRCodec/*
rm -rf ./src/ogre/Plugins/ParticleFX/*
rm -rf ./src/ogre/Plugins/BSPSceneManager/*
rm -rf ./src/ogre/Plugins/CgProgramManager/*
rm -rf ./src/ogre/Samples/Media/*
cd ./src/ogre
find . -name "*.o" -exec rm {} \;
find . -name "*.so" -exec rm {} \;
find . -name "*.a" -exec rm {} \;
rm -f lib/*.so.*
rm -rf ./Docs/*
