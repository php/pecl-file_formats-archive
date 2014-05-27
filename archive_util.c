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
#include "archive_util.h"

/* {{{ _archive_normalize_path
 */
void _archive_normalize_path(char **pathname, int *pathname_len)
{
	while (*pathname_len && (*pathname[0] == '.' || *pathname[0] == '/')) {
		(*pathname)++;
		(*pathname_len)--;
	}
}
/* }}} */

/* {{{ _archive_pathname_compare
 */
int _archive_pathname_compare(const void *a, const void *b TSRMLS_DC)
{
    Bucket *f, *s;
    zval result, first, second;

    f = *((Bucket **) a);
    s = *((Bucket **) b);

	Z_TYPE(first) = IS_STRING;
	Z_STRVAL(first) = (char *)f->arKey;
	Z_STRLEN(first) = f->nKeyLength-1;

	Z_TYPE(second) = IS_STRING;
	Z_STRVAL(second) = (char *)s->arKey;
	Z_STRLEN(second) = s->nKeyLength-1;

    if (string_compare_function(&result, &first, &second TSRMLS_CC) != SUCCESS) {
        return 0;
    }
    return (Z_LVAL(result) < 0 ? -1 : (Z_LVAL(result) > 0 ? 1 : 0));
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
