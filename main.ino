#include <WiFi.h>
#include <ESP32Servo.h>
#include <WebServer.h>

// WiFi Access Point credentials
const char* ssid = "ESP32_Servo_AP";
const char* password = "12345678";

// Create Servo objects
Servo servos[5];

// GPIO pins for the 5 servos
const int servoPins[5] = {13, 14, 15, 2 , 17};

// Create Web server on port 80
WebServer server(80);

// Enhanced HTML UI
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 5-Servo Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f2f2f2;
      text-align: center;
      padding: 20px;
    }
    h2 {
      color: #333;
    }
    .slider-container {
      background: white;
      padding: 15px;
      border-radius: 10px;
      box-shadow: 0 0 10px rgba(0,0,0,0.1);
      margin: 20px auto;
      max-width: 400px;
    }
    input[type=range] {
      width: 100%;
    }
    .angle-display {
      font-weight: bold;
      margin-top: 5px;
    }
  </style>
</head>
<body>
  <h2>ESP32 - 5 Servo Web Control</h2>

  %SLIDER_BLOCKS%

  <script>
    function updateServo(id, angle) {
      document.getElementById('angle' + id).textContent = angle;
      fetch(`/setServo?id=${id}&angle=${angle}`);
    }
  </script>
</body>
</html>
)rawliteral";

// Template to insert sliders dynamically
const char* sliderBlock = R"rawliteral(
  <div class="slider-container">
    <label for="servo{id}">Servo {id}</label>
    <input type="range" id="servo{id}" min="0" max="180" value="90"
           oninput="updateServo({id}, this.value)">
    <div class="angle-display">Angle: <span id="angle{id}">90</span>°</div>
  </div>
)rawliteral";

// Replace placeholder with actual sliders
String generateHTML() {
  String page = htmlPage;
  String sliders = "";
  for (int i = 0; i < 5; i++) {
    String block = sliderBlock;
    block.replace("{id}", String(i));
    block.replace("{id}", String(i)); // second replacement
    sliders += block;
  }
  page.replace("%SLIDER_BLOCKS%", sliders);
  return page;
}

// Route: Main page
void handleRoot() {
  server.send(200, "text/html", generateHTML());
}

// Route: Set individual servo angle
void handleSetServo() {
  if (server.hasArg("id") && server.hasArg("angle")) {
    int id = server.arg("id").toInt();
    int angle = server.arg("angle").toInt();

    if (id >= 0 && id < 5 && angle >= 0 && angle <= 180) {
      servos[id].write(angle);
      Serial.printf("Servo %d set to %d°\n", id, angle);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Invalid servo ID or angle");
    }
  } else {
    server.send(400, "text/plain", "Missing parameters");
  }
}

void setup() {
  Serial.begin(115200);

  // Attach all servos
  for (int i = 0; i < 5; i++) {
    servos[i].setPeriodHertz(50);
    servos[i].attach(servoPins[i], 500, 2400);
    servos[i].write(90); // Initial position
  }

  // Start WiFi Access Point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point started at http://");
  Serial.println(IP);

  // Define routes
  server.on("/", handleRoot);
  server.on("/setServo", handleSetServo);
  server.begin();
}

void loop() {
  server.handleClient();
}
