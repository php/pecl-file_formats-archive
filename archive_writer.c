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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"
#include "php_archive.h"
#include "archive_clbk.h"
#include "archive_writer.h"
#include "php_archive_entry.h"

#define DEFAULT_BYTES_PER_BLOCK 8192

zend_class_entry *ce_ArchiveWriter;
zend_class_entry *ce_ArchiveWriterInterface;

/* function_entry funcs_ArchiveWriterInterface {{{ */
function_entry funcs_ArchiveWriterInterface[] = {
 	ZEND_ABSTRACT_ME(ArchiveWriter, __construct,  NULL)
	ZEND_ABSTRACT_ME(ArchiveWriter, addEntry,  NULL)
	ZEND_ABSTRACT_ME(ArchiveWriter, finish,  NULL)
	{NULL, NULL, NULL}
};
/* }}} */

/* function_entry funcs_ArchiveWriter {{{ */
function_entry funcs_ArchiveWriter[] = {
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
	int error_num, filename_len, result, format=0, compression=0;
	
	php_set_error_handling(EH_THROW, ce_ArchiveException TSRMLS_CC);
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll", &filename, &filename_len, &format, &compression) == FAILURE) {
		php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
		return;
	}

	if (PG(safe_mode) && (!php_checkuid(filename, NULL, CHECKUID_CHECK_FILE_AND_DIR))) {
		php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
		return;
	}
	
	if (php_check_open_basedir(filename TSRMLS_CC)) {
		php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
		return;
	}
	
	arch = (archive_file_t *) emalloc(sizeof(archive_file_t));
	
	arch->stream = NULL;
	arch->mode = PHP_ARCHIVE_WRITE_MODE;
	arch->buf = emalloc(PHP_ARCHIVE_BUF_LEN + 1);
	arch->filename = estrndup(filename, filename_len);	
	arch->arch = archive_write_new();

	switch (compression) {
#ifdef HAVE_ZLIB
		case 0:
		case PHP_ARCHIVE_GZIP:
			archive_write_set_compression_gzip(arch->arch);
			break;
#else
		case PHP_ARCHIVE_GZIP:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Gzip compression is not supported in this build");
			php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
			return;
			break;
#endif

#ifdef HAVE_BZ2
		case PHP_ARCHIVE_BZIP2:
			archive_write_set_compression_gzip(arch->arch);
			break;
#else
		case PHP_ARCHIVE_BZIP2:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Bzip2 compression is not supported in this build");
			php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
			return;
			break; 
#endif
		case 0:
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported compression type %d", compression);
			php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
			return;
			break;
	}

	if (!format) {
		format = PHP_ARCHIVE_FORMAT_TAR;
		archive_write_set_format_ustar(arch->arch);
	}
	else {
		switch (format) {
			case PHP_ARCHIVE_FORMAT_TAR:
			case PHP_ARCHIVE_FORMAT_PAX_RESTRICTED:
				archive_write_set_format_ustar(arch->arch);
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
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported archive format: %d", format);
				php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
				return;
				break;
		}
	}
	//archive_write_set_bytes_per_block(arch->arch, DEFAULT_BYTES_PER_BLOCK);
	result = archive_write_open(arch->arch, arch, _archive_open_clbk, _archive_write_clbk, _archive_close_clbk);
	
	if (result) {
		error_num = archive_errno(arch->arch);
		error_string = archive_error_string(arch->arch);
		efree(arch->filename);
		efree(arch->buf);
		efree(arch);
		if (error_num && error_string) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to open file %s for writing: error #%d, %s", filename, error_num, error_string);
		}
		else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to open file %s for writing: unknown error %d", filename, result);
		}	
		php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
		return;
	}

	resource_id = zend_list_insert(arch,le_archive);
	add_property_resource(this, "fd", resource_id);
	
	php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
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
	archive_entry_t *entry;
	int result, error_num;
	const char *error_string;
	mode_t mode;
	php_stream *stream;
	
	php_set_error_handling(EH_THROW, ce_ArchiveException TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &entry_obj) == FAILURE) {
		php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
		return;
	}
	
	if (!_archive_get_fd(this, &arch TSRMLS_CC)) {
		php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
		return;
	}
	
	if (!instanceof_function(Z_OBJCE_P(entry_obj), ce_ArchiveEntry TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "An instance of ArchiveEntry is required");
		php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
		return;
	}
	
	if(!_archive_get_entry_struct(entry_obj, &entry TSRMLS_CC)) {
		php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
		return;
	}
	
	archive_entry_set_pathname(entry->entry, entry->filename);
	
	mode = archive_entry_mode(entry->entry);

	if(S_ISREG(mode) && archive_entry_size(entry->entry) > 0) {
		if ((stream = php_stream_open_wrapper_ex(entry->filename, "r", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL, NULL))) {
			char buf[PHP_ARCHIVE_BUF_LEN];
			int header_written=0;
			int read_bytes;
			
			while ((read_bytes = php_stream_read(stream, buf, PHP_ARCHIVE_BUF_LEN))) {
				if (!header_written) {
					/* write header only after first successful read */
					archive_write_header(arch->arch, entry->entry);
					header_written = 1;
				}
				
				result = archive_write_data(arch->arch, buf, read_bytes);
				
				if (result <=0) {
					error_num = archive_errno(arch->arch);
					error_string = archive_error_string(arch->arch);

					if (error_num && error_string) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write file %s to archive: error #%d, %s", entry->filename, error_num, error_string);
					}
					else {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write file %s: unknown error %d", entry->filename, result);
					}
					php_stream_close(stream);
					php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);	
					return;
				}
			}
			php_stream_close(stream);
		}
	}
	else {
		archive_write_header(arch->arch, entry->entry);		
	}
	RETURN_TRUE;
}
/* }}} */

/* ArchiveWriter::finish {{{
 *
*/
ZEND_METHOD(ArchiveWriter, finish) 
{
	zval *this = getThis();
	int resourse_id;
	
	php_set_error_handling(EH_THROW, ce_ArchiveException TSRMLS_CC);

	if ((resourse_id = _archive_get_rsrc_id(this TSRMLS_CC))) {
		add_property_resource(this, "fd", 0);
		zend_list_delete(resourse_id);
		php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
		RETURN_TRUE;
	}
	
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to finish writing of archive file");
	php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
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
