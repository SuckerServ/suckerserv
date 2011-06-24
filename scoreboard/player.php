<?php
///////////////////////Player Details Page
session_start();
include("includes/geoip.inc");
include("includes/hopmod.php");

startbench();
$querydate = "month";
if ( isset($_GET['querydate']) ) {
	if ($_GET['querydate'] != "day" | "week" | "month" | "year") { $querydate = "month";} else { $querydate = $_GET['querydate']; }
}
if ( isset($_GET['showprofile']) ) {
	$profile_name = "and name='".sqlite_escape_string($_GET['showprofile'])."' ";
}
if ( isset($_GET['name']) ) {
}
check_get();
// Setup Geoip for location information.
$gi = geoip_open("/usr/share/GeoIP/GeoIP.dat",GEOIP_STANDARD);

// Pull Variables from Running Hopmod Server
serverDetails($serverhost, $serverport);
// Setup statsdb and assign it to an object.
$dbh = setup_pdo_statsdb($db);

$start_date = date("Y");
$start_date = strtotime("1 January $start_date");


// Setup main sqlite query.
$sql = "select name,
                ipaddr,
                sum(score) as TotalScored,
                sum(teamkills) as TotalTeamkills,
                max(frags) as MostFrags,
                sum(frags) as TotalFrags,
                sum(deaths) as TotalDeaths,
                count(name) as TotalGames,
                round((0.0+sum(hits))/(sum(hits)+sum(misses))*100) as Accuracy,
                round((0.0+sum(frags))/sum(deaths),2) as Kpd
        from players
                inner join games on players.game_id=games.id
        where games.datetime > $start_date and name = '".addslashes($_SESSION['name'])."' group by name";

$last_10 = "
select games.id as id,datetime,gamemode,mapname,duration,players,servername
        from games
                inner join players on players.game_id=games.id

        where games.datetime > $start_date and name = '".addslashes($_SESSION['name'])."' order by datetime desc limit ".$_SESSION['paging'].",".$rows_per_page." 
";
$pager_query = "
select count(*) from 
(select games.id as id,datetime,gamemode,mapname,duration,players
        from games
                inner join players on players.game_id=games.id

        where games.datetime > $start_date and name = '".addslashes($_SESSION['name'])."') T
";
?>
<html>
<head>
	<title><?php print $server_title; ?> scoreboard</title>
	<script type="text/javascript" src="js/overlib.js"><!-- overLIB (c) Erik Bosrup --></script>
	<script type="text/javascript" src="js/jquery-latest.js"></script>
	<script type="text/javascript" src="js/jquery.tablesorter.js"></script>
	<script type="text/javascript" src="js/jquery.uitablefilter.js"></script>
	<script type="text/javascript" src="js/hopstats.js"></script>
	<link rel="stylesheet" type="text/css" href="css/style.css" />
</head>
<body>
<noscript><div class="error">This page uses JavaScript for table column sorting and producing an enhanced tooltip display.</div></noscript>



<div id="content-container">
<div id="content"><div style="">
<h1><?php print $_SESSION['name'] ?>'s profile</h1>

<div class="box" style="position:absolute">
<table class="navbar" cellpadding="0" cellspacing="1">
<?php
//Build table data
foreach ($dbh->query($sql) as $row)
{
		$country = geoip_country_name_by_addr($gi, $row['ipaddr']);
		$code = geoip_country_code_by_addr($gi, $row['ipaddr']);
		if (isset($code)) {
			$code = strtolower($code) . ".png";
			$flag_image = "<img src=images/flags/$code />";
		}
        	print "
				<tr>
					<td style=\"width:100px;\" class=\"headcol\">Name</td>
					<td align=\"center\">$row[name]</td>
				</tr>
				";
				?>
				<tr>
					<td style="width:100px;" class="headcol">Country</td>
					<td align="center"><?php overlib($country); print $flag_image ?></a></td>
				</tr>
				<?php

		print "
				<tr>
                                <td style=\"width:100px;\" class=\"headcol\">Most Frags</td>
				<td align=\"center\">$row[MostFrags]</td>
				</tr>
				<tr>
                                <td style=\"width:100px;\" class=\"headcol\">Total Frags</td>
				<td align=\"center\">$row[TotalFrags]</td>
				</tr>
				<tr>
                                <td style=\"width:100px;\" class=\"headcol\">Total Deaths</td>
				<td align=\"center\">$row[TotalDeaths]</td>
				</tr>
				<tr>
                                <td style=\"width:100px;\" class=\"headcol\">Accuracy</td>
				<td align=\"center\">$row[Accuracy]</td>	
				</tr>
				<tr>
                                <td style=\"width:100px;\" class=\"headcol\">KpD</td>
				<td align=\"center\">$row[Kpd]</td>
				</tr>
				<tr>
                                <td style=\"width:100px;\" class=\"headcol\">Team Kills</td>
				<td align=\"center\">$row[TotalTeamkills]</td>	
				</tr>
				<tr>
                                <td style=\"width:100px;\" class=\"headcol\">Total Games</td>
				<td align=\"center\">$row[TotalGames]</td>
				</tr>
        		";
	$flag_image ="";
}
?>
</table>
</div>






<div style="margin-left:300px">

<a name="gm"></a>

<h2>Game history</h2>

<?php match_player_table($dbh->query($last_10)); //Build game table data ?>
<div style="margin-left:10%;width:600px; overflow: hidden" id="pagebar">
<?php build_pager($_GET['page'],$pager_query,$rows_per_page); //Generate Pager Bar ?>
</div>
</div>
</div>
</div>
<div id="footer">
</div>
</div>
<?php stopbench(); ?>
</body>
</html>
