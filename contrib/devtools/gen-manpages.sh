#!/bin/sh

TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
SRCDIR=${SRCDIR:-$TOPDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

TELESTAID=${TELESTAID:-$SRCDIR/telestaid}
TELESTAICLI=${TELESTAICLI:-$SRCDIR/telestai-cli}
TELESTAITX=${TELESTAITX:-$SRCDIR/telestai-tx}
TELESTAIQT=${TELESTAIQT:-$SRCDIR/qt/telestai-qt}

[ ! -x $TELESTAID ] && echo "$TELESTAID not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
RVNVER=($($TELESTAICLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for telestaid if --version-string is not set,
# but has different outcomes for telestai-qt and telestai-cli.
echo "[COPYRIGHT]" > footer.h2m
$TELESTAID --version | sed -n '1!p' >> footer.h2m

for cmd in $TELESTAID $TELESTAICLI $TELESTAITX $TELESTAIQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${RVNVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${RVNVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
