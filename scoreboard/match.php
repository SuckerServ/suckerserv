<?php
include("includes/geoip.inc");
include("includes/hopmod.php");

$day_end = strtotime("today +1 day");
$week_end = strtotime("this week today +1 week");
$month_end = strtotime("this month today +1 month");
$year_end = strtotime("today");

$day_start = strtotime("today");
$week_start = strtotime("this week today");
$month_start = strtotime("this month today");
$year_start = strtotime("-365 days");

$_SESSION['start_date'] = ${$_SESSION['querydate']."_start"};
$_SESSION['end_date'] = ${$_SESSION['querydate']."_end"};

// Start session for session vars
session_start();
        $sql = "
select 	name,
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
where game_id = '".$_GET['id']."' group by name order by frags desc
";

// Start page benchmark
startbench();

// Check for any http GET activity
check_get();
// Pull Variables from Running Hopmod Server
serverDetails($serverhost, $serverport);
// Setup statsdb and assign it to an object.
$dbh = setup_pdo_statsdb($db);

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
</div>
<?php match_table($_GET['id']); //Build stats table data ?>
<?php stats_table($sql); ?>
<br />
<?php stopbench(); //Stop and display benchmark.?>
</body>
</html>












