--TEST--
gnucpio.cpio
--SKIPIF--
<?php
    if (!extension_loaded("archive")) print "skip";
    if (!defined('ARCH_COMPRESSION_BZIP2')) print "skip bzip2 uncompression in not supported";
?>
--FILE--
<?php

$ar = new ArchiveReader(dirname(__FILE__)."/../_files/gnucpio.cpio.bz2");

while ($e = $ar->getNextEntry(true)) {
	var_dump($e);
	var_dump($e->isDir());
	var_dump($e->isFile());
	var_dump($e->isLink());
	var_dump($e->getPathname());
	var_dump($e->getResolvedPathname());
	var_dump($e->getUser());
	var_dump($e->getGroup());
	var_dump($e->getMtime());
	var_dump($e->getSize());
	var_dump($e->getPerms());
	var_dump($e->getData());
}

echo "Done\n";
?>
--EXPECTF--
object(ArchiveEntry)#%d (1) {
  ["entry"]=>
  resource(%d) of type (archive entry descriptor)
}
bool(false)
bool(true)
bool(false)
string(8) "test.txt"
bool(false)
bool(false)
bool(false)
int(1105229717)
int(1125)
int(33204)
string(1125) "  +----------------------------------------------------------------------+
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
"
Done
