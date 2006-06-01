dnl $Heimdal: find-func.m4,v 1.1.42.1 2004/04/01 07:27:33 joda Exp $
dnl $NetBSD: find-func.m4,v 1.1.1.4 2004/04/02 14:48:06 lha Exp $
dnl
dnl AC_FIND_FUNC(func, libraries, includes, arguments)
AC_DEFUN([AC_FIND_FUNC], [
AC_FIND_FUNC_NO_LIBS([$1], [$2], [$3], [$4])
if test -n "$LIB_$1"; then
	LIBS="$LIB_$1 $LIBS"
fi
])
