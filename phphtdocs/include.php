<?
define("DFLT_WIDTH", 900);
define("DFLT_HEIGHT", 256);

define("INT_DAILY", 60*60*24*2);
define("INT_WEEKLY", 60*60*24*8);
define("INT_MONTHLY", 60*60*24*35);
define("INT_YEARLY", 60*60*24*400);

define("DFLT_INTERVAL", INT_DAILY); 

define("XOFFSET", 90);
define("YOFFSET", 45);

function ConnectDb()
    {
    $db = pg_pconnect("user = derby dbname = bandwidthd");
    if (!$db)
        {
        printf("DB Error, could not connect to database");
        exit(1);
        }
    return($db);
    }
                                                                                                                             
function fmtb($bytes)
	{
	$Max = 1024;
	$Output = $bytes;

	if ($Output > $Max)
		{
		$Output /= 1024;
		$Suffix = 'K';
		}

	if ($Output > $Max)
		{
		$Output /= 1024;
		$Suffix = 'M';
		}

	if ($Output > $Max)
		{
		$Output /= 1024;
		$Suffix = 'G';
		}

	return(sprintf("<td align=right><tt>%.1f%s</td>", $Output, $Suffix));
	}

$starttime = time();
set_time_limit(300);
?>