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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_exceptions.h"
#include "php_archive.h"
#include "archive_reader.h"
#include "archive_writer.h"
#include "archive_util.h"
#include "archive_clbk.h"
#include "php_archive_entry.h"

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 2)
#define archive_ce_Exception zend_exception_get_default()
#else
#define archive_ce_Exception zend_exception_get_default(TSRMLS_C)
#endif

/* {{{ archive_functions[]
 */
zend_function_entry archive_functions[] = {
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ archive_module_entry
 */
zend_module_entry archive_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"archive",
	archive_functions,
	PHP_MINIT(archive),
	NULL, /* nothing */
	NULL, /*  to do  */
	NULL, /*  here   */
	PHP_MINFO(archive),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_ARCHIVE_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ARCHIVE
ZEND_GET_MODULE(archive)
#endif

static void _archive_desc_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);

/* {{{ _archive_entries_hash_dtor
 */
void _archive_entries_hash_dtor(void *data)
{
	archive_entry_t *entry = *(archive_entry_t **)data;
	TSRMLS_FETCH();

	_archive_entry_free(entry TSRMLS_CC);
}
/* }}} */

/* {{{ _archive_desc_dtor
 */
static void _archive_desc_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	 archive_file_t *arch = (archive_file_t *)rsrc->ptr;

	if (arch->mode == PHP_ARCHIVE_READ_MODE) {
		archive_read_close(arch->arch);
		archive_read_free(arch->arch);
	}
	else if (arch->mode == PHP_ARCHIVE_WRITE_MODE) {
		archive_write_close(arch->arch);
		archive_write_free(arch->arch);
	}

	if (arch->stream) {
		php_stream_close(arch->stream);
	}

	if (arch->filename) {
		efree(arch->filename);
	}

	if (arch->entries) {
		zend_hash_destroy(arch->entries);
		efree(arch->entries);
	}

	efree(arch->buf);
	efree(arch);
}
/* }}} */

/* {{{ _archive_get_rsrc_id
 */
int _archive_get_rsrc_id(zval *this TSRMLS_DC)
{
	zval **prop;

	if (zend_hash_find(Z_OBJPROP_P(this), "fd", sizeof("fd"), (void **)&prop) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find archive file descriptor");
		return 0;
	}

	return Z_LVAL_PP(prop);
}
/* }}} */

/* {{{ _archive_get_fd
 */
int _archive_get_fd(zval *this, archive_file_t **arch TSRMLS_DC)
{
	int resource_id, type;


	if ((resource_id = _archive_get_rsrc_id(this TSRMLS_CC))) {
		*arch = (archive_file_t *) zend_list_find(resource_id, &type);
		if (*arch && type==le_archive) {
			return 1;
		}
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find archive file descriptor #%d", resource_id);
	}
	return 0;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(archive)
{
	zend_class_entry tmp_ce_ArchiveException;

	le_archive = zend_register_list_destructors_ex(_archive_desc_dtor, NULL, "archive descriptor", module_number);

	INIT_CLASS_ENTRY(tmp_ce_ArchiveException, "ArchiveException", NULL);
	ce_ArchiveException = zend_register_internal_class_ex(&tmp_ce_ArchiveException, archive_ce_Exception, NULL TSRMLS_CC);

	PHP_MINIT(archive_entry)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(archive_reader)(INIT_FUNC_ARGS_PASSTHRU);

	PHP_MINIT(archive_writer)(INIT_FUNC_ARGS_PASSTHRU);

	REGISTER_LONG_CONSTANT("ARCH_FORMAT_TAR", PHP_ARCHIVE_FORMAT_TAR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_FORMAT_CPIO", PHP_ARCHIVE_FORMAT_CPIO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_FORMAT_PAX", PHP_ARCHIVE_FORMAT_PAX, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_FORMAT_PAX_RESTRICTED", PHP_ARCHIVE_FORMAT_PAX_RESTRICTED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_FORMAT_SHAR", PHP_ARCHIVE_FORMAT_SHAR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_FORMAT_USTAR", PHP_ARCHIVE_FORMAT_USTAR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_FORMAT_RAR", PHP_ARCHIVE_FORMAT_RAR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_FORMAT_ZIP", PHP_ARCHIVE_FORMAT_ZIP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_FORMAT_7ZIP", PHP_ARCHIVE_FORMAT_7ZIP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_FORMAT_NONE", PHP_ARCHIVE_FORMAT_NONE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_COMPRESSION_GZIP", PHP_ARCHIVE_COMPRESSION_GZIP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_COMPRESSION_BZIP2", PHP_ARCHIVE_COMPRESSION_BZIP2, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_COMPRESSION_COMPRESS", PHP_ARCHIVE_COMPRESSION_COMPRESS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARCH_COMPRESSION_NONE", PHP_ARCHIVE_COMPRESSION_NONE, CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(archive)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "archive support", "enabled");
	php_info_print_table_row(2, "extension version", PHP_ARCHIVE_VERSION);
	php_info_print_table_row(2, "libarchive version",  archive_version_string());
	php_info_print_table_end();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
