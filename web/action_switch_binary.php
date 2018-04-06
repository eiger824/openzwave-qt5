<html>
<body>

<h2>Switch Binary </h2>
<br>

<?php
$status = 0;
if (isset($_POST["binarySwitch"])) {
    $status = 1;
    echo "Switch will be turned on";
} else {
    $status = 0;
    echo "Switch will be turned off";
}
$result = exec('ozw-proxy-client -n21 -v'.$status);
?>

<br>

<a href="/index.html">Back to main page</a>

</body>
</html>
