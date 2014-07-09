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

#include <sys/stat.h>

#include "php.h"
#include "zend_exceptions.h"
#include "php_archive.h"
#include "php_archive_entry.h"

/* zend_function_entry funcs_ArchiveEntry {{{ */
zend_function_entry funcs_ArchiveEntry[] = {
	ZEND_ME(ArchiveEntry, __construct,  NULL, 0)
	ZEND_ME(ArchiveEntry, isDir,  NULL, 0)
	ZEND_ME(ArchiveEntry, isFile,  NULL, 0)
	ZEND_ME(ArchiveEntry, isLink,  NULL, 0)
	ZEND_ME(ArchiveEntry, getPathname,  NULL, 0)
	ZEND_ME(ArchiveEntry, getResolvedPathname,  NULL, 0)
	ZEND_ME(ArchiveEntry, getUser,  NULL, 0)
	ZEND_ME(ArchiveEntry, getGroup,  NULL, 0)
	ZEND_ME(ArchiveEntry, getMtime,  NULL, 0)
	ZEND_ME(ArchiveEntry, getSize,  NULL, 0)
	ZEND_ME(ArchiveEntry, getPerms,  NULL, 0)
	ZEND_ME(ArchiveEntry, getData,  NULL, 0)
	{NULL, NULL, NULL}
};
/* }}} */
	
static void _archive_entry_desc_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);	

#define PHP_ENTRY_GET_STRUCT \
	zend_error_handling error_handling; \
	zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC); \
	if(!_archive_get_entry_struct(this, &entry TSRMLS_CC)) { \
		zend_restore_error_handling(&error_handling TSRMLS_CC); \
		return; \
	} \
	zend_restore_error_handling(&error_handling TSRMLS_CC);

/* {{{ _archive_entry_desc_dtor
 */
static void _archive_entry_desc_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	archive_entry_t *entry = (archive_entry_t *)rsrc->ptr;
	_archive_entry_free(entry TSRMLS_CC);	
}
/* }}} */

/* {{{ _archive_entry_free
 */
void _archive_entry_free(archive_entry_t *entry TSRMLS_DC)
{	
	if (entry->data) {
		efree(entry->data);
	}
	
	if (entry->filename) {
		efree(entry->filename);
	}
	
	if (entry->resolved_filename) {
		efree(entry->resolved_filename);
	}
	
	/*
	if (entry->entry) {
		 archive_entry_free(entry->entry); 
	}
	*/

	efree(entry);
}
/* }}} */

/* {{{ _archive_get_entry_rsrc_id
 */
int _archive_get_entry_rsrc_id(zval *this TSRMLS_DC)
{
	zval **prop;
	
	if (zend_hash_find(Z_OBJPROP_P(this), "entry", sizeof("entry"), (void **)&prop) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find entry descriptor");
		return 0;
	}
	
	return Z_LVAL_PP(prop);
}
/* }}} */

/* {{{ _archive_get_entry_struct
 */
int _archive_get_entry_struct(zval *this, archive_entry_t **entry TSRMLS_DC)
{
	int resource_id, type;
	
	if ((resource_id = _archive_get_entry_rsrc_id(this TSRMLS_CC))) {
		*entry = (archive_entry_t *) zend_list_find(resource_id, &type);
		if (*entry && type==le_archive_entry) {
			return 1;
		}
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find entry descriptor #%d", resource_id);
	}
	return 0;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(archive_entry) {
	zend_class_entry ce;

	le_archive_entry = zend_register_list_destructors_ex(_archive_entry_desc_dtor, NULL, "archive entry descriptor", module_number);
	
	INIT_CLASS_ENTRY(ce, "ArchiveEntry", funcs_ArchiveEntry);
	ce_ArchiveEntry = zend_register_internal_class_ex(&ce, ce_ArchiveEntry, NULL TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

/* ArchiveEntry::__construct {{{
 *
*/
ZEND_METHOD(ArchiveEntry, __construct) 
{
	zval *this = getThis();
	char *filename;
	int filename_len, resource_id;
	archive_entry_t *entry;
	struct stat *stat_sb;
	php_stream_statbuf ssb;
	zend_error_handling error_handling;

	zend_replace_error_handling(EH_THROW, ce_ArchiveException, &error_handling TSRMLS_CC); 

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE) {
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

	if (php_stream_stat_path_ex((char *)filename, PHP_STREAM_URL_STAT_LINK, &ssb, NULL)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "stat failed for %s", filename);
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	stat_sb = &ssb.sb;

	entry = (archive_entry_t *) emalloc(sizeof(archive_entry_t));
	entry->resolved_filename = NULL;

#if (!defined(__BEOS__) && !defined(NETWARE) && HAVE_REALPATH) || defined(ZTS)
	if (S_ISLNK(stat_sb->st_mode)) {	
		char resolved_path_buff[MAXPATHLEN];

		if (VCWD_REALPATH(filename, resolved_path_buff)) {
			/* TODO figure out if we need this check */
#if 0 && defined(ZTS)
			if (VCWD_ACCESS(resolved_path_buff, F_OK)) {
				efree(entry);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "stat failed for %s", filename);
				zend_restore_error_handling(&error_handling TSRMLS_CC);
				return;
			}
#endif
			entry->resolved_filename = estrdup(resolved_path_buff);
		}
	}
#endif

	if (!S_ISREG(stat_sb->st_mode)) {
		stat_sb->st_size = 0;
	}
	
	entry->entry = archive_entry_new();
	entry->data = NULL;
	entry->data_len = 0;
	entry->filename = estrndup(filename, filename_len);

	archive_entry_copy_stat(entry->entry, &ssb.sb);

	resource_id = zend_list_insert(entry,le_archive_entry);	
	add_property_resource(this, "entry", resource_id);

	zend_restore_error_handling(&error_handling TSRMLS_CC);
	return;
}
/* }}} */

/* ArchiveEntry::isDir {{{
 *
*/
ZEND_METHOD(ArchiveEntry, isDir) 
{
	zval *this = getThis();
	archive_entry_t *entry;
	mode_t mode;

	PHP_ENTRY_GET_STRUCT;

	mode = archive_entry_mode(entry->entry);
	
	RETURN_BOOL(S_ISDIR(mode));
}
/* }}} */

/* ArchiveEntry::isFile {{{
 *
*/
ZEND_METHOD(ArchiveEntry, isFile) 
{
	zval *this = getThis();
	archive_entry_t *entry;
	mode_t mode;
	
	PHP_ENTRY_GET_STRUCT;
	
	mode = archive_entry_mode(entry->entry);
	
	RETURN_BOOL(S_ISREG(mode));
}
/* }}} */

/* ArchiveEntry::isLink {{{
 *
*/
ZEND_METHOD(ArchiveEntry, isLink) 
{
	zval *this = getThis();
	archive_entry_t *entry;
	mode_t mode;
	
	PHP_ENTRY_GET_STRUCT;
	
	mode = archive_entry_mode(entry->entry);
	
	RETURN_BOOL(S_ISLNK(mode));
}
/* }}} */

/* ArchiveEntry::getPathname {{{
 *
*/
ZEND_METHOD(ArchiveEntry, getPathname) 
{
	zval *this = getThis();
	archive_entry_t *entry;
	const char *pathname;
	
	PHP_ENTRY_GET_STRUCT;
	
	if (entry->filename) {
		RETURN_STRING(entry->filename, 1);
	}

	if ((pathname = archive_entry_pathname(entry->entry))) {
		RETURN_STRING((char *)pathname, 1);
	}
	RETURN_FALSE;
}
/* }}} */

/* ArchiveEntry::getResolvedPathname {{{
 *
*/
ZEND_METHOD(ArchiveEntry, getResolvedPathname) 
{
	zval *this = getThis();
	archive_entry_t *entry;
	const char *pathname;
	
	PHP_ENTRY_GET_STRUCT;

	if (entry->resolved_filename) {
		RETURN_STRING(entry->resolved_filename, 1);
	}
	
	if ((pathname = archive_entry_symlink(entry->entry))) {
		RETURN_STRING((char *)pathname, 1);
	}
	
	RETURN_FALSE;
}
/* }}} */

/* ArchiveEntry::getUser {{{
 *
*/
ZEND_METHOD(ArchiveEntry, getUser) 
{
	zval *this = getThis();
	archive_entry_t *entry;
	const char *uname;
	
	PHP_ENTRY_GET_STRUCT;
	
	if ((uname = archive_entry_uname(entry->entry))) {
		RETURN_STRING((char *)uname, 1);
	}
	RETURN_FALSE;
}
/* }}} */

/* ArchiveEntry::getGroup {{{
 *
*/
ZEND_METHOD(ArchiveEntry, getGroup) 
{
	zval *this = getThis();
	archive_entry_t *entry;
	const char *gname;
	
	PHP_ENTRY_GET_STRUCT;
	
	if ((gname = archive_entry_gname(entry->entry))) {
		RETURN_STRING((char *)gname, 1);
	}
	RETURN_FALSE;
}
/* }}} */

/* ArchiveEntry::getMtime {{{
 *
*/
ZEND_METHOD(ArchiveEntry, getMtime) 
{
	zval *this = getThis();
	archive_entry_t *entry;
	time_t mtime;
	
	PHP_ENTRY_GET_STRUCT;
	
	mtime = archive_entry_mtime(entry->entry);
	
	RETURN_LONG(mtime);
}
/* }}} */

/* ArchiveEntry::getSize {{{
 *
*/
ZEND_METHOD(ArchiveEntry, getSize) 
{
	zval *this = getThis();
	archive_entry_t *entry;
	off_t size; 
	
	PHP_ENTRY_GET_STRUCT;
	
	size = archive_entry_size(entry->entry);
	
	RETURN_LONG(size);
}
/* }}} */

/* ArchiveEntry::getPerms {{{
 *
*/
ZEND_METHOD(ArchiveEntry, getPerms) 
{
	zval *this = getThis();
	archive_entry_t *entry;
	mode_t mode;
	
	PHP_ENTRY_GET_STRUCT;
	
	mode = archive_entry_mode(entry->entry);
	
	RETURN_LONG(mode);
}
/* }}} */

/* ArchiveEntry::getData {{{
 *
*/
ZEND_METHOD(ArchiveEntry, getData) 
{
	zval *this = getThis();
	archive_entry_t *entry;
	
	PHP_ENTRY_GET_STRUCT;
	
	if (entry->data) {
		RETURN_STRINGL(entry->data, entry->data_len, 1);
	}
	RETURN_FALSE;
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
