<?php

function unlink_entry($cwd, $pathname) {
	$bits = explode ("/", $pathname);
	if (is_dir($cwd.$bits[0])) {
		$dh = opendir($cwd.$bits[0]);
		while ($file = readdir($dh)) {
			if ($file == '.' || $file == '..') {
				continue;
			}
			if (is_dir($cwd.$bits[0].'/'.$file)) {
				//echo "unlink 1", $cwd.$bits[0].'/'.$file, "\n";
				unlink_entry($cwd.$bits[0].'/', $file);
			}
			else {
				//echo "unlink 2", $cwd.$bits[0].'/'.$file, "\n";
				@unlink($cwd.$bits[0].'/'.$file);
			}
		}
		
		closedir($dh);
		//echo "rmdir ", $cwd.$bits[0], "\n";
		@rmdir($cwd.$bits[0]);
	}
	else {
		//echo "unlink 3", $cwd.$bits[0],"\n";
		@unlink($cwd.$bits[0]);
	}
}

?>
