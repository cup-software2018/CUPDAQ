#!/usr/bin/env bash 

export MALLOC_CHECK_=0

OPTS=`getopt -o r:o:n:dmt --long onldaqdir:,rawdatadir: -n 'executedaq.sh' -- "$@"`

if [ $? != 0 ]; then
    echo "Terminating..." >&2; exit 1;
fi

eval set -- "$OPTS"

RUNNUM=
ONLDAQDIR=
RAWDATADIR=
EXE=
ADCOPT=
DAQID=
DAQNAME=

while true; do
    case "$1" in
    -r) RUNNUM="$2"
        shift 2;;
    -o) DAQOPT="$2"
        shift 2;;
    -n) DAQNAME="$2"
        shift 2;;
    -d) EXE="nulldaq"
        shift ;;
    -m) EXE="nullmerger"
        shift ;;
    -t) EXE="nulltcb"
        shift ;;
    --onldaqdir) ONLDAQDIR="$2"
        shift 2;;
    --rawdatadir) RAWDATADIR="$2"
        shift 2;;
    --) shift;
        break ;;
    *) break ;;
    esac
done

if [ "$ONLDAQDIR" ]
then
    export ONLDAQ_DIR="$ONLDAQDIR"
fi

if [ "$RAWDATADIR" ]
then
    export RAWDATA_DIR="$RAWDATADIR"
fi

if [ "$VERSION" ]
then
    # plz setting modules (like root, ...), here ...!!!
    export LD_LIBRARY_PATH=${ONLDAQ_DIR}/lib:$LD_LIBRARY_PATH
fi

RUNNUMSTR=`printf "%06d" $RUNNUM`

${ONLDAQ_DIR}/bin/$EXE $DAQOPT > ${RAWDATA_DIR}/LOG/${DAQNAME}_${RUNNUMSTR}.log 2>&1 &

exit
