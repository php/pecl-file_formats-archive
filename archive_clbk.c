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
#include "php_archive.h"
#include "archive_clbk.h"

/* {{{ _archive_read_clbk
 */
ssize_t _archive_read_clbk(struct archive *a, void *client_data, const void **buff)
{
	archive_file_t *arch = (archive_file_t *)client_data;
	ssize_t len = 0;

	if (arch->stream == NULL) {
		return 0;
	}
	
	*buff = arch->buf;
	if ((len = php_stream_read(arch->stream, arch->buf, PHP_ARCHIVE_BUF_LEN))) {
		return len;
	}
	return 0;
}
/* }}} */

/* {{{ _archive_write_clbk
 */
ssize_t _archive_write_clbk(struct archive *a, void *client_data, void *buff, size_t buf_len)
{
	archive_file_t *arch = (archive_file_t *)client_data;
	ssize_t len;
	
	if (arch->stream == NULL) {
		return 0;
	}

	if ((len = php_stream_write(arch->stream, (char *)buff, buf_len))) {
		return len;
	}
	return 0;
}
/* }}} */

/* {{{ _archive_open_clbk
 */
int _archive_open_clbk(struct archive *a, void *client_data)
{
	archive_file_t *arch = (archive_file_t *)client_data;
	
	if (arch->mode == PHP_ARCHIVE_WRITE_MODE) {
		arch->stream = php_stream_open_wrapper_ex(arch->filename, "w", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL, NULL);
	} else if (arch->mode == PHP_ARCHIVE_READ_MODE) {
		arch->stream = php_stream_open_wrapper_ex(arch->filename, "r", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL, NULL);
	}
	
	if (arch->stream) {
		return 0;
	}
	return 1;
}
/* }}} */

/* {{{ _archive_close_clbk
 */
int _archive_close_clbk(struct archive *a, void *client_data)
{
	archive_file_t *arch = (archive_file_t *)client_data;
	
	if (arch->stream) {
		php_stream_close(arch->stream);
	}
	arch->stream = NULL;
	return 0;
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
