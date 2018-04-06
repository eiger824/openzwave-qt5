<html>
<body>

<h2>Switch Multilevel</h2>
<br>

<?php 

$level = $_POST["multilevelSwitch"];
$result = exec('ozw-proxy-client -n15 -v'.$level);

?>

<br>

<a href="/index.html">Back to main page</a>

</body>
</html>
