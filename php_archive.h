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
  | Author: Antony Dovgal <antony@zend.com>                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_ARCHIVE_H
#define PHP_ARCHIVE_H

#include <archive.h>
#include <archive_entry.h>


#define	PHP_ARCHIVE_FORMAT_TAR				(1<<0) 
#define	PHP_ARCHIVE_FORMAT_CPIO				(1<<1)
#define	PHP_ARCHIVE_FORMAT_PAX				(1<<2)
#define	PHP_ARCHIVE_FORMAT_PAX_RESTRICTED	(1<<3)
#define	PHP_ARCHIVE_FORMAT_SHAR				(1<<4)
#define	PHP_ARCHIVE_FORMAT_USTAR			(1<<5)

enum {
	PHP_ARCHIVE_READ_MODE,
	PHP_ARCHIVE_WRITE_MODE
};

#define	PHP_ARCHIVE_COMPRESSION_GZIP		(1<<0)
#define	PHP_ARCHIVE_COMPRESSION_BZIP2		(1<<1)
#define	PHP_ARCHIVE_COMPRESSION_COMPRESS	(1<<2)
#define	PHP_ARCHIVE_COMPRESSION_NONE		(1<<3)

typedef struct php_archive_entry {
	struct archive_entry	*entry;
	char					*data;
	int						data_len;
	char					*filename; /* used only for writing */
	char					*resolved_filename; /* used only for writing */
} archive_entry_t;

typedef struct archive_file {
	int				mode;
	php_stream		*stream;
	struct archive	*arch;
	archive_entry_t  *current_entry;
	char			*filename;
	char			*buf;
	int				struct_state;
	HashTable		*entries;
} archive_file_t;

#define PHP_ARCHIVE_BUF_LEN 8196

#if defined(HAVE_BZ2) && defined(HAVE_ZLIB)
#define _php_archive_read_support_compression_all(arch) \
	archive_read_support_compression_bzip2(arch); \
	archive_read_support_compression_gzip(arch); \
	archive_read_support_compression_compress(arch);
#endif

#if defined(HAVE_BZ2) && !defined(HAVE_ZLIB)
#define _php_archive_read_support_compression_all(arch) \
	archive_read_support_compression_bzip2(arch); \
	archive_read_support_compression_compress(arch);
#endif

#if !defined(HAVE_BZ2) && defined(HAVE_ZLIB)
#define _php_archive_read_support_compression_all(arch) \
	archive_read_support_compression_gzip(arch); \
	archive_read_support_compression_compress(arch);
#endif

#if !defined(HAVE_BZ2) && !defined(HAVE_ZLIB)
#define _php_archive_read_support_compression_all(arch) \
	archive_read_support_compression_compress(arch);
#endif

PHPAPI zend_class_entry *ce_ArchiveException;
int le_archive;

extern zend_module_entry archive_module_entry;
#define phpext_archive_ptr &archive_module_entry

#ifdef PHP_WIN32
#define PHP_ARCHIVE_API __declspec(dllexport)
#else
#define PHP_ARCHIVE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

int _archive_get_rsrc_id(zval * TSRMLS_DC);
int _archive_get_fd(zval *, archive_file_t ** TSRMLS_DC);
void _archive_entries_hash_dtor(void *data TSRMLS_DC);
	
PHP_MINIT_FUNCTION(archive);
PHP_MINFO_FUNCTION(archive);

#ifdef ZTS
#define ARCHIVE_G(v) TSRMG(archive_globals_id, zend_archive_globals *, v)
#else
#define ARCHIVE_G(v) (archive_globals.v)
#endif

#endif	/* PHP_ARCHIVE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
