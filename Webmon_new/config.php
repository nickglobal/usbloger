<?php
$dir_data = '//var//log//usbl_server//';
//$dir_data = './';

$config_file = $dir_data.'serial.txt';
$log_file = $dir_data.'report.txt';

function read_file($file)
{
	$handle = @fopen($file, "r");
	if ($handle)
	{
		while (!feof($handle))
		{
			$tmp = trim(fgets($handle, 4096));
			if ($tmp != '')
				$lines[] = $tmp;
		}
		fclose($handle);
	}
	return $lines;
}
?>
