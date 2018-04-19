<html>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
.content{
    width: 50%;
    text-align: center;
    margin: auto;
}
#myDIV {
    width: 100%;
    text-align: center;
    background-color: #e8e8e8;
    margin-top: 20px;
    display: none;
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
$result = shell_exec('/usr/bin/ozw-proxy-client -n21 -v'.$status.' 2>&1');
$tmpfile = fopen(".status", "w");
fwrite($tmpfile, $result);
fclose($tmpfile);
?>

<br><br>

<fieldset>
    <button id="myButton" onclick="showAndHide()">(Show Info)</button>
    <div id="myDIV" display="none">
<?php
    $tmpfile = fopen(".status", "r");
    echo fgets($tmpfile);
    fclose($tmpfile);
?>
    </div>
</fieldset>

<br>

<script>
function showAndHide() {
    var x = document.getElementById("myDIV");
    var y = document.getElementById("myButton");
    if (x.style.display === "none") {
        x.style.display = "block";
        y.innerHTML = "(Hide Info)";
    } else {
        x.style.display = "none";
        y.innerHTML = "(Show Info)";
    }
}
</script>

<br>
<br>

<a href="/home.html">Back to main page</a>

</div>
</body>
</html>
