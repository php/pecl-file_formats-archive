--TEST--
basic tests
--SKIPIF--
<?php if (!extension_loaded("archive")) print "skip"; ?>
--FILE--
<?php 

try {
	$ar = new ArchiveReader("nonex.file");
} catch (ArchiveException $e) {
	var_dump($e);
}

try {
	$ar = new ArchiveReader(".");
} catch (ArchiveException $e) {
	var_dump($e);
}

try {
	var_dump($ar = new ArchiveReader(dirname(__FILE__)."/../_files/gnutar.tar", ARCH_FORMAT_TAR));
} catch (ArchiveException $e) {
	var_dump($e);
}

var_dump($ar->close());

echo "Done\n";

?>
--EXPECTF--
object(ArchiveException)#%d (7) {
  ["message:protected"]=>
  string(88) "ArchiveReader::__construct(nonex.file): failed to open stream: No such file or directory"
  ["string:private"]=>
  string(0) ""
  ["code:protected"]=>
  int(0)
  ["file:protected"]=>
  string(%d) "%s001.php"
  ["line:protected"]=>
  int(%d)
  ["trace:private"]=>
  array(0) {
  }
  ["severity"]=>
  int(2)
}
object(ArchiveException)#%d (7) {
  ["message:protected"]=>
  string(143) "ArchiveReader::__construct(): Failed to open file . for reading: error #84, Empty input file: Invalid or incomplete multibyte or wide character"
  ["string:private"]=>
  string(0) ""
  ["code:protected"]=>
  int(0)
  ["file:protected"]=>
  string(%d) "%s001.php"
  ["line:protected"]=>
  int(%d)
  ["trace:private"]=>
  array(0) {
  }
  ["severity"]=>
  int(2)
}
object(ArchiveReader)#%d (1) {
  ["fd"]=>
  resource(%d) of type (archive descriptor)
}
bool(true)
Done
