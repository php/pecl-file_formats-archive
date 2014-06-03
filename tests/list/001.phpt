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
  array(1) {
    [0]=>
    array(6) {
      ["file"]=>
      string(%d) "%s001.php"
      ["line"]=>
      int(%d)
      ["function"]=>
      string(11) "__construct"
      ["class"]=>
      string(13) "ArchiveReader"
      ["type"]=>
      string(2) "->"
      ["args"]=>
      array(1) {
        [0]=>
        string(10) "nonex.file"
      }
    }
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
