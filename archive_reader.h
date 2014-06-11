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

#ifndef ARCHIVE_READER_H
#define ARCHIVE_READER_H
	
PHP_MINIT_FUNCTION(archive_reader);

ZEND_METHOD(ArchiveReader, __construct);
ZEND_METHOD(ArchiveReader, getStream);
ZEND_METHOD(ArchiveReader, getArchiveFormat);
ZEND_METHOD(ArchiveReader, getNextEntry);
ZEND_METHOD(ArchiveReader, getCurrentEntryData);
ZEND_METHOD(ArchiveReader, readCurrentEntryData);
ZEND_METHOD(ArchiveReader, skipCurrentEntryData);
ZEND_METHOD(ArchiveReader, extractCurrentEntry);
ZEND_METHOD(ArchiveReader, close);

#endif	/* ARCHIVE_READER_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
