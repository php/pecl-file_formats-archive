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
#include "archive_reader.h"
#include "php_archive_entry.h"

zend_class_entry *ce_ArchiveReader;
zend_class_entry *ce_ArchiveReaderInterface;

/* zend_function_entry funcs_ArchiveReaderInterface {{{ */
zend_function_entry funcs_ArchiveReaderInterface[] = {
 	ZEND_ABSTRACT_ME(ArchiveReader, __construct,  NULL)
	ZEND_ABSTRACT_ME(ArchiveReader, getNextEntry,  NULL)
	ZEND_ABSTRACT_ME(ArchiveReader, getStream,  NULL)
	ZEND_ABSTRACT_ME(ArchiveReader, getArchiveFormat,  NULL)
	ZEND_ABSTRACT_ME(ArchiveReader, getCurrentEntryData,  NULL)
	ZEND_ABSTRACT_ME(ArchiveReader, readCurrentEntryData,  NULL)
	ZEND_ABSTRACT_ME(ArchiveReader, skipCurrentEntryData,  NULL)
	ZEND_ABSTRACT_ME(ArchiveReader, extractCurrentEntry,  NULL)
	ZEND_ABSTRACT_ME(ArchiveReader, close,  NULL)
	{NULL, NULL, NULL}
};
/* }}} */

/* zend_function_entry funcs_ArchiveReader {{{ */
zend_function_entry funcs_ArchiveReader[] = {
 	ZEND_ME(ArchiveReader, __construct,  NULL, 0)
	ZEND_ME(ArchiveReader, getStream,  NULL, 0)
	ZEND_ME(ArchiveReader, getArchiveFormat,  NULL, 0)
	ZEND_ME(ArchiveReader, getNextEntry,  NULL, 0)
	ZEND_ME(ArchiveReader, getCurrentEntryData,  NULL, 0)
	ZEND_ME(ArchiveReader, readCurrentEntryData,  NULL, 0)
	ZEND_ME(ArchiveReader, skipCurrentEntryData,  NULL, 0)
	ZEND_ME(ArchiveReader, extractCurrentEntry,  NULL, 0)
	ZEND_ME(ArchiveReader, close,  NULL, 0)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(archive_reader)
{
	zend_class_entry tmp_ce_ArchiveReader, tmp_ce_ArchiveReaderInterface;

	INIT_CLASS_ENTRY(tmp_ce_ArchiveReaderInterface, "ArchiveReaderInterface", funcs_ArchiveReaderInterface);
	ce_ArchiveReaderInterface = zend_register_internal_class(&tmp_ce_ArchiveReaderInterface TSRMLS_CC);
	ce_ArchiveReaderInterface->ce_flags = ZEND_ACC_INTERFACE;

	INIT_CLASS_ENTRY(tmp_ce_ArchiveReader, "ArchiveReader", funcs_ArchiveReader);
	ce_ArchiveReader = zend_register_internal_class(&tmp_ce_ArchiveReader TSRMLS_CC);

	zend_class_implements(ce_ArchiveReader TSRMLS_CC, 1, ce_ArchiveReaderInterface);

	return SUCCESS;
}
/* }}} */

/* ArchiveReader::__construct {{{
 *
*/
ZEND_METHOD(ArchiveReader, __construct)
{
	archive_file_t *arch = NULL;
	int resource_id;
	zval *this = getThis();
	const char *error_string = NULL;
	char *filename;
	long error_num, filename_len, result, format = 0, compression = 0, block_size = 0;
	zend_error_handling error_handling;

	zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lll", &filename, &filename_len, &format, &compression, &block_size) == FAILURE) {
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

	if (block_size <= 0) {
		block_size = PHP_ARCHIVE_BUF_LEN;
	}

	arch = (archive_file_t *) emalloc(sizeof(archive_file_t));

	arch->stream = NULL;
	arch->current_entry = NULL;
	arch->entries = NULL;
	arch->struct_state = ARCHIVE_OK;
	arch->block_size = block_size;
	arch->mode = PHP_ARCHIVE_READ_MODE;
	arch->buf = emalloc(arch->block_size + 1);
	arch->filename = estrndup(filename, filename_len);
	arch->arch = archive_read_new();


	archive_read_support_filter_all(arch->arch);
	switch(format) {
		case PHP_ARCHIVE_FORMAT_TAR:
			archive_read_support_format_tar(arch->arch);
			break;
		case PHP_ARCHIVE_FORMAT_CPIO:
			archive_read_support_format_cpio(arch->arch);
			break;
		default:
			archive_read_support_format_all(arch->arch);
			break;
	}

	switch(compression) {
		case PHP_ARCHIVE_COMPRESSION_NONE:
			break;
		case PHP_ARCHIVE_COMPRESSION_GZIP:
			if (archive_read_support_filter_gzip(arch->arch) != ARCHIVE_OK) {
				efree(arch->filename);
				efree(arch->buf);
				efree(arch);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Gzip compression support is not available in this build ");
				zend_restore_error_handling(&error_handling TSRMLS_CC);
				return;
			}
			break;
		case PHP_ARCHIVE_COMPRESSION_BZIP2:
			if (archive_read_support_filter_gzip(arch->arch) != ARCHIVE_OK) {
				efree(arch->filename);
				efree(arch->buf);
				efree(arch);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Bzip2 compression support is not available in this build ");
				zend_restore_error_handling(&error_handling TSRMLS_CC);
				return;
			}
		default:
			archive_read_support_filter_all(arch->arch);
			break;
	}

	result = archive_read_open(arch->arch, arch, _archive_open_clbk, _archive_read_clbk, _archive_close_clbk);

	if (result) {
		error_num = archive_errno(arch->arch);
		error_string = archive_error_string(arch->arch);
		if (arch->stream) {
			php_stream_close(arch->stream);
		}
		if (error_num && error_string) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to open file %s for reading: error #%ld, %s", filename, error_num, error_string);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to open file %s for reading: unknown error %ld", filename, result);
		}	
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		archive_read_close(arch->arch);
		archive_read_free(arch->arch);
		efree(arch->filename);
		efree(arch->buf);
		efree(arch);
		return;
	}

	resource_id = zend_list_insert(arch,le_archive);
	add_property_resource(this, "fd", resource_id);

	zend_restore_error_handling(&error_handling TSRMLS_CC);
	return;
}
/* }}} */

/* ArchiveReader::getArchiveFormat{{{
 *
 * */
ZEND_METHOD(ArchiveReader, getArchiveFormat) {
	long format;
	zval *this = getThis();
	archive_file_t *arch;
    zend_error_handling error_handling;

    zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);
	if (!_archive_get_fd(this, &arch TSRMLS_CC)) {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		RETURN_FALSE;
	}
	zend_restore_error_handling(&error_handling TSRMLS_CC);
	format = archive_format(arch->arch);

	RETURN_LONG(format);
}/*}}}*/

/* ArchiveReader::getStream{{{
 *
 * */
ZEND_METHOD(ArchiveReader, getStream) {
	zval *this = getThis();
	archive_file_t *arch;
    zend_error_handling error_handling;

    zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);
	if (!_archive_get_fd(this, &arch TSRMLS_CC)) {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}
	if (arch->stream) {
		php_stream_to_zval(arch->stream, return_value);
	} else {
		RETURN_FALSE;
	}
}
/*}}}*/

/* ArchiveReader::getNextEntry {{{
 *
*/
ZEND_METHOD(ArchiveReader, getNextEntry)
{
	zval *this = getThis();
	archive_file_t *arch;
	int result, error_num, resource_id;
	const char *error_string;
	zend_bool fetch_entry_data = 0;
	archive_entry_t *entry;
	struct archive_entry *current_entry;
	size_t len;
	off_t offset;
	char *buf;
    zend_error_handling error_handling;

    zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &fetch_entry_data) == FAILURE) {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (!_archive_get_fd(this, &arch TSRMLS_CC)) {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (arch->struct_state == ARCHIVE_OK) {
		result = archive_read_next_header(arch->arch, &current_entry);
		arch->struct_state = result;
		entry = (archive_entry_t *) emalloc(sizeof(archive_entry_t));
		entry->entry = current_entry;
		entry->data = NULL;
		entry->filename = NULL;
		entry->resolved_filename = NULL;
		entry->data_len = 0;
		arch->current_entry = entry;
	} else {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		RETURN_FALSE;
	}

	if (result && result != ARCHIVE_EOF) {
		arch->current_entry = NULL;
		error_num = archive_errno(arch->arch);
		error_string = archive_error_string(arch->arch);
		efree(entry);

		if (error_num && error_string) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to read file %s: error #%d, %s", arch->filename, error_num, error_string);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to read file %s: unknown error %d", arch->filename, result);
		}
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (result == ARCHIVE_EOF) {
		arch->current_entry = NULL;
		efree(entry);
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		RETURN_FALSE;
	}

	object_init_ex(return_value, ce_ArchiveEntry);

	if (fetch_entry_data) {
		while ((result = archive_read_data_block(arch->arch, (const void **)&buf, &len, &offset)) == ARCHIVE_OK) {
			entry->data = erealloc(entry->data, entry->data_len + len + 1);
			memcpy(entry->data + entry->data_len, buf, len);
			entry->data_len += len;
		}

		if (result && result != ARCHIVE_EOF) {
			error_num = archive_errno(arch->arch);
			error_string = archive_error_string(arch->arch);
			efree(entry);

			if (error_num && error_string) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to read file %s: error #%d, %s", arch->filename, error_num, error_string);
			}
			else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to read file %s: unknown error %d", arch->filename, result);
			}
            zend_restore_error_handling(&error_handling TSRMLS_CC);
			return;
		}
	}

	if (entry->entry) {
		resource_id = zend_list_insert(entry,le_archive_entry);
		add_property_resource(return_value, "entry", resource_id);
	}

    zend_restore_error_handling(&error_handling TSRMLS_CC);
}
/* }}} */

/* ArchiveReader::readCurrentEntryData(count) {{{
 *
*/
ZEND_METHOD(ArchiveReader, readCurrentEntryData) {
	zval *this = getThis();
	archive_file_t *arch;
	const char *error_string;
	size_t len;
	int r, error_num;
	long count;
    zend_error_handling error_handling;

    zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &count) == FAILURE) {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (!_archive_get_fd(this, &arch TSRMLS_CC)) {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}
	if (arch->current_entry == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Current archive entry is not available");
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}
	Z_STRVAL_P(return_value) = emalloc(count+1);
	len = 0;
	while(count > 0) {
		r = archive_read_data(arch->arch, Z_STRVAL_P(return_value)+len, count);
		if (r < ARCHIVE_OK) {
			error_num = archive_errno(arch->arch);
			error_string = archive_error_string(arch->arch);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Decompress entry failed errno(%d):%s", error_num, error_string);
			zend_restore_error_handling(&error_handling TSRMLS_CC);
			return;
		}
		if (r == 0) {
			break;
		}
		count -= r;
		len += r;
	}
	Z_STRVAL_P(return_value)[len] = 0;
	Z_STRLEN_P(return_value) = len;
	Z_TYPE_P(return_value) = IS_STRING;
    zend_restore_error_handling(&error_handling TSRMLS_CC);
}
/*}}}*/

/* ArchiveReader::getCurrentEntryData {{{
 *
*/
ZEND_METHOD(ArchiveReader, getCurrentEntryData)
{
	zval *this = getThis();
	archive_file_t *arch;
	int result, error_num;
	size_t len;
	off_t offset;
	const char *error_string;
	char *buf;
    zend_error_handling error_handling;

    zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);

	if (!_archive_get_fd(this, &arch TSRMLS_CC)) {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (arch->current_entry == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Current archive entry is not available");
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (arch->current_entry->data) {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		RETURN_STRINGL(arch->current_entry->data, arch->current_entry->data_len, 1);
	}

	while ((result = archive_read_data_block(arch->arch, (const void **)&buf, &len, &offset)) == ARCHIVE_OK) {
		arch->current_entry->data = erealloc(arch->current_entry->data, arch->current_entry->data_len + len + 1);
		memcpy(arch->current_entry->data + arch->current_entry->data_len, buf, len);
		arch->current_entry->data_len += len;
	}

	if (result && result != ARCHIVE_EOF) {
		error_num = archive_errno(arch->arch);
		error_string = archive_error_string(arch->arch);

		if (error_num && error_string) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to read entry data: error #%d, %s", error_num, error_string);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to read entry data: unknown error %d", result);
		}
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

    zend_restore_error_handling(&error_handling TSRMLS_CC);
	RETURN_STRINGL(arch->current_entry->data, arch->current_entry->data_len, 1);
}
/* }}} */

/* ArchiveReader::skipCurrentEntryData{{{
 * */
ZEND_METHOD(ArchiveReader, skipCurrentEntryData) {
	zval *this = getThis();
	archive_file_t *arch;
	zend_error_handling error_handling;
	int r;

	zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);

	if (!_archive_get_fd(this, &arch TSRMLS_CC)) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}
	r = archive_read_data_skip(arch->arch);
	if (r != ARCHIVE_OK) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Skip archive entry failed");
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}
	return;
}/*}}}*/

/* ArchiveReader::extractCurrentEntry {{{
 *
*/
ZEND_METHOD(ArchiveReader, extractCurrentEntry)
{
	zval *this = getThis();
	archive_file_t *arch;
	int result, error_num;
	long flags = 0;
	const char *error_string;
    zend_error_handling error_handling;

    zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &flags) == FAILURE) {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (!_archive_get_fd(this, &arch TSRMLS_CC)) {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (arch->current_entry == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Current archive entry is not available");
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (arch->current_entry->data) {
		/* again, rather annoying libarchive limitation: you can't extract or
		 * read entry anymore if it had been extracted/read before.
		 * */
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		RETURN_FALSE;
	}

	result = archive_read_extract(arch->arch, arch->current_entry->entry, flags);

	if (result && result != ARCHIVE_EOF) {
		error_num = archive_errno(arch->arch);
		error_string = archive_error_string(arch->arch);

		if (error_num && error_string) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to extract entry: error #%d, %s", error_num, error_string);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to extract entry: unknown error %d", result);
		}

        zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}
    zend_restore_error_handling(&error_handling TSRMLS_CC);
	RETURN_TRUE;
}
/* }}} */

/* ArchiveReader::close {{{
 *
*/
ZEND_METHOD(ArchiveReader, close)
{
	zval *this = getThis();
	int resourse_id;
    zend_error_handling error_handling;

    zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC);

	if ((resourse_id = _archive_get_rsrc_id(this TSRMLS_CC))) {
		add_property_resource(this, "fd", 0);
		zend_list_delete(resourse_id);
        zend_restore_error_handling(&error_handling TSRMLS_CC);
		RETURN_TRUE;
	}

	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to close archive file descriptor");
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
