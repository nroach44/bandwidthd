<center>
<img src=logo.gif>
<br>
<br>
<FORM name="navigation">
<SELECT name="sensors" onChange="window.location=document.navigation.sensors.options[document.navigation.sensors.selectedIndex].value"> 
<OPTION SELECTED value="index.php">--Select A Sensor--
<?
include("include.php");
$db = ConnectDb();
$sql = "SELECT sensor_id from bd_tx_log group by sensor_id order by sensor_id;";
$result = pg_query($sql);
while ($r = pg_fetch_array($result))
	echo "<option value=report.php?sensor_id=".$r['sensor_id'].">".$r['sensor_id'];
?>
</SELECT>
</FORM>
