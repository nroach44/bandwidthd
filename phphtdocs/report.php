<?include("include.php");?>
<html>
<center>
<img src=logo.gif>
<?
// Get variables from url

if (isset($_GET['sensor_name']))
    $sensor_name = $_GET['sensor_name'];
else
	{
	echo "<br>Please provide a sensor_name";
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
$sql = "SELECT sensor_name from sensors order by sensor_name;";
$result = pg_query($sql);
$url="report.php?";
if (isset($limit))
    $url .= "&limit=$limit";
if (isset($subnet))
    $url .= "&subnet=$subnet";
if (isset($interval))
	$url .= "&interval=$interval";
while ($r = pg_fetch_array($result))
    echo "<option value=$url&sensor_name=".$r['sensor_name'].">".$r['sensor_name']."\n";
?>
</SELECT>
<td><SELECT name="intervals" onChange="window.location=document.navigation.intervals.options[document.navigation.intervals.selectedIndex].value">
<OPTION SELECTED value="index.php">--Select An Interval--
<?
$url="report.php?sensor_name=$sensor_name";
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
$url="report.php?sensor_name=$sensor_name";
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
$url="report.php?sensor_name=$sensor_name";
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
	echo "<h2>Top $limit - $sensor_name</h2>";
else
	echo "<h2>All Records - $sensor_name</h2>";

// Sqlize the incomming variables
if (isset($subnet))
	$sql_subnet = "and ip <<= '$subnet'";

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
from sensors, bd_tx_log
where sensor_name = '$sensor_name'
and sensors.sensor_id = bd_tx_log.sensor_id
$sql_subnet
and timestamp > $timestamp::abstime and timestamp < ".($timestamp+$interval)."::abstime
group by ip) as tx,

(SELECT ip, max(total/sample_duration)*8 as scale, sum(total) as total, sum(tcp) as tcp, sum(udp) as udp, sum(icmp) as icmp,
sum(http) as http, sum(p2p) as p2p, sum(ftp) as ftp
from sensors, bd_rx_log
where sensor_name = '$sensor_name'
and sensors.sensor_id = bd_rx_log.sensor_id
$sql_subnet
and timestamp > $timestamp::abstime and timestamp < ".($timestamp+$interval)."::abstime
group by ip) as rx

where tx.ip = rx.ip
order by total desc $limit;";

//echo "</center><pre>$sql</pre><center>"; exit(0);
pg_query("SET sort_mem TO 30000;");
$result = pg_query($sql);
pg_query("set sort_mem to default;");
echo "<table width=100% border=1 cellspacing=0><tr><td>Ip<td>Name<td>Total<td>Sent<td>Received<td>tcp<td>udp<td>icmp<td>http<td>p2p<td>ftp";

if (!isset($subnet)) // Set this now for total graphs
	$subnet = "0.0.0.0/0";

// Output Total Line
echo "<TR><TD><a href=Total>Total</a><TD>$subnet";
foreach (array("total", "sent", "received", "tcp", "udp", "icmp", "http", "p2p", "ftp") as $key)
	{
	for($Counter=0, $Total = 0; $Counter < pg_num_rows($result); $Counter++)
		{
		$r = pg_fetch_array($result, $Counter);
		$Total += $r[$key];
		}
	echo fmtb($Total);
	}

// Output Other Lines
for($Counter=0; $Counter < pg_num_rows($result); $Counter++)
	{
	$r = pg_fetch_array($result, $Counter);
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

// Output Total Graph
for($Counter=0, $Total = 0; $Counter < pg_num_rows($result); $Counter++)
	{
	$r = pg_fetch_array($result, $Counter);
	$scale = max($r['txscale'], $scale);
	$scale = max($r['rxscale'], $scale);
	}

if ($subnet == "0.0.0.0/0")
	$total_table = "bd_tx_total_log";
else
	$total_table = "bd_tx_log";
echo "<a name=Total><h3><a href=details.php?sensor_name=$sensor_name&ip=$subnet>";
echo "Total - Total of $subnet</h3>";
echo "</a>";
echo "Send:<br><img src=graph.php?ip=$subnet&interval=$interval&sensor_name=".$sensor_name."&table=$total_table&yscale=$scale><br>";
echo "<img src=legend.gif><br>";
if ($subnet == "0.0.0.0/0")
	$total_table = "bd_rx_total_log";
else
	$total_table = "bd_rx_log";
echo "Receive:<br><img src=graph.php?ip=$subnet&interval=$interval&sensor_name=".$sensor_name."&table=$total_table&yscale=$scale><br>";
echo "<img src=legend.gif><br>";


// Output Other Graphs
for($Counter=0; $Counter < pg_num_rows($result); $Counter++) 
	{
	$r = pg_fetch_array($result, $Counter);
	echo "<a name=".$r['ip']."><h3><a href=details.php?sensor_name=$sensor_name&ip=".$r['ip'].">";
	if ($r['ip'] == "0.0.0.0")
		echo "Total - Total of all subnets</h3>";
	else
		echo $r['ip']." - ".gethostbyaddr($r['ip'])."</h3>";
	echo "</a>";
	echo "Send:<br><img src=graph.php?ip=".$r['ip']."&interval=$interval&sensor_name=".$sensor_name."&table=bd_tx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
	echo "<img src=legend.gif><br>";
	echo "Receive:<br><img src=graph.php?ip=".$r['ip']."&interval=$interval&sensor_name=".$sensor_name."&table=bd_rx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
	echo "<img src=legend.gif><br>";
	}

include('footer.php');