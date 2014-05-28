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

#ifndef ARCHIVE_CLBK_H
#define ARCHIVE_CLBK_H

ssize_t _archive_read_clbk(struct archive *, void *, const void **);
int    _archive_open_clbk(struct archive *, void *);
int    _archive_close_clbk(struct archive *, void *);
off_t _archive_skip_clbk(struct archive *, void *, off_t);
ssize_t _archive_seek_clbk(struct archive *, void *, off_t, int);
ssize_t _archive_write_clbk(struct archive *, void *, const void *, size_t);

#endif /* ARCHIVE_CLBK_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
