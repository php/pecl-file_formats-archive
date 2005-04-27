--TEST--
basic ArchiveWriter tests
--SKIPIF--
<?php
    if (!extension_loaded("archive")) print "skip";
?>
--FILE--
<?php

$file = dirname(__FILE__)."/tmp.tar";

$writer = new ArchiveWriter($file);

$entry = new ArchiveEntry(dirname(__FILE__)."/../_files/bsdtar.cpio");
var_dump($writer->addEntry($entry));

$entry = new ArchiveEntry(dirname(__FILE__)."/../_files/");
var_dump($writer->addEntry($entry));

$entry = new ArchiveEntry(dirname(__FILE__)."/../_files/bsdtar.cpio.gz");
var_dump($writer->addEntry($entry));

var_dump($writer->finish());

$reader = new ArchiveReader($file);


while ($e = $reader->getNextEntry(false)) {
	    var_dump($e);
		var_dump($e->isDir());
		var_dump($e->isFile());
		var_dump($e->isLink());
		var_dump($e->getPathname());
		var_dump($e->getResolvedPathname());
		var_dump($e->getSize());
}

@unlink($file);

echo "Done\n";
?>
--EXPECTF--	
bool(true)
bool(true)
bool(true)
bool(true)
object(ArchiveEntry)#%d (1) {
  ["entry"]=>
  resource(%d) of type (archive entry descriptor)
}
bool(true)
bool(false)
bool(false)
string(%d) "%sadd/../_files/"
bool(false)
int(0)
object(ArchiveEntry)#%d (1) {
  ["entry"]=>
  resource(%d) of type (archive entry descriptor)
}
bool(false)
bool(true)
bool(false)
string(%d) "%sadd/../_files/bsdtar.cpio"
bool(false)
int(1297)
object(ArchiveEntry)#%d (1) {
  ["entry"]=>
  resource(%d) of type (archive entry descriptor)
}
bool(false)
bool(true)
bool(false)
string(%d) "%sadd/../_files/bsdtar.cpio.gz"
bool(false)
int(463)
Done
