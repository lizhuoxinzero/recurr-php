dnl $Id$
dnl config.m4 for extension recurr
PHP_ARG_ENABLE(recurr, recurr support,
[  --enable-recurr          Enable recurr support], [enable_recurr="yes"])

dnl Check whether the extension is enabled at all
if test "$PHP_RECURR" != "no"; then
  dnl Finally, tell the build system about the extension and what files are needed
  PHP_NEW_EXTENSION(recurr, php_recurr.c time.c datemap.c, $ext_shared)
  PHP_SUBST(RECURR_SHARED_LIBADD)
dnl $RECURR_SOURCES
fi
