/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2004 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Antony Dovgal <tony2001@php.net>                             |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_ARCHIVE_ENTRY_H
#define PHP_ARCHIVE_ENTRY_H

#ifndef S_ISDIR
#define S_ISDIR(mode)	(((mode)&S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(mode)	(((mode)&S_IFMT) == S_IFREG)
#endif
#ifndef S_ISLNK
#define S_ISLNK(mode)	(((mode)&S_IFMT) == S_IFLNK)
#endif

zend_class_entry *ce_ArchiveEntry;

ZEND_METHOD(ArchiveEntry, __construct);
ZEND_METHOD(ArchiveEntry, isDir);
ZEND_METHOD(ArchiveEntry, isFile);
ZEND_METHOD(ArchiveEntry, isLink);
ZEND_METHOD(ArchiveEntry, getPathname);
ZEND_METHOD(ArchiveEntry, getResolvedPathname);
ZEND_METHOD(ArchiveEntry, getUser);
ZEND_METHOD(ArchiveEntry, getGroup);
ZEND_METHOD(ArchiveEntry, getMtime);
ZEND_METHOD(ArchiveEntry, getSize);
ZEND_METHOD(ArchiveEntry, getPerms);
ZEND_METHOD(ArchiveEntry, getData);

PHP_MINIT_FUNCTION(archive_entry);

int le_archive_entry;

int _archive_get_entry_rsrc_id(zval * TSRMLS_DC);
int _archive_get_entry_struct(zval *, archive_entry_t ** TSRMLS_DC);
void _archive_entry_free(archive_entry_t * TSRMLS_DC);

#endif /* PHP_ARCHIVE_ENTRY_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
