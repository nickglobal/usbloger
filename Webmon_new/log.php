<?php
include 'config.php';

if ($_GET['t'] == 'true')
{
	$last_size = 0;
	if (file_exists('last'))
		$last_size = file_get_contents('last');
	if (!file_exists($log_file) || $last_size == filesize($log_file))
		exit;
	file_put_contents('last', filesize($log_file));
}
$all_lines = read_file($log_file);
if (!is_array($all_lines) || count($all_lines) == 0)
	exit;
$header = split("\t",$all_lines[0]);
unset($all_lines[0]);
$all_lines = array_reverse($all_lines);
$pages = array_chunk($all_lines, 20);
$p = intval($_GET['p']);
if ($p < 0)
	$p = 0;
if ($p >= count($pages))
	$p = count($pages)-1;
$arr = $pages[$p];
?>
<table width="100%" class="data">
	<tr>
<?php
for($i=0;$i<count($header);$i++)
{
echo <<<EOM
<th align="center" bgcolor="#FF0066">{$header[$i]}</th>
EOM;
}
?>
	</tr>
<?php
for($i=0;$i<count($arr);$i++)
{
	$t = split("\t",$arr[$i]);
	echo '<tr>';
	for($j=0;$j<count($header);$j++)
	{
		echo '<td align="center" bgcolor="#E0E0E0">'.$t[$j].'</th>';
	}
	echo '</tr>';
}
?>
</table>
<br>
<?php for($i=0; $i < count($pages); $i++): ?>
     <a href="#" onclick="get_data(<?php echo $i; ?>); return false;"><?php echo ($i+1); ?></a>
<?php endfor; ?>
