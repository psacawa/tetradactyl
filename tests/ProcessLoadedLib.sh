#!/usr/bin/bash

if [[ $# < 2 ]]; then
  echo "Run command then check whether shared object was loaded"
  echo "N.B. this depends on the process living at least a short time"
  echo "Use: ProcessLoadedLib.sh --lib <lib> -- <command> <arg1> ..."
  exit 1
fi

TEMP=$(getopt -o 'l:' --long 'lib:' -n 'ProcessLoadedLib.sh' -- "$@")

if [ $? -ne 0 ]; then
	echo 'Terminating...' >&2
	exit 1
fi

eval set -- "$TEMP"
unset TEMP

while true; do
  case $1 in
    '-l'|'--lib')
      LIB="$2"
      shift 2
      continue
      ;;
    *)
      break
      ;;
  esac
done

if [[ -z "$LIB" ]]; then
  echo need lib argument to detect
  exit 1
fi

# get rid of '--'
shift

echo "$@"
"$@" 2>/dev/null & 

echo $PID
PID=$!
sleep 0.5

grep -q "$LIB" "/proc/$PID/maps"
RET=$?
echo $RET

kill $PID
exit $RET
