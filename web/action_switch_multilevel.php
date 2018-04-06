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

<h2>Switch Multilevel</h2>
<br>

<?php 

$level = $_POST["multilevelSwitch"];
echo "Will set multilevel dimmer to level ".$level;
$result = exec('ozw-proxy-client -n15 -v'.$level);

?>

<br>
<br>

<a href="/index.html">Back to main page</a>

</div>
</body>
</html>
