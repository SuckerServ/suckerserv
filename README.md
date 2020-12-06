# SuckerServ-v6

* WARNING : This is NOT a stable release, so if you encounter problems,
  please report all of them and don't hesitate to send patches. 
  Bugtracker: https://github.com/SuckerServ/suckerserv/issues *

* Since the v3 authserver has not been entirely ported to v4/5/6,
 if you want to use it, please install the SuckerServ-v3's one.
 More information at: https://wiki.piernov.org/en:suckerserv:auth *

This server is based on HopMod-v5. http://hopmod.googlecode.com/
It is intended to be used with Cube 2: Sauerbraten 2020 Edition.

## How to build SuckerServ

Installation guide : https://wiki.piernov.org/en:suckerserv:installation

Be sure to install all the dependencies needed!
List: https://wiki.piernov.org/en:suckerserv:installation#required_dependencies

### Quick installation

Make sure the script is marked as executable:
    chmod +x compile.sh
Then run it:
    ./compile.sh

Usage: ./compile.sh [--recompile] [--debug] — Build SuckerServ-v6
          --recompile   — Delete $COMPILEDIR (release_build or debug_build with --debug) before compiling SuckerServ again
          --debug       — Make a debug build

In case the compilation fails with memory allocation error, you can try to 
lower the number of threads used to compile by setting the THREADS variable.
For exemple:
    THREADS=1 ./compile.sh


## Starting and stopping the server

To start the server, run:
    ./bin/server start
And to stop it:
    ./bin/server stop


## Configuration

The configuration file is located at conf/server_conf.lua


## Help

For more informations, read the wiki located at: https://wiki.piernov.org/
You can also contact us at IRC: #suckerserv@irc.gamesurge.net
