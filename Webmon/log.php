<?php
include 'config.php';

if ($_GET['t'] == 'true')
{
	$last_size = 0;
	if (file_exists('last'))
		$last_size = file_get_contents('last');
	if ($last_size == filesize($log_file))
		exit;
	file_put_contents('last', filesize($log_file));
}
$all_lines = read_file($log_file);
//if (!is_array($all_lines) || count($all_lines) == 0)
//	exit;
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
		<th align="center" bgcolor="#FF0066">Time</th>
		<th align="center" bgcolor="#FF0066">Host name</th>
		<th align="center" bgcolor="#FF0066">Serial</th>
		<th align="center" bgcolor="#FF0066">Vendor ID</th>
		<th align="center" bgcolor="#FF0066">Product ID</th>
		<th align="center" bgcolor="#FF0066">Action</th>
	</tr>
<?php
for($i=0;$i<count($arr);$i++)
{
	$t = split("\t",$arr[$i]);
echo <<<EOM
<tr>
	<td align="center" bgcolor="#E0E0E0">{$t[0]}</td>
	<td align="center" bgcolor="#E0E0E0">{$t[1]}</td>
	<td align="center" bgcolor="#E0E0E0">{$t[2]}</td>
	<td align="center" bgcolor="#E0E0E0">{$t[3]}</td>
	<td align="center" bgcolor="#E0E0E0">{$t[4]}</td>
	<td align="center" bgcolor="#E0E0E0">{$t[5]}</td>
</tr>
EOM;
}
?>
</table>
<br>
<?php for($i=0; $i < count($pages); $i++): ?>
     <a href="#" onclick="get_data(<?php echo $i; ?>); return false;"><?php echo ($i+1); ?></a>
<?php endfor; ?>
