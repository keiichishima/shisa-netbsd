#	$NetBSD: largepages.inc,v 1.1 2000/09/07 17:20:58 thorpej Exp $
#
# Options to create a kernel suitable for mapping with large
# pages.
#

makeoptions	KERN_LDSCRIPT="kern.ldscript.4MB"
options 	LARGEPAGES		# Use large pages for kernel image
