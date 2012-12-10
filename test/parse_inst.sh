#!/bin/sh
if test ! -f "$1"
then
 echo "Error: executable $1 does not exist."
 exit 1
fi
if test ! -f "$2"
then
 echo "Error: trace log $2 does not exist."
 exit 1
fi
EXECUTABLE="$1"
TRACELOG="$2"
while read LINETYPE TID FADDR CADDR; do
 FNAME="$(addr2line -f -e ${EXECUTABLE} ${FADDR}|head -1)"
 OUTPUTF=${EXECUTABLE}_${TID}.xml
 if test "${LINETYPE}" = "e"
 then
 CNAME="$(addr2line -f -e ${EXECUTABLE} ${CADDR}|head -1)"
 CLINE="$(addr2line -s -e ${EXECUTABLE} ${CADDR})"
 echo "<func name=\"${FNAME}\" from=\"${CNAME}\" line=\"${CLINE}\">" >> ${OUTPUTF}
 fi
 if test "${LINETYPE}" = "x"
 then
 echo "</func>" >> ${OUTPUTF}
 fi
done < "${TRACELOG}"
