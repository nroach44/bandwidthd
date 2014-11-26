<?php
include("include.php");
include("header.php");

if (isset($_GET['sensor_id']))
    $sensor_id = $_GET['sensor_id'];
else
    {
    echo "<br>Please provide a sensor_id";
    include('footer.php');
    exit(1);
    }

if (isset($_GET['ip']))
    $ip = pg_escape_string($_GET['ip']);
else
    {
    echo "<br>Please provide an ip address";
    include('footer.php');
    exit(1);
    }
                                                                                                                             
echo "<h3>";
if (strpos($ip, "/") === FALSE)
	echo "$ip - ".gethostbyaddr($ip)."</h3>";
else
	echo "Total - $ip</h3>";

$db = ConnectDb();

if ($ip == pg_escape_string("0.0.0.0/0"))
	{
    $rxtable = "bd_rx_total_log";
	$txtable = "bd_tx_total_log";
	}
else
	{
    $rxtable = "bd_rx_log";
	$txtable = "bd_tx_log";
	}

$sql = "select rx.scale as rxscale, tx.scale as txscale, tx.total+rx.total as total, tx.total as sent,
rx.total as received, tx.tcp+rx.tcp as tcp, tx.udp+rx.udp as udp,
tx.icmp+rx.icmp as icmp, tx.http+rx.http as http,
tx.p2p+rx.p2p as p2p, tx.ftp+rx.ftp as ftp
from
                                                                                                                             
(SELECT ip, max(total/sample_duration)*8 as scale, sum(total) as total, sum(tcp) as tcp, sum(udp) as udp, sum(icmp) as icmp,
sum(http) as http, sum(p2p) as p2p, sum(ftp) as ftp
from sensors, $txtable
where sensors.sensor_id = '$sensor_id'
and sensors.sensor_id = ".$txtable.".sensor_id
and ip <<= '$ip'
group by ip) as tx,
                                                                                                                             
(SELECT ip, max(total/sample_duration)*8 as scale, sum(total) as total, sum(tcp) as tcp, sum(udp) as udp, sum(icmp) as icmp,
sum(http) as http, sum(p2p) as p2p, sum(ftp) as ftp
from sensors, $rxtable
where sensors.sensor_id = '$sensor_id'
and sensors.sensor_id = ".$rxtable.".sensor_id
and ip <<= '$ip'
group by ip) as rx
                                                                                                                             
where tx.ip = rx.ip;";
//echo "</center><pre>$sql</pre><center>";exit(0);
$result = pg_query($sql);
?>

<table width="100%" border=1 cellspacing=0>
<tr>
<th>Ip</th><th>Name</th>
<th>Total</th><th>Sent</th><th>Received</th>
<th>tcp</th><th>udp</th><th>icmp</th>
<th>http</th><th>p2p</th><th>ftp</th>
</tr>

<?php

$r = pg_fetch_array($result);

echo "<tr><td>";
if (strpos($ip, "/") === FALSE)
	echo "$ip</td><td>".gethostbyaddr($ip) . "</td>";
else
	echo "Total</td><td>$ip</td>";

echo fmtb($r['total']).fmtb($r['sent']).fmtb($r['received']).
	fmtb($r['tcp']).fmtb($r['udp']).fmtb($r['icmp']).fmtb($r['http']).
    fmtb($r['p2p']).fmtb($r['ftp']);
echo "</table>";

echo "<center><h3>Daily</h3></center>";
echo "Send:<br><img src=\"graph.php?ip=$ip&amp;sensor_id=".$sensor_id."&amp;table=$txtable&amp;yscale=".(max($r['txscale'], $r['rxscale']))."\"><br>";
echo '<img src="legend.gif"><br>';
echo "Receive:<br><img src=\"graph.php?ip=$ip&amp;sensor_id=".$sensor_id."&amp;table=$rxtable&amp;yscale=".(max($r['txscale'], $r['rxscale']))."\"><br>";
echo '<img src="legend.gif"><br>';

echo "<center><h3>Weekly</h3></center>";
echo "Send:<br><img src=\"graph.php?interval=".INT_WEEKLY."&amp;ip=$ip&amp;sensor_id=$sensor_id&amp;table=$txtable&amp;yscale=".(max($r['txscale'], $r['rxscale']))."\"><br>";
echo '<img src="legend.gif"><br>';
echo "Receive:<br><img src=\"graph.php?interval=".INT_WEEKLY."&amp;ip=$ip&amp;sensor_id=$sensor_id&amp;table=$rxtable&amp;yscale=".(max($r['txscale'], $r['rxscale']))."\"><br>";
echo '<img src="legend.gif"><br>';

echo "<center><h3>Monthly</h3></center>";
echo "Send:<br><img src=\"graph.php?interval=".INT_MONTHLY."&amp;ip=$ip&amp;sensor_id=$sensor_id&amp;table=$txtable&amp;yscale=".(max($r['txscale'], $r['rxscale']))."\"><br>";
echo '<img src="legend.gif"><br>';
echo "Receive:<br><img src=\"graph.php?interval=".INT_MONTHLY."&amp;ip=$ip&amp;sensor_id=$sensor_id&amp;table=$rxtable&amp;yscale=".(max($r['txscale'], $r['rxscale']))."\"><br>";
echo '<img src="legend.gif"><br>';

echo "<center><h3>Yearly</h3></center>";
echo "Send:<br><img src=\"graph.php?interval=".INT_YEARLY."&amp;ip=$ip&amp;sensor_id=$sensor_id&amp;table=$txtable&amp;yscale=".(max($r['txscale'], $r['rxscale']))."\"><br>";
echo '<img src="legend.gif"><br>';
echo "Receive:<br><img src=\"graph.php?interval=".INT_YEARLY."&amp;ip=$ip&amp;sensor_id=$sensor_id&amp;table=$rxtable&amp;yscale=".(max($r['txscale'], $r['rxscale']))."\"><br>";
echo '<img src="legend.gif"><br>';

include('footer.php');
?>
