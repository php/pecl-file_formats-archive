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
#include "zend_exceptions.h"
#include "php_archive.h"
#include "archive_clbk.h"
#include "archive_writer.h"
#include "archive_util.h"
#include "php_archive_entry.h"

#define DEFAULT_BYTES_PER_BLOCK 8192

zend_class_entry *ce_ArchiveWriter;
zend_class_entry *ce_ArchiveWriterInterface;

/* zend_function_entry funcs_ArchiveWriterInterface {{{ */
zend_function_entry funcs_ArchiveWriterInterface[] = {
 	ZEND_ABSTRACT_ME(ArchiveWriter, __construct,  NULL)
	ZEND_ABSTRACT_ME(ArchiveWriter, addEntry,  NULL)
	ZEND_ABSTRACT_ME(ArchiveWriter, finish,  NULL)
	{NULL, NULL, NULL}
};
/* }}} */

/* zend_function_entry funcs_ArchiveWriter {{{ */
zend_function_entry funcs_ArchiveWriter[] = {
 	ZEND_ME(ArchiveWriter, __construct,  NULL, 0)
	ZEND_ME(ArchiveWriter, addEntry,  NULL, 0)
	ZEND_ME(ArchiveWriter, finish,  NULL, 0)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(archive_writer)
{
	zend_class_entry tmp_ce_ArchiveWriter, tmp_ce_ArchiveWriterInterface;

	INIT_CLASS_ENTRY(tmp_ce_ArchiveWriterInterface, "ArchiveWriterInterface", funcs_ArchiveWriterInterface);
	ce_ArchiveWriterInterface = zend_register_internal_class(&tmp_ce_ArchiveWriterInterface TSRMLS_CC);
	ce_ArchiveWriterInterface->ce_flags = ZEND_ACC_INTERFACE;

	INIT_CLASS_ENTRY(tmp_ce_ArchiveWriter, "ArchiveWriter", funcs_ArchiveWriter);
	ce_ArchiveWriter = zend_register_internal_class(&tmp_ce_ArchiveWriter TSRMLS_CC);

	zend_class_implements(ce_ArchiveWriter TSRMLS_CC, 1, ce_ArchiveWriterInterface);

	return SUCCESS;
}
/* }}} */

/* ArchiveWriter::__construct {{{
 *
*/
ZEND_METHOD(ArchiveWriter, __construct)
{
	archive_file_t *arch = NULL;
	int resource_id;
	zval *this = getThis();
	const char *error_string = NULL;
	char *filename;
	long error_num, filename_len, result, format=0, compression=0;
	zend_error_handling error_handling;

	zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll", &filename, &filename_len, &format, &compression) == FAILURE) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

#if PHP_API_VERSION < 20100412
	if (PG(safe_mode) && (!php_checkuid(filename, NULL, CHECKUID_CHECK_FILE_AND_DIR))) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}
#endif

	if (php_check_open_basedir(filename TSRMLS_CC)) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	arch = (archive_file_t *) emalloc(sizeof(archive_file_t));

	arch->stream = NULL;

	ALLOC_HASHTABLE(arch->entries);
	zend_hash_init(arch->entries, 10, NULL, _archive_entries_hash_dtor, 0);

	arch->mode = PHP_ARCHIVE_WRITE_MODE;
	arch->buf = emalloc(PHP_ARCHIVE_BUF_LEN + 1);
	arch->filename = estrndup(filename, filename_len);
	arch->arch = archive_write_new();

	switch (compression) {
		case PHP_ARCHIVE_COMPRESSION_GZIP:
			if (archive_write_add_filter_gzip(arch->arch) != ARCHIVE_OK) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Gzip compression is not supported in this build");
				zend_restore_error_handling(&error_handling TSRMLS_CC);
				return;
			}
			break;

		case PHP_ARCHIVE_COMPRESSION_BZIP2:
			if (archive_write_add_filter_bzip2(arch->arch) != ARCHIVE_OK) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Bzip2 compression is not supported in this build");
				zend_restore_error_handling(&error_handling TSRMLS_CC);
				return;
			}
			break;
		case 0: /* default value */
		case PHP_ARCHIVE_COMPRESSION_NONE:
			/* always supported */
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported compression type %ld", compression);
			zend_restore_error_handling(&error_handling TSRMLS_CC);
			return;
			break;
	}

	switch (format) {
		case 0: /* default value */
		case PHP_ARCHIVE_FORMAT_TAR:
		case PHP_ARCHIVE_FORMAT_PAX_RESTRICTED:
			archive_write_set_format_pax_restricted(arch->arch);
			break;
		case PHP_ARCHIVE_FORMAT_PAX:
			archive_write_set_format_pax(arch->arch);
			break;
		case PHP_ARCHIVE_FORMAT_CPIO:
			archive_write_set_format_cpio(arch->arch);
			break;
		case PHP_ARCHIVE_FORMAT_SHAR:
			archive_write_set_format_shar(arch->arch);
			break;
		case PHP_ARCHIVE_FORMAT_USTAR:
			archive_write_set_format_ustar(arch->arch);
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported archive format: %ld", format);
			zend_restore_error_handling(&error_handling TSRMLS_CC);
			return;
			break;
	}

	archive_write_set_bytes_per_block(arch->arch, DEFAULT_BYTES_PER_BLOCK);
	result = archive_write_open(arch->arch, arch, _archive_open_clbk, _archive_write_clbk, _archive_close_clbk);
	/* do not pad the last block */
	archive_write_set_bytes_in_last_block(arch->arch, 1);

	if (result) {
		error_num = archive_errno(arch->arch);
		error_string = archive_error_string(arch->arch);
		efree(arch->filename);
		efree(arch->buf);
		efree(arch);
		if (error_num && error_string) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to open file %s for writing: error #%ld, %s", filename, error_num, error_string);
		}
		else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to open file %s for writing: unknown error %ld", filename, result);
		}
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	resource_id = zend_list_insert(arch,le_archive);
	add_property_resource(this, "fd", resource_id);

	zend_restore_error_handling(&error_handling TSRMLS_CC);
	return;
}
/* }}} */

/* ArchiveWriter::addEntry {{{
 *
*/
ZEND_METHOD(ArchiveWriter, addEntry)
{
	zval *this = getThis();
	zval *entry_obj;
	archive_file_t *arch;
	archive_entry_t *entry, *entry_copy;
	char *pathname;
	int pathname_len;
	const struct stat *stat_sb;
	zend_error_handling error_handling;

	zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &entry_obj) == FAILURE) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (!_archive_get_fd(this, &arch TSRMLS_CC)) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (!instanceof_function(Z_OBJCE_P(entry_obj), ce_ArchiveEntry TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "An instance of ArchiveEntry is required");
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (!_archive_get_entry_struct(entry_obj, &entry TSRMLS_CC)) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	pathname = entry->filename;
	pathname_len = strlen(pathname);

	_archive_normalize_path(&pathname, &pathname_len);

	if (pathname_len == 0 || pathname[0] == '\0') {
		/* user is probably trying to add "./", "/", ".." or ".", ignoring it silently */
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		RETURN_TRUE;
	}

	/* copy entry.. */
	entry_copy = emalloc(sizeof(archive_entry_t));
	memcpy(entry_copy, entry, sizeof(archive_entry_t));
	entry_copy->entry = archive_entry_new();
	entry_copy->filename = estrdup(entry->filename);

	entry_copy->data = NULL;
	entry_copy->data_len = 0;

	archive_entry_copy_pathname(entry_copy->entry, pathname);
	stat_sb = archive_entry_stat(entry->entry);
	archive_entry_copy_stat(entry_copy->entry, stat_sb);

	/* ..and add it to the hash */
	zend_hash_update(arch->entries, pathname, pathname_len + 1, &entry_copy, sizeof(archive_entry_t), NULL);
	zend_restore_error_handling(&error_handling TSRMLS_CC);
	RETURN_TRUE;
}
/* }}} */

/* ArchiveWriter::finish {{{
 *
*/
ZEND_METHOD(ArchiveWriter, finish)
{
	zval *this = getThis();
	int resource_id;
	HashPosition pos;
	archive_file_t *arch;
	archive_entry_t **entry;
	int result, error_num;
	const char *error_string;
	mode_t mode;
	php_stream *stream;
	zend_error_handling error_handling;

	zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);

	if (!_archive_get_fd(this, &arch TSRMLS_CC)) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (zend_hash_sort(arch->entries, zend_qsort, _archive_pathname_compare, 0 TSRMLS_CC) == FAILURE) {
		RETURN_FALSE;
	}

	zend_hash_internal_pointer_reset_ex(arch->entries, &pos);
	while (zend_hash_get_current_data_ex(arch->entries, (void **)&entry, &pos) == SUCCESS) {

		mode = archive_entry_mode((*entry)->entry);

		if (S_ISREG(mode) && archive_entry_size((*entry)->entry) > 0) {
			if ((stream = php_stream_open_wrapper_ex((*entry)->filename, "r", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL, NULL))) {
				char buf[PHP_ARCHIVE_BUF_LEN];
				int header_written=0;
				int read_bytes;

				while ((read_bytes = php_stream_read(stream, buf, PHP_ARCHIVE_BUF_LEN))) {
					if (!header_written) {
						/* write header only after the first successful read */
						result = archive_write_header(arch->arch, (*entry)->entry);
						if (result == ARCHIVE_FATAL) {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write entry header for file %s: fatal error", (*entry)->filename);
							zend_restore_error_handling(&error_handling TSRMLS_CC);
							return;
						}
						header_written = 1;
					}
					result = archive_write_data(arch->arch, buf, read_bytes);

					if (result <= 0) {
						error_num = archive_errno(arch->arch);
						error_string = archive_error_string(arch->arch);

						if (error_num && error_string) {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write file %s to archive: error #%d, %s", (*entry)->filename, error_num, error_string);
						}
						else {
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write file %s: unknown error %d", (*entry)->filename, result);
						}
						php_stream_close(stream);
						zend_restore_error_handling(&error_handling TSRMLS_CC);
						return;
					}
				}
				php_stream_close(stream);
			}
		}
		else {
			result = archive_write_header(arch->arch, (*entry)->entry);
			if (result == ARCHIVE_FATAL) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write entry header for file %s: fatal error", (*entry)->filename);
				zend_restore_error_handling(&error_handling TSRMLS_CC);
				return;
			}
		}
		zend_hash_move_forward_ex(arch->entries, &pos);
	}

	if ((resource_id = _archive_get_rsrc_id(this TSRMLS_CC))) {
		add_property_resource(this, "fd", 0);
		zend_list_delete(resource_id);
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		RETURN_TRUE;
	}

	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to finish writing of archive file");
	zend_restore_error_handling(&error_handling TSRMLS_CC);
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
