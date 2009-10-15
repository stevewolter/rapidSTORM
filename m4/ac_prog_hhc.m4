# SYNOPSIS
#
#   AC_PROG_HHC([default-flags])
#
# DESCRIPTION
#
#   Find a hhc executable.
#
#   Input:
#
#   "default-flags" is the default $HHC_FLAGS, which will be overridden
#   if the user specifies --with-hhc-flags.
#
#   Output:
#
#   $HHC contains the path to hhc, or is empty if none was found
#   or the user specified --without-hhc. $HHC_FLAGS contains the
#   flags to use with hhc.
#
# LICENSE
#
#   Adapted for HHC 2009 Steve Wolter <http://swolter.sdf1.org>
#   Copyright (c) 2008 Zmanda Inc. <http://www.zmanda.com/>
#   Copyright (c) 2008 Dustin J. Mitchell <dustin@zmanda.com>
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.

AC_DEFUN([AC_PROG_HHC],
[
HHC_FLAGS="$1"
AC_SUBST(HHC_FLAGS)

# The (lack of) whitespace and overquoting here are all necessary for
# proper formatting.
AC_ARG_WITH(hhc,
AS_HELP_STRING([--with-hhc[[[[[=PATH]]]]]],
               [Use the Microsoft HTML Help compiler binary in in PATH.]),
    [ ac_with_hhc=$withval; ],
    [ ac_with_hhc=maybe; ])

AC_ARG_WITH(hhc-flags,
AS_HELP_STRING([  --with-hhc-flags=FLAGS],
               [Flags to pass to hhc (default $1)]),
    [ if test "x$withval" == "xno"; then
	HHC_FLAGS=''
    else
	if test "x$withval" != "xyes"; then
	    HHC_FLAGS="$withval"
	fi
    fi
	])

# search for hhc if it wasn't specified
if test "$ac_with_hhc" = "yes" -o "$ac_with_hhc" = "maybe"; then
    AC_PATH_PROGS(HHC,hhc)
else
    if test "$ac_with_hhc" != "no"; then
        if test -x "$ac_with_hhc"; then
            HHC="$ac_with_hhc";
        else
            AC_MSG_WARN([Specified hhc of $ac_with_hhc isn't])
            AC_MSG_WARN([executable; searching for an alternative.])
            AC_PATH_PROGS(HHC,hhc)
        fi
    fi
fi
])
