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
#include "php_archive.h"
#include "archive_clbk.h"

/* {{{ _archive_read_clbk
 */
ssize_t _archive_read_clbk(struct archive *a, void *client_data, const void **buff)
{
	archive_file_t *arch = (archive_file_t *)client_data;
	ssize_t len = 0;
	TSRMLS_FETCH();

	if (arch->stream == NULL) {
		return 0;
	}
	
	*buff = arch->buf;
	if ((len = php_stream_read(arch->stream, arch->buf, arch->block_size))) {
		return len;
	}
	return 0;
}
/* }}} */

/* {{{ _archive_write_clbk
 */
ssize_t _archive_write_clbk(struct archive *a, void *client_data, const void *buff, size_t buf_len)
{
	archive_file_t *arch = (archive_file_t *)client_data;
	ssize_t len;
	TSRMLS_FETCH();
	
	if (arch->stream == NULL) {
		return 0;
	}

	if ((len = php_stream_write(arch->stream, (char *)buff, buf_len))) {
		return len;
	}
	return 0;
}
/* }}} */

/* {{{ _archive_skip_clbk
 * */
off_t _archive_skip_clbk(struct archive *a, void *client_data, off_t request)
{
	archive_file_t *arch = (archive_file_t *)client_data;
	off_t size;
	int r;
	TSRMLS_FETCH();

	if(request == 0) {
		return 0;
	}

	if(arch->stream){
		size = (request/arch->block_size) * arch->block_size;
		if(size == 0){ /*do not break a block*/
			return 0;
		}
		/*TODO maybe lasy seek is a better idea for performance
		 * refer: libarchive archive_read_open_filename.c file_skip_lseek
		 * */
		r = php_stream_seek(arch->stream, size, SEEK_CUR);
		if(r < 0){
			return 0; 
		}
		return size;
	}
	return 0;
}
/*}}}*/

/* {{{ _archive_seek_clbk
 * */
ssize_t _archive_seek_clbk(struct archive *a, void *client_data, off_t offset, int whence)
{
	int r;
	archive_file_t *arch = (archive_file_t *)client_data;
	TSRMLS_FETCH();

	r = php_stream_seek(arch->stream, offset, whence);
	if(r == 0){
		return php_stream_tell(arch->stream);
	}
	return r;
}/*}}}*/

/* {{{ _archive_open_clbk
 */
int _archive_open_clbk(struct archive *a, void *client_data)
{
	archive_file_t *arch = (archive_file_t *)client_data;
	TSRMLS_FETCH();

	if (arch->mode == PHP_ARCHIVE_WRITE_MODE) {
		arch->stream = php_stream_open_wrapper_ex(arch->filename, "w", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL, NULL);
		if (arch->stream) {
			return 0;
		}
	} else if (arch->mode == PHP_ARCHIVE_READ_MODE) {
		arch->stream = php_stream_open_wrapper_ex(arch->filename, "r", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL, NULL);
		if (arch->stream) {
			/* Use libarchive to manage buffer
			 * here we set non-buffer of php stream
			 * */
			arch->stream->flags |= PHP_STREAM_FLAG_NO_BUFFER;
			if((arch->stream->flags & PHP_STREAM_FLAG_NO_SEEK) == 0){
				archive_read_set_skip_callback(arch->arch, _archive_skip_clbk);
				archive_read_set_seek_callback(arch->arch, _archive_seek_clbk);
			}
			return 0;
		}
	}

	return 1;
}
/* }}} */

/* {{{ _archive_close_clbk
 */
int _archive_close_clbk(struct archive *a, void *client_data)
{
	archive_file_t *arch = (archive_file_t *)client_data;
	TSRMLS_FETCH();

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
