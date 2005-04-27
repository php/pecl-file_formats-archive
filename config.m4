dnl $Id$

PHP_ARG_WITH(archive, for tar/cpio support,
[  --with-archive=[DIR]    Include tar/cpio support])

if test "$PHP_ARCHIVE" != "no"; then
  SEARCH_PATH="/usr/local /usr"    
  SEARCH_FOR="/include/archive.h"  
  if test -r $PHP_ARCHIVE/$SEARCH_FOR; then 
    ARCHIVE_DIR=$PHP_ARCHIVE
  else 
    AC_MSG_CHECKING([for libarchive files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        ARCHIVE_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$ARCHIVE_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the libarchive distribution])
  fi

  PHP_ADD_INCLUDE($ARCHIVE_DIR/include)

  LIBNAME=archive
  LIBSYMBOL=archive_read_new

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $ARCHIVE_DIR/lib, ARCHIVE_SHARED_LIBADD)
    AC_DEFINE(HAVE_ARCHIVELIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong libarchive version or lib not found])
  ],[
    -L$ARCHIVE_DIR/lib -lm -ldl
  ])
  
  PHP_SUBST(ARCHIVE_SHARED_LIBADD)

dnl comment out writer while it's not ready
  PHP_NEW_EXTENSION(archive, archive.c archive_reader.c archive_writer.c archive_clbk.c archive_entry.c archive_util.c, $ext_shared)
dnl  PHP_NEW_EXTENSION(archive, archive.c archive_reader.c archive_clbk.c archive_entry.c, $ext_shared)
fi
