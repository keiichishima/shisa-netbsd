divert(-1)
#
# Copyright (c) 2003 Sendmail, Inc. and its suppliers.
#	All rights reserved.
#
# By using this file, you agree to the terms and conditions set
# forth in the LICENSE file which can be found at the top level of
# the sendmail distribution.
#
#
#

divert(0)
VERSIONID(`Id: unicos.m4,v 1.1 2003/04/21 17:03:51 ca Exp')
VERSIONID(`$NetBSD: unicos.m4,v 1.1.1.1 2005/03/15 02:06:02 atatat Exp $')
define(`ALIAS_FILE', `/usr/lib/aliases')
define(`HELP_FILE', `/usr/lib/sendmail.hf')
define(`QUEUE_DIR', `/usr/spool/mqueue')
define(`STATUS_FILE', `/usr/lib/sendmail.st')
MODIFY_MAILER_FLAGS(`LOCAL', `+aSPpmnxXu')
MODIFY_MAILER_FLAGS(`SMTP', `+anpeLC')
define(`LOCAL_SHELL_FLAGS', `pxehu')
define(`confPID_FILE', `/etc/sendmail.pid')dnl
