<?include("include.php");?>
<html>
<center>
<img src=logo.gif>
<?
// Get variables from url

if (isset($_GET['sensor_id']))
    $sensor_id = $_GET['sensor_id'];
else
	{
	echo "<br>Please provide a sensor_id";
	exit(1);
	}

if (isset($_GET['interval']))
    $interval = $_GET['interval'];

if (isset($_GET['timestamp']))
    $timestamp = $_GET['timestamp'];

if (isset($_GET['subnet']))
    $subnet = $_GET['subnet'];

if (isset($_GET['limit']))
	$limit = $_GET['limit'];

$db = ConnectDb();
?>
</center>
<FORM name="navigation">
<table width=100% cellspacing=0 cellpadding=5 border=1>
<tr>
<td><SELECT name="sensors" onChange="window.location=document.navigation.sensors.options[document.navigation.sensors.selectedIndex].value">

<OPTION SELECTED value="index.php">--Select A Sensor--
<?
$sql = "SELECT sensor_id from bd_tx_log group by sensor_id order by sensor_id;";
$result = pg_query($sql);
$url="report.php?";
if (isset($limit))
    $url .= "&limit=$limit";
if (isset($subnet))
    $url .= "&subnet=$subnet";
if (isset($interval))
	$url .= "&interval=$interval";
while ($r = pg_fetch_array($result))
    echo "<option value=$url&sensor_id=".$r['sensor_id'].">".$r['sensor_id']."\n";
?>
</SELECT>
<td><SELECT name="intervals" onChange="window.location=document.navigation.intervals.options[document.navigation.intervals.selectedIndex].value">
<OPTION SELECTED value="index.php">--Select An Interval--
<?
$url="report.php?sensor_id=$sensor_id";
if (isset($limit))
	$url .= "&limit=$limit";
if (isset($subnet))
	$url .= "&subnet=$subnet";
?>
<OPTION value=<?=$url?>&interval=<?=INT_DAILY?>>Daily
<OPTION value=<?=$url?>&interval=<?=INT_WEEKLY?>>Weekly
<OPTION value=<?=$url?>&interval=<?=INT_MONTHLY?>>Monthly
<OPTION value=<?=$url?>&interval=<?=INT_YEARLY?>>Yearly
</select>

<td><SELECT name="limit" onChange="window.location=document.navigation.limit.options[document.navigation.limit.selectedIndex].value">
<OPTION SELECTED value="index.php">--How Many Results--
<?
$url="report.php?sensor_id=$sensor_id";
if (isset($interval))
    $url .= "&interval=$interval";
if (isset($subnet))
    $url .= "&subnet=$subnet";
?>
<OPTION value=<?=$url?>&limit=20>20
<OPTION value=<?=$url?>&limit=50>50
<OPTION value=<?=$url?>&limit=100>100
<OPTION value=<?=$url?>&limit=all>All
</select>

<?
$url="report.php?sensor_id=$sensor_id";
if (isset($limit))
    $url .= "&limit=$limit";
if (isset($interval))
    $url .= "&interval=$interval";
?>

<td>Subnet Filter:<input name=subnet value=<?=isset($subnet)?$subnet:"0.0.0.0/0"?>> 
<input type=button value="Filter" onclick="window.location='<?=$url?>&subnet='+document.navigation.subnet.value">
</table>
</FORM>
<center>
<?
// Set defaults
if (!isset($interval))
	$interval = DFLT_INTERVAL;

if (!isset($timestamp))
	$timestamp = time() - $interval + (0.05*$interval);

if (!isset($limit))
	$limit = 20;

if ($limit == "all")
	unset($limit);

if (isset($limit))
	echo "<h2>Top $limit - $sensor_id</h2>";
else
	echo "<h2>All Records - $sensor_id</h2>";

// Sqlize the incomming variables
if (isset($subnet))
	$subnet = "and ip <<= '$subnet'";

if (isset($limit))
	$limit = "limit $limit";

// Sql Statement
$sql = "select tx.ip, rx.scale as rxscale, tx.scale as txscale, tx.total+rx.total as total, tx.total as sent, 
rx.total as received, tx.tcp+rx.tcp as tcp, tx.udp+rx.udp as udp,
tx.icmp+rx.icmp as icmp, tx.http+rx.http as http,
tx.p2p+rx.p2p as p2p, tx.ftp+rx.ftp as ftp
from

(SELECT ip, max(total/sample_duration)*8 as scale, sum(total) as total, sum(tcp) as tcp, sum(udp) as udp, sum(icmp) as icmp,
sum(http) as http, sum(p2p) as p2p, sum(ftp) as ftp
from bd_tx_log
where sensor_id = '$sensor_id'
$subnet
and extract(epoch from timestamp) > $timestamp and extract(epoch from timestamp) < ".($timestamp+$interval)."
group by ip) as tx,

(SELECT ip, max(total/sample_duration)*8 as scale, sum(total) as total, sum(tcp) as tcp, sum(udp) as udp, sum(icmp) as icmp,
sum(http) as http, sum(p2p) as p2p, sum(ftp) as ftp
from bd_rx_log
where sensor_id = '$sensor_id'
$subnet
and extract(epoch from timestamp) > $timestamp and extract(epoch from timestamp) < ".($timestamp+$interval)."
group by ip) as rx

where tx.ip = rx.ip
order by total desc $limit;";

//echo "</center><pre>$sql</pre><center>";

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