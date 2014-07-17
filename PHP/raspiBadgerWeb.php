<?php
//
// raspiBadgerWeb.php (monitor activities of raspiBadgerComm)
// Started with control.php from
// http://www.instructables.com/id/Web-Control-of-Raspberry-Pi-GPIO
// for a web interface and completely changed the layout.
//
// TODO:  Add method to submit actions to specific badge numbers
// TODO:  Add method to update poke list pending actions.
//

session_start();
//////////////////////////////
// EDIT THESE TWO VARIABLES //
//////////////////////////////
$MySQLUsername = "USERNAME HERE";
$MySQLPassword = "PASSWORD HERE";

/////////////////////////////////
// DO NOT EDIT BELOW THIS LINE //
/////////////////////////////////
$MySQLHost = "localhost";
$MySQLDB = "raspiBadgerData";

If (($MySQLUsername == "USERNAME HERE") || ($MySQLPassword == "PASSWORD HERE")){
	print 'ERROR - Please set up the MySQLUsername/Password in raspiBadgerWeb first';
	exit();
}

$dbConnection = mysql_connect($MySQLHost, $MySQLUsername, $MySQLPassword);
mysql_select_db($MySQLDB, $dbConnection);
If (isset($_POST['action'])){
	If ($_POST['action'] == "setPassword"){
		$password1 = $_POST['password1'];
		$password2 = $_POST['password2'];
		If ($password1 != $password2){
			header('Location: raspiBadgerWeb.php');
		}
		$password = mysql_real_escape_string($_POST['password1']);
		If (strlen($password) > 28){
			mysql_close();
			header('location: raspiBadgerWeb.php');
		}
		$resetQuery = "SELECT username, salt FROM users WHERE username = 'admin';";
		$resetResult = mysql_query($resetQuery);
		If (mysql_num_rows($resetResult) < 1){
			mysql_close();
			header('location: raspiBadgerWeb.php');
		}
		$resetData = mysql_fetch_array($resetResult, MYSQL_ASSOC);
		$resetHash = hash('sha256', $salt . hash('sha256', $password));
		$hash = hash('sha256', $password);
		function createSalt(){
			$string = md5(uniqid(rand(), true));
			return substr($string, 0, 8);
		}
		$salt = createSalt();
		$hash = hash('sha256', $salt . $hash);
		mysql_query("UPDATE users SET salt='$salt' WHERE username='admin'");
		mysql_query("UPDATE users SET password='$hash' WHERE username='admin'");
		mysql_close();
		header('location: raspiBadgerWeb.php');
	}
}
If ((isset($_POST['username'])) && (isset($_POST['password']))){
	$username = mysql_real_escape_string($_POST['username']);
	$password = mysql_real_escape_string($_POST['password']);
	$loginQuery = "SELECT UserID, password, salt FROM users WHERE username = '$username';";
	$loginResult = mysql_query($loginQuery);
	If (mysql_num_rows($loginResult) < 1){
		mysql_close();
		header('location: raspiBadgerWeb.php?error=incorrectLogin');
	}
	$loginData = mysql_fetch_array($loginResult, MYSQL_ASSOC);
	$loginHash = hash('sha256', $loginData['salt'] . hash('sha256', $password));
	If ($loginHash != $loginData['password']){
		mysql_close();
		header('location: raspiBadgerWeb.php?error=incorrectLogin');
	} else {
		session_regenerate_id();
		$_SESSION['username'] = "admin";
		$_SESSION['userID'] = "1";
		mysql_close();
		header('location: raspiBadgerWeb.php');
	}
}
// Show Login sreen
If ((!isset($_SESSION['username'])) || (!isset($_SESSION['userID']))){
	print '
	<html>
	<head>
	<title>RaspiBadger - Login</title>
	</head>
	<body>
	<table border="0" align="center">
	<form name="login" action="raspiBadgerWeb.php" method="post">
	<tr>
	<td>Username: </td><td><input type="text" name="username"></td>
	</tr>
	<tr>
	<td>Password: </td><td><input type="password" name="password"></td>
	</tr>
	<tr>
	<td colspan="2" align="center"><input type="submit" value="Log In"></td>
	</tr>
	</form>
	</table>
	</body>
	</html>
	';
	die();
}
If (isset($_GET['action'])){
	If ($_GET['action'] == "logout"){
		$_SESSION = array();
		session_destroy();
		header('Location: raspiBadgerWeb.php');
	} else If ($_GET['action'] == "setPassword"){
		print '
		<form name="changePassword" action="raspiBadgerWeb.php" method="post">
		<input type="hidden" name="action" value="setPassword">
		<p>Enter New Password: <input type="password" name="password1">  Confirm: <input type="password" name="password2"><input type="submit" value="submit"></p>
		</form>
		';
	} else {  // ********************************start of code that needs work*******************************************
		$action = $_GET['action'];
		$badge = mysql_real_escape_string($_GET['badge']);
		if ($action == "poke"){
			// TODO: Add something here to send data to the arduino badges
			//       Possibly add values to a queue in mysql that is read by C++ program.
			$setting = "255";  // will adjust later (trigger vibration module?)
			mysql_query("INSERT INTO badgeAction (badgeID, action) VALUES ('$badge', '$setting');");
			mysql_close();
			header('Location: raspiBadgerWeb.php');
		} else {
			header('Location: raspiBadgerWeb.php');
		}
	}
} else {
	print '
		<html>
		<head>
		<title>RaspiBadger Control Panel</title>
		</head>
		<font face="verdana">
		<H1 align="center">RaspiBadger Monitor</H1>
		<table align="center">
		<tr>
		<td align="center" colspan="2">badge# |___| <-pokebtn-> force report</td>
		</tr><tr>
		<td align="center"><a href="raspiBadgerWeb.php?action=setPassword">Change Password</a></td>
		<td align="center"><a href="raspiBadgerWeb.php?action=logout">Log out</a></td>
		</tr>
		</table>
		<br>
	';
		// Get list of Badge ID numbers that have reported
		// TODO: use this same query to sort by MAX(ticksTotal)
		$query = mysql_query("SELECT badgeID, MAX(ticksTotal) AS totalTicks FROM badgeTick GROUP BY badgeID ORDER BY BadgeID;");
		$query2 = mysql_query("SELECT badgeID, COUNT(*) AS tickCount FROM badgeTick GROUP BY badgeID ORDER BY badgeID;");
		// count the number of distinct badges that have reported
		$totalBadgeCount = mysql_num_rows($query);
		$currentBadgeCount = 0;
		print '<table name="Badge Tick Report" border="1" cellpadding="5" align="center">';
		print '<tr><th>Badge #</th><th>Number of tick reports</th><th>Number of ticks the badge reported</th><th>Poke Badge (TODO)</th></tr>';
		while ($currentBadgeCount < $totalBadgeCount){
			$badgeRow = mysql_fetch_assoc($query);
			$ticksRow = mysql_fetch_assoc($query2);	
			$badgeNumber = $badgeRow['badgeID'];
			$tickCount = $ticksRow['tickCount'];
			$totalTicks = $badgeRow['totalTicks'];
			print '<tr>';
			print '<td align="center">' . $badgeNumber . '</td><td align="center">' . $tickCount . '</td><td align="center">' . $totalTicks . '</td><td align="center"><form name="badge' . $badgeNumber . '" action="raspiBadgerWeb.php" method="get"><input type="hidden" name="action" value="poke"><input type="hidden" name="badge" value="' . $badgeNumber . '"><input type="submit" value="Poke ' . $badgeNumber . '"></form></td>';
			print '</tr>';
			$currentBadgeCount ++;
		}
		print '</table><br><br>';
		// list any pending actions.  This should be cleared periodically by raspiBadgerComm.cpp
		// might want to add timestamps and delete pending actions that take too long to go through.
		print '<table name="Badge Pending Actions" border="1" cellpadding="5" align="center">
		<tr><th>Badge Number</th><th>Pending Action</th></tr>';
		$query3 = mysql_query("SELECT badgeID, action FROM badgeAction;");
		$totalBadgeActions = mysql_num_rows($query3);
		$currentBadgeCount = 0;
		while ($currentBadgeCount < $totalBadgeActions){
			$badgeRow = mysql_fetch_assoc($query3);
			$badgeNumber = $badgeRow['badgeID'];
			$badgeAction = $badgeRow['action'];
			print'<tr><td align="center">' . $badgeNumber . '</td><td align="center">' . $badgeAction . '</td></tr>';
			$currentBadgeCount ++;
		}
		print '</table><br>';
		mysql_close();
	print '
	</font>
	</html>
	';
}
?>
