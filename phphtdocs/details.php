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

if (isset($_GET['ip']))
    $ip = $_GET['ip'];
else
    {
    echo "<br>Please provide an ip address";
    exit(1);
    }
                                                                                                                             
echo "<h3>";
if ($ip == "0.0.0.0")
	echo "Total - Total of all subnets</h3>";
else
	echo "$ip - ".gethostbyaddr($ip)."</h3>";

$db = ConnectDb();

$sql = "select rx.scale as rxscale, tx.scale as txscale, tx.total+rx.total as total, tx.total as sent,
rx.total as received, tx.tcp+rx.tcp as tcp, tx.udp+rx.udp as udp,
tx.icmp+rx.icmp as icmp, tx.http+rx.http as http,
tx.p2p+rx.p2p as p2p, tx.ftp+rx.ftp as ftp
from
                                                                                                                             
(SELECT ip, max(total/sample_duration)*8 as scale, sum(total) as total, sum(tcp) as tcp, sum(udp) as udp, sum(icmp) as icmp,
sum(http) as http, sum(p2p) as p2p, sum(ftp) as ftp
from bd_tx_log
where sensor_id = '$sensor_id' and
ip = '$ip'
group by ip) as tx,
                                                                                                                             
(SELECT ip, max(total/sample_duration)*8 as scale, sum(total) as total, sum(tcp) as tcp, sum(udp) as udp, sum(icmp) as icmp,
sum(http) as http, sum(p2p) as p2p, sum(ftp) as ftp
from bd_rx_log
where sensor_id = '$sensor_id' and
ip = '$ip'
group by ip) as rx
                                                                                                                             
where tx.ip = rx.ip;";
//echo $sql;
$result = pg_query($sql);
echo "<table width=100% border=1 cellspacing=0><tr><td>Ip<td>Name<td>Total<td>Sent<td>Received<td>tcp<td>udp<td>icmp<td>http<td>p2p<td>ftp";
$r = pg_fetch_array($result);
echo "<tr><td>";
if ($ip == "0.0.0.0")
	echo "Total<td>Total Traffic";
else
	echo "$ip<td>".gethostbyaddr($ip);
echo fmtb($r['total']).fmtb($r['sent']).fmtb($r['received']).
	fmtb($r['tcp']).fmtb($r['udp']).fmtb($r['icmp']).fmtb($r['http']).
    fmtb($r['p2p']).fmtb($r['ftp']);
echo "</table></center>";



echo "<center><h4>Daily</h4></center>";
echo "Send:<br><img src=graph.php?ip=$ip&sensor_id=".$sensor_id."&table=bd_tx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
echo "<img src=legend.gif><br>";
echo "Receive:<br><img src=graph.php?ip=$ip&sensor_id=".$sensor_id."&table=bd_rx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
echo "<img src=legend.gif><br>";

echo "<center><h4>Weekly</h4></center>";
echo "Send:<br><img src=graph.php?interval=".INT_WEEKLY."&ip=$ip&sensor_id=$sensor_id&table=bd_tx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
echo "<img src=legend.gif><br>";
echo "Receive:<br><img src=graph.php?interval=".INT_WEEKLY."&ip=$ip&sensor_id=$sensor_id&table=bd_rx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
echo "<img src=legend.gif><br>";

echo "<center><h4>Monthly</h4></center>";
echo "Send:<br><img src=graph.php?interval=".INT_MONTHLY."&ip=$ip&sensor_id=$sensor_id&table=bd_tx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
echo "<img src=legend.gif><br>";
echo "Receive:<br><img src=graph.php?interval=".INT_MONTHLY."&ip=$ip&sensor_id=$sensor_id&table=bd_rx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
echo "<img src=legend.gif><br>";

echo "<center><h4>Yearly</h4></center>";
echo "Send:<br><img src=graph.php?interval=".INT_YEARLY."&ip=$ip&sensor_id=$sensor_id&table=bd_tx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
echo "<img src=legend.gif><br>";
echo "Receive:<br><img src=graph.php?interval=".INT_YEARLY."&ip=$ip&sensor_id=$sensor_id&table=bd_rx_log&yscale=".(max($r['txscale'], $r['rxscale']))."><br>";
echo "<img src=legend.gif><br>";