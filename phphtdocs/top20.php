<?include("include.php");?>
<html>
<center>
<img src=logo.gif>
<?
if (isset($_GET['sensor_id']))
    $sensor_id = $_GET['sensor_id'];
else
	{
	echo "<br>Please provide a sensor_id";
	exit(1);
	}

if (isset($_GET['interval']))
    $interval = $_GET['interval'];
else
    $interval = DFLT_INTERVAL;

if (isset($_GET['timestamp']))
    $timestamp = $_GET['timestamp'];
else
    $timestamp = time() - $interval + (0.05*$interval);

$db = ConnectDb();
?>
</center>
<FORM name="navigation">
<SELECT name="sensors" onChange="window.location=document.navigation.sensors.options[document.navigation.sensors.selectedIndex].value">

<OPTION SELECTED value="index.php">--Select A Sensor--
<?
$sql = "SELECT sensor_id from bd_tx_log group by sensor_id order by sensor_id;";
$result = pg_query($sql);
while ($r = pg_fetch_array($result))
    echo "<option value=top20.php?sensor_id=".$r['sensor_id']."&interval=$interval>".$r['sensor_id'];
?>
</SELECT>
<SELECT name="intervals" onChange="window.location=document.navigation.intervals.options[document.navigation.intervals.selectedIndex].value">
<OPTION SELECTED value="index.php">--Select An Interval--
<OPTION value="top20.php?sensor_id=<?=$sensor_id?>&interval=<?=INT_DAILY?>">Daily
<OPTION value="top20.php?sensor_id=<?=$sensor_id?>&interval=<?=INT_WEEKLY?>">Weekly
<OPTION value="top20.php?sensor_id=<?=$sensor_id?>&interval=<?=INT_MONTHLY?>">Monthly
<OPTION value="top20.php?sensor_id=<?=$sensor_id?>&interval=<?=INT_YEARLY?>">Yearly
</select>
</FORM>
<center>
<h2>Top 20 - <?=$sensor_id?>
<?
$sql = "select tx.ip, rx.scale as rxscale, tx.scale as txscale, tx.total+rx.total as total, tx.total as sent, 
rx.total as received, tx.tcp+rx.tcp as tcp, tx.udp+rx.udp as udp,
tx.icmp+rx.icmp as icmp, tx.http+rx.http as http,
tx.p2p+rx.p2p as p2p, tx.ftp+rx.ftp as ftp
from

(SELECT ip, max(total/sample_duration)*8 as scale, sum(total) as total, sum(tcp) as tcp, sum(udp) as udp, sum(icmp) as icmp,
sum(http) as http, sum(p2p) as p2p, sum(ftp) as ftp
from bd_tx_log
where sensor_id = '$sensor_id' and
extract(epoch from timestamp) > $timestamp and extract(epoch from timestamp) < ".($timestamp+$interval)."
group by ip) as tx,

(SELECT ip, max(total/sample_duration)*8 as scale, sum(total) as total, sum(tcp) as tcp, sum(udp) as udp, sum(icmp) as icmp,
sum(http) as http, sum(p2p) as p2p, sum(ftp) as ftp
from bd_rx_log
where sensor_id = '$sensor_id' and 
extract(epoch from timestamp) > $timestamp and extract(epoch from timestamp) < ".($timestamp+$interval)."
group by ip) as rx

where tx.ip = rx.ip
order by total desc limit 20;";

$result = pg_query($sql);
echo "<table width=100% border=1 cellspacing=0><tr><td>Ip<td>Name<td>Total<td>Sent<td>Received<td>tcp<td>udp<td>icmp<td>http<td>p2p<td>ftp";
while ($r = pg_fetch_array($result))
	{
	echo "<tr><td><a href=#".$r['ip'].">";
	if ($r['ip'] == "0.0.0.0")
		echo "Total<td>Total Traffic";
	else
		echo $r['ip']."<td>".gethostbyaddr($r['ip']);
	echo "</a>";
	echo fmtb($r['total']).fmtb($r['sent']).fmtb($r['received']).
		fmtb($r['tcp']).fmtb($r['udp']).fmtb($r['icmp']).fmtb($r['http']).
		fmtb($r['p2p']).fmtb($r['ftp']);
	}
echo "</table></center>";

for($Counter=0; $Counter < pg_num_rows($result); $Counter++) 
	{
	$r = pg_fetch_array($result, $Counter);
	echo "<a name=".$r['ip']."><h3><a href=details.php?sensor_id=$sensor_id&ip=".$r['ip'].">";
	if ($r['ip'] == "0.0.0.0")
		echo "Total - Total of all subnets</h3>";
	else
		echo $r['ip']." - ".gethostbyaddr($r['ip'])."</h3>";
	echo "</a>";
	echo "Send:<br><img src=graph.php?ip=".$r['ip']."&interval=$interval&sensor_id=".$sensor_id."&table=bd_tx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
	echo "<img src=legend.gif><br>";
	echo "Receive:<br><img src=graph.php?ip=".$r['ip']."&interval=$interval&sensor_id=".$sensor_id."&table=bd_rx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
	echo "<img src=legend.gif><br>";
	}

include('footer.php');