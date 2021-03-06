#!/bin/sh
# $NetBSD: genwakecode.sh,v 1.2 2007/12/09 20:32:18 jmcneill Exp $

P='/WAKEUP_/ { printf("#define\t%s%s\t%s\n", $2, length($2) < 16 ? "\t" : "", $1); }'
awk "$P" < acpi_wakecode.bin.map

echo 
echo 'static const unsigned char wakecode[] = {';
hexdump -v -e '"\t" 8/1 "0x%02x, " "\n"' < acpi_wakecode.bin | sed 's/0x  /0x00/g'
echo '};'

exit 0
