<html>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
.content{
    width: 50%;
    text-align: center;
    margin: auto;
}
</style>
<body>
<div class="content">

<h2>Switch Binary </h2>
<br>

<?php
$status = 0;
if (isset($_POST["binarySwitch"])) {
    $status = 1;
    echo "Switch will be turned on.";
} else {
    $status = 0;
    echo "Switch will be turned off.";
}
$result = exec('/usr/bin/ozw-proxy-client -n21 -v'.$status);
?>

<br>
<br>

<a href="/index.html">Back to main page</a>

</div>
</body>
</html>
