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

#ifndef PHP_ARCHIVE_H
#define PHP_ARCHIVE_H

#include <archive.h>
#include <archive_entry.h>

#define PHP_ARCHIVE_VERSION					"0.4.1-dev"

#define	PHP_ARCHIVE_FORMAT_NONE		        (0)
#define	PHP_ARCHIVE_FORMAT_TAR				ARCHIVE_FORMAT_TAR 
#define	PHP_ARCHIVE_FORMAT_CPIO				ARCHIVE_FORMAT_CPIO
#define	PHP_ARCHIVE_FORMAT_PAX				ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE
#define	PHP_ARCHIVE_FORMAT_PAX_RESTRICTED	ARCHIVE_FORMAT_TAR_PAX_RESTRICTED
#define	PHP_ARCHIVE_FORMAT_SHAR				ARCHIVE_FORMAT_SHAR
#define	PHP_ARCHIVE_FORMAT_USTAR			ARCHIVE_FORMAT_TAR_USTAR
#define	PHP_ARCHIVE_FORMAT_RAR				ARCHIVE_FORMAT_RAR
#define	PHP_ARCHIVE_FORMAT_ZIP		        ARCHIVE_FORMAT_ZIP
#define	PHP_ARCHIVE_FORMAT_7ZIP		        ARCHIVE_FORMAT_7ZIP

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
	int				block_size;
	HashTable		*entries;
} archive_file_t;

#define PHP_ARCHIVE_BUF_LEN 8196

#if ARCHIVE_VERSION_NUMBER < 3000000
#define archive_read_free archive_read_finish
#define archive_write_free archive_write_finish

#define archive_read_support_filter_all                  archive_read_support_compression_all
#define archive_read_support_filter_bzip2                archive_read_support_compression_bzip2
#define archive_read_support_filter_compress             archive_read_support_compression_compress
#define archive_read_support_filter_gzip                 archive_read_support_compression_gzip
#define archive_read_support_filter_lzip                 archive_read_support_compression_lzip
#define archive_read_support_filter_lzma                 archive_read_support_compression_lzma
#define archive_read_support_filter_none                 archive_read_support_compression_none
#define archive_read_support_filter_program              archive_read_support_compression_program
#define archive_read_support_filter_program_signature    archive_read_support_compression_program_signature
#define archive_read_support_filter_rpm                  archive_read_support_compression_rpm
#define archive_read_support_filter_uu                   archive_read_support_compression_uu
#define archive_read_support_filter_xz                   archive_read_support_compression_xz

#define archive_write_add_filter_bzip2     archive_write_set_compression_bzip2
#define archive_write_add_filter_compress  archive_write_set_compression_compress
#define archive_write_add_filter_gzip      archive_write_set_compression_gzip
#define archive_write_add_filter_lzip      archive_write_set_compression_lzip
#define archive_write_add_filter_lzma      archive_write_set_compression_lzma
#define archive_write_add_filter_none      archive_write_set_compression_none
#define archive_write_add_filter_program   archive_write_set_compression_program
#define archive_write_add_filter_xz        archive_write_set_compression_xz
#endif

#if ZEND_MODULE_API_NO < 20090626
typedef  int zend_error_handling;
#define zend_replace_error_handling(type, exception, current) php_set_error_handling((type), (exception) TSRMLS_CC)
#define zend_restore_error_handling(exception) php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC)
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
void _archive_entries_hash_dtor(void *data);
	
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
