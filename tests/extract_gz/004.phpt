--TEST--
extract gnucpio.cpio.gz
--SKIPIF--
<?php
    if (!extension_loaded("archive")) print "skip";
    if (!defined('ARCH_COMPRESSION_GZIP')) print "skip gzip uncompression in not supported";
?>
--FILE--
<?php

include_once dirname(__FILE__)."/../unlink_entry.inc";

chdir(dirname(__FILE__));

$ar = new ArchiveReader(dirname(__FILE__)."/../_files/gnucpio.cpio.gz");

$files = Array();
while ($e = $ar->getNextEntry(false)) {
	var_dump($files[] = $e->getPathname());
	var_dump($ar->extractCurrentEntry());
}

foreach ($files as $file) {
	if (!file_exists(dirname(__FILE__).'/'.$file)) {
		echo $file, " doesn't exist!\n";
	}
}

foreach ($files as $file) {
	$file = realpath($file);
	$file = substr($file, strlen(dirname(__FILE__).'/'));
	unlink_entry(dirname(__FILE__).'/', $file);
}

echo "Done\n";
?>
--EXPECT--
string(8) "test.txt"
bool(true)
Done
