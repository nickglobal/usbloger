<?php
include 'config.php';

function save_config($new_config)
{
	global $config_file;
	$out = fopen($config_file, "w");
	for($i = 0; $i < count($new_config); $i++)
	{
		fwrite($out, $new_config[$i]."\n");
	}
	fclose($out);
}

function clean_file()
{
	global $log_file;
	$all_lines = read_file($log_file);
	$header = $all_lines[0];
	$handle = fopen($log_file, 'w');
	if ($handle)
	{
		fwrite($handle, $header."\n");
		fclose($handle);
	}
}

$all_serials = read_file($config_file);
if ($_POST)
{
	if ($_POST['act'] == 'del')
	{
		unset($all_serials[intval($_POST['data'])]);
	}
	if ($_POST['act'] == 'save')
	{
		foreach($_POST as $k=>$v)
		{
			if (strpos($k, 'serial_') !== false)
			{
				$id = intval(substr($k, 7));
				$all_serials[$id] = $v;
			}
		}
	}
	if ($_POST['act'] == 'add')
	{
		if ($_POST['new_serial'] != '')
		{
			$all_serials[] = $_POST['new_serial'];
		}
	}
	if ($_POST['act'] == 'clean')
	{
		clean_file();
	}
	$all_serials = array_values($all_serials);
	save_config($all_serials);
}
?>

<html>
<head>
	<title>USBlogger</title>
	<style>
		.data {
			border: 1px solid grey;
		}
	</style>
	<script src="jquery-1.6.4.min.js" type="text/javascript"></script>
	<script src="jquery.timers-1.2.js" type="text/javascript"></script>
	<script type="text/javascript">
	function remove(id)
	{
		var form = document.forms['config'];
		form['act'].value = 'del';
		form['data'].value = id;
		form.submit();
	}
	function save()
	{
		var form = document.forms['config'];
		form['act'].value = 'save';
		form.submit();
	}
	function add()
	{
		var form = document.forms['config'];
		form['act'].value = 'add';
		form.submit();
	}
	function clean()
	{
		var form = document.forms['config'];
		form['act'].value = 'clean';
		form.submit();
	}
	function get_data(p, t)
	{
		$.get("log.php",{ p: p, t: t} , function(data){
			if (data != '')
			{
				$('#result').html(data);
			}
		});
	}
	$(function() {
		get_data(0);
	});
	
	$(document).everyTime(2000, function(i) {
		get_data(0, true);
	});
	</script>
</head>
<body>
<table align="center" width="90%">
	<tr align="center">
		<td colspan="2">
			<h1>USBLogger</h1>
			<br><br><br>
		</td>
	</tr>
	<tr align="center">
		<td width="80%">Logs <a href="#" onclick="get_data(0); return false;">reload</a></td>
		<td width="30%">Serials</td>
	</tr>
	<tr valign="top" align="center">
		<td>
			<div id="result"></div>
		</td>
		<td>
<form method="POST" name="config">
<input type="hidden" name="data" value="">
<input type="hidden" name="act" value="">
			<table width="90%" class="data">
				<tr>
					<th bgcolor="#00CCFF">Allowed Serials</th>
				</tr>
			</table>
			<div style="overflow-y: auto; width: 90%; height: 460px;" >
			<table width="100%" class="data">
<?php
for($i=0; $i < count($all_serials); $i++)
{
	$str = trim($all_serials[$i]);
echo <<<EOM
<tr>
	<td><input name="serial_{$i}" type="text" value="{$str}" style="width: 100%;" /></td>
	<td width="10"><a href="#" onclick="remove({$i})">remove</a></td>
</tr>
EOM;
}
?>
			</table>
			</div>
<br>
<input type="text" name="new_serial" value="" title="Type new allowed serial and press Add button"><a onclick="add()"><button>Add</button></a>
<br><br>
<a onclick="save()"><button title="After edit some serial number in the table, press this button to save changes">Save serials</button></a>
<a onclick="clean()"><button title="Clears all the statistics">Clear log</button></a>
</form>
		</td>
	</tr>
</table>
<div style="text-align: center">
<span style="color:#0101DF; font-size: 80%">
GlobalLogic Inc. Leaders in Software R&D Services, 2011.
</span>
</div>
</body>
</html>
