#!/bin/sh

color() {
  color="$1"
  text="$2"
  echo "$(tput bold)$(tput setaf ${color})${text}$(tput sgr0)"
}

helpMsg() {
  cat << EOH
Usage: $(color 4 ${0}) [--$(color 5 recompile)] [--$(color 1 debug)] — Build $PROJECT
          --$(color 5 recompile)   — Delete $(color 6 \$COMPILEDIR) (release_build or debug_build with --$(color 1 debug)) before compiling $PROJECT again
          --$(color 1 debug)       — Make a $(color 1 debug) build

All extra parameters are passed to cmake.
Environment variables:
          STRIP         — Set to "0" to disable binary stripping in release build (default: "1")
EOH
}

PROJECT="$(color 3 SuckerServ-v6)"
STRCOMPILE="$(color 2 Compiling)"
RECOMPILE=false
COMPILEFLAGS="-DCMAKE_COLOR_DIAGNOSTICS=On -DCMAKE_INSTALL_PREFIX="

if command -v ninja >/dev/null 2>&1; then
  COMPILEFLAGS="$COMPILEFLAGS -DCMAKE_GENERATOR=Ninja"
fi

if [ -z "${DESTDIR}" ]; then
  export DESTDIR="$(cd "$(dirname "$0")" && pwd)"
fi

COMPILEDIR="release_build"
BUILDTYPE="$(color 6 release)"
BUILDTYPEFLAGS="-DCMAKE_BUILD_TYPE=Release"

for arg in "$@"; do
  case $arg in
    --help)
      helpMsg
      exit
    ;;
    --debug)
      COMPILEDIR="debug_build"
      BUILDTYPEFLAGS="-DCMAKE_BUILD_TYPE=Debug"
      BUILDTYPE="$(color 1 debug)"
    ;;
    --recompile)
      STRCOMPILE="$(color 5 Recompiling)"
      RECOMPILE=true
    ;;
    *) # pass other arguments to CMAKE
      COMPILEFLAGS="$COMPILEFLAGS $arg"
  esac
done

COMPILEFLAGS="$COMPILEFLAGS $BUILDTYPEFLAGS"

if [ "$RECOMPILE" = true ]; then
  rm -rf $COMPILEDIR
fi

if [ -z "${STRIP}" ]; then
  STRIP="1"
fi

# Now compile the source code and install it in server's directory
echo "$STRCOMPILE $PROJECT ($BUILDTYPE build)"
echo "Extra flags passed to CMake: $COMPILEFLAGS"
cmake -S . -B "$COMPILEDIR" $COMPILEFLAGS
cmake --build "$COMPILEDIR" --parallel

[ "$?" != "0" ] && color 1 "CMAKE FAILED" && exit 1
if `echo "$COMPILEFLAGS" | grep -q "DEBUG"` | [ "${STRIP}" = "0" ]; then
  cmake --install "$COMPILEDIR"
  [ "$?" != "0" ] && color 1 "CMAKE INSTALL FAILED" && exit 1
else
  cmake --install "$COMPILEDIR" --strip
  [ "$?" != "0" ] && color 1 "CMAKE INSTALL/STRIP FAILED" && exit 1
fi

exit 0
