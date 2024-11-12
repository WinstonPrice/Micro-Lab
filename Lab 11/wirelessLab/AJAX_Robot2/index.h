const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<div id="demo">
<h1>CEC 326 Robot interface without refresh via AJAX</h1>
  <button type="button" onclick="sendData(1)">LED ON</button>
  <button type="button" onclick="sendData(0)">LED OFF</button><BR>
  <input type="range" min="1" max="180" value="90" class="slider" id="servoControl">
  <label for="servoControl">Servo Position</label><br>
  <input type="color" id="neocolor" name="favcolor" value="#0000ff">
  <label for="neocolor">NeoPixel Color</label><br>
</div>

<div>
  Distance is : <span id="DistVal">0</span><br>
    LED State is : <span id="LEDState">NA</span>
</div>
<script>
var colorP = document.getElementById("neocolor");
colorP.onchange = function() {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "N="+this.value.substring(1), true);
  xhttp.send();
}

var slider = document.getElementById("servoControl");
slider.oninput = function() {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "S="+this.value, true);
  xhttp.send();
}

function sendData(led) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("LEDState").innerHTML =
      this.responseText;
    }
  };
  if(led) {
    xhttp.open("GET", "H", true);
  } else {
    xhttp.open("GET", "L", true);
  }
  xhttp.send();
}

setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getData();
}, 2000); //2000mSeconds update rate

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("DistVal").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "getDist", true);
  xhttp.send();
}
</script>
</body>
</html>
)=====";
