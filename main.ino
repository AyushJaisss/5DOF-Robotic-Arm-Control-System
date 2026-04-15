#include <WiFi.h>
#include <ESP32Servo.h>
#include <WebServer.h>

// ─── Configuration ────────────────────────────────────────────────────────────

#define NUM_SERVOS        5
#define DEFAULT_ANGLE     90
#define SERVO_MIN_US      500
#define SERVO_MAX_US      2400
#define SERVO_FREQ_HZ     50
#define SMOOTH_STEP       2      // degrees moved per loop tick toward target
#define SMOOTH_DELAY_MS   15     // ms between each interpolation step

const char* WIFI_SSID     = "ESP32_ArmControl";
const char* WIFI_PASSWORD = "arm12345";
const int   HTTP_PORT     = 80;

// GPIO pins assigned to each servo (0-indexed)
const int SERVO_PINS[NUM_SERVOS] = {13, 14, 15, 2, 17};

// Human-readable joint labels shown in the UI
const char* SERVO_LABELS[NUM_SERVOS] = {
  "Base Rotation",
  "Shoulder",
  "Elbow",
  "Wrist Pitch",
  "Gripper"
};

// ─── Data Structures ──────────────────────────────────────────────────────────

/**
 * Tracks the live state of a single servo.
 * `current` is what the physical servo is at right now.
 * `target`  is where it should smoothly move toward.
 */
struct ServoState {
  Servo   servo;
  int     current;   // actual physical position (degrees)
  int     target;    // desired position (degrees)
  int     pin;
  const char* label;
};

ServoState joints[NUM_SERVOS];

// ─── Global Objects ───────────────────────────────────────────────────────────

WebServer server(HTTP_PORT);

// ─── Servo Control ────────────────────────────────────────────────────────────

/**
 * Initialise all servos: attach, set frequency, move to default position.
 */
void initServos() {
  for (int i = 0; i < NUM_SERVOS; i++) {
    joints[i].pin     = SERVO_PINS[i];
    joints[i].label   = SERVO_LABELS[i];
    joints[i].current = DEFAULT_ANGLE;
    joints[i].target  = DEFAULT_ANGLE;

    joints[i].servo.setPeriodHertz(SERVO_FREQ_HZ);
    joints[i].servo.attach(joints[i].pin, SERVO_MIN_US, SERVO_MAX_US);
    joints[i].servo.write(DEFAULT_ANGLE);
  }
  Serial.println("[Servos] All joints initialised at 90°");
}

/**
 * Called every loop() tick.
 * Moves each servo one SMOOTH_STEP closer to its target angle.
 * Physical writes only happen when movement is needed.
 */
void updateServos() {
  for (int i = 0; i < NUM_SERVOS; i++) {
    if (joints[i].current != joints[i].target) {
      if (joints[i].current < joints[i].target) {
        joints[i].current = min(joints[i].current + SMOOTH_STEP, joints[i].target);
      } else {
        joints[i].current = max(joints[i].current - SMOOTH_STEP, joints[i].target);
      }
      joints[i].servo.write(joints[i].current);
    }
  }
}

/**
 * Schedule a servo to move to `angle` smoothly.
 * Returns false if id or angle is out of range.
 */
bool scheduleMove(int id, int angle) {
  if (id < 0 || id >= NUM_SERVOS) return false;
  if (angle < 0 || angle > 180)   return false;
  joints[id].target = angle;
  return true;
}

/**
 * Reset all servos to the default 90° position.
 */
void resetAllServos() {
  for (int i = 0; i < NUM_SERVOS; i++) {
    joints[i].target = DEFAULT_ANGLE;
  }
  Serial.println("[Servos] Reset all joints to 90°");
}

// ─── WiFi ─────────────────────────────────────────────────────────────────────

/**
 * Start ESP32 as a WiFi Access Point and print the AP IP to Serial.
 */
void initWiFi() {
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("[WiFi] Access Point: ");
  Serial.print(WIFI_SSID);
  Serial.print("  |  URL: http://");
  Serial.println(ip);
}

// ─── HTML / UI Generation ─────────────────────────────────────────────────────

/**
 * Build and return the full HTML control page.
 * Uses a card-grid layout with circular angle displays — visually distinct
 * from the original vertical slider stack.
 */
String buildHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>5DOF Arm Controller</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&family=Rajdhani:wght@400;600&display=swap');

    :root {
      --bg:        #0d0d14;
      --card:      #16161f;
      --border:    #2a2a3d;
      --accent:    #00e5ff;
      --accent2:   #ff4081;
      --text:      #e0e0e0;
      --muted:     #6b6b8a;
      --track:     #2a2a3d;
      --thumb:     #00e5ff;
    }

    * { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      background: var(--bg);
      font-family: 'Rajdhani', sans-serif;
      color: var(--text);
      min-height: 100vh;
      padding: 24px 16px 40px;
    }

    header {
      text-align: center;
      margin-bottom: 32px;
    }
    header h1 {
      font-family: 'Orbitron', monospace;
      font-size: 1.6rem;
      letter-spacing: 4px;
      color: var(--accent);
      text-shadow: 0 0 18px rgba(0,229,255,0.4);
    }
    header p {
      color: var(--muted);
      font-size: 0.85rem;
      margin-top: 6px;
      letter-spacing: 1px;
    }

    /* ── Joint Cards Grid ── */
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
      gap: 18px;
      max-width: 900px;
      margin: 0 auto 28px;
    }

    .card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 14px;
      padding: 20px 22px;
      position: relative;
      transition: border-color 0.25s;
    }
    .card:hover { border-color: var(--accent); }
    .card:hover .ring { stroke: var(--accent); }

    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: flex-start;
      margin-bottom: 16px;
    }
    .joint-id {
      font-family: 'Orbitron', monospace;
      font-size: 0.7rem;
      color: var(--muted);
      letter-spacing: 2px;
    }
    .joint-name {
      font-size: 1.1rem;
      font-weight: 600;
      color: var(--text);
      margin-top: 2px;
    }

    /* Circular angle badge */
    .angle-ring {
      width: 62px;
      height: 62px;
      flex-shrink: 0;
      position: relative;
    }
    .angle-ring svg {
      transform: rotate(-90deg);
    }
    .ring-bg { stroke: var(--track); }
    .ring { stroke: var(--accent2); transition: stroke-dashoffset 0.3s; }
    .angle-label {
      position: absolute;
      inset: 0;
      display: flex;
      align-items: center;
      justify-content: center;
      font-family: 'Orbitron', monospace;
      font-size: 0.75rem;
      font-weight: 700;
      color: var(--text);
      pointer-events: none;
    }

    /* Slider */
    .slider-row {
      display: flex;
      align-items: center;
      gap: 10px;
      margin-top: 8px;
    }
    .slider-label { font-size: 0.78rem; color: var(--muted); min-width: 22px; }

    input[type=range] {
      -webkit-appearance: none;
      appearance: none;
      flex: 1;
      height: 4px;
      background: var(--track);
      border-radius: 4px;
      outline: none;
      cursor: pointer;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 18px; height: 18px;
      border-radius: 50%;
      background: var(--thumb);
      border: 2px solid var(--bg);
      box-shadow: 0 0 8px rgba(0,229,255,0.6);
      transition: background 0.2s;
    }
    input[type=range]::-webkit-slider-thumb:hover { background: #fff; }

    /* Step buttons */
    .step-btns {
      display: flex;
      gap: 6px;
      margin-top: 12px;
    }
    .step-btn {
      flex: 1;
      padding: 6px 0;
      font-family: 'Rajdhani', sans-serif;
      font-size: 0.85rem;
      font-weight: 600;
      background: transparent;
      border: 1px solid var(--border);
      border-radius: 8px;
      color: var(--muted);
      cursor: pointer;
      transition: all 0.2s;
    }
    .step-btn:hover {
      border-color: var(--accent);
      color: var(--accent);
      box-shadow: 0 0 8px rgba(0,229,255,0.2);
    }

    /* ── Footer Controls ── */
    .footer-bar {
      max-width: 900px;
      margin: 0 auto;
      display: flex;
      gap: 14px;
      flex-wrap: wrap;
      justify-content: center;
    }

    .btn {
      padding: 13px 32px;
      font-family: 'Orbitron', monospace;
      font-size: 0.8rem;
      letter-spacing: 2px;
      border: none;
      border-radius: 10px;
      cursor: pointer;
      transition: all 0.2s;
    }
    .btn-reset {
      background: rgba(255,64,129,0.12);
      color: var(--accent2);
      border: 1px solid var(--accent2);
    }
    .btn-reset:hover {
      background: var(--accent2);
      color: #fff;
      box-shadow: 0 0 20px rgba(255,64,129,0.5);
    }

    .status-bar {
      text-align: center;
      margin-top: 20px;
      font-size: 0.78rem;
      color: var(--muted);
      font-family: 'Orbitron', monospace;
      letter-spacing: 1px;
    }
    .status-bar span { color: var(--accent); }
  </style>
</head>
<body>

<header>
  <h1>5DOF ARM CTRL</h1>
  <p>ESP32 · WIRELESS SERVO CONTROLLER · 5 JOINTS</p>
</header>

<div class="grid" id="jointGrid"></div>

<div class="footer-bar">
  <button class="btn btn-reset" onclick="resetAll()">⟳ RESET ALL TO 90°</button>
</div>

<div class="status-bar" id="statusBar">STATUS: <span>READY</span></div>

<script>
  const NUM = 5;
  const LABELS = ["Base Rotation","Shoulder","Elbow","Wrist Pitch","Gripper"];

  // Build joint cards dynamically
  const grid = document.getElementById('jointGrid');
  for (let i = 0; i < NUM; i++) {
    const circumference = 2 * Math.PI * 26;  // r=26
    grid.innerHTML += `
      <div class="card" id="card${i}">
        <div class="card-header">
          <div>
            <div class="joint-id">JOINT ${i}</div>
            <div class="joint-name">${LABELS[i]}</div>
          </div>
          <div class="angle-ring">
            <svg width="62" height="62" viewBox="0 0 62 62">
              <circle class="ring-bg" cx="31" cy="31" r="26"
                fill="none" stroke-width="5"/>
              <circle class="ring" id="ring${i}" cx="31" cy="31" r="26"
                fill="none" stroke-width="5"
                stroke-dasharray="${circumference}"
                stroke-dashoffset="${circumference * (1 - 90/180)}"
                stroke-linecap="round"/>
            </svg>
            <div class="angle-label" id="angleLabel${i}">90°</div>
          </div>
        </div>

        <div class="slider-row">
          <span class="slider-label">0°</span>
          <input type="range" id="slider${i}" min="0" max="180" value="90"
            oninput="onSlider(${i}, this.value)">
          <span class="slider-label">180°</span>
        </div>

        <div class="step-btns">
          <button class="step-btn" onclick="step(${i}, -10)">−10°</button>
          <button class="step-btn" onclick="step(${i},  -5)">−5°</button>
          <button class="step-btn" onclick="step(${i},  +5)">+5°</button>
          <button class="step-btn" onclick="step(${i}, +10)">+10°</button>
        </div>
      </div>`;
  }

  // ── Angle Ring Update ────────────────────────────────────────────────────────
  function setRing(id, angle) {
    const circumference = 2 * Math.PI * 26;
    const offset = circumference * (1 - angle / 180);
    document.getElementById('ring' + id).style.strokeDashoffset = offset;
    document.getElementById('angleLabel' + id).textContent = angle + '°';
    document.getElementById('slider' + id).value = angle;
  }

  // ── Send Command to ESP32 ────────────────────────────────────────────────────
  function sendServo(id, angle) {
    setStatus('MOVING JOINT ' + id + ' → ' + angle + '°');
    fetch('/setServo?id=' + id + '&angle=' + angle)
      .then(r => r.text())
      .then(() => setStatus('READY'))
      .catch(() => setStatus('ERROR — CHECK CONNECTION'));
  }

  function onSlider(id, val) {
    val = parseInt(val);
    setRing(id, val);
    sendServo(id, val);
  }

  function step(id, delta) {
    const slider = document.getElementById('slider' + id);
    const newVal = Math.max(0, Math.min(180, parseInt(slider.value) + delta));
    setRing(id, newVal);
    sendServo(id, newVal);
  }

  function resetAll() {
    setStatus('RESETTING ALL JOINTS...');
    fetch('/resetAll')
      .then(r => r.json())
      .then(data => {
        data.angles.forEach((a, i) => setRing(i, a));
        setStatus('ALL JOINTS RESET TO 90°');
      })
      .catch(() => setStatus('ERROR'));
  }

  function setStatus(msg) {
    document.getElementById('statusBar').innerHTML =
      'STATUS: <span>' + msg + '</span>';
  }

  // ── Poll live state every 500ms ──────────────────────────────────────────────
  function pollState() {
    fetch('/getState')
      .then(r => r.json())
      .then(data => {
        data.angles.forEach((a, i) => setRing(i, a));
      })
      .catch(() => {});
  }
  setInterval(pollState, 500);
</script>
</body>
</html>
)rawliteral";
  return html;
}

// ─── HTTP Route Handlers ──────────────────────────────────────────────────────

/** GET /  →  Serve the control UI */
void handleRoot() {
  server.send(200, "text/html", buildHTML());
}

/**
 * GET /setServo?id=<0-4>&angle=<0-180>
 * Schedules smooth movement to the requested angle.
 */
void handleSetServo() {
  if (!server.hasArg("id") || !server.hasArg("angle")) {
    server.send(400, "text/plain", "Missing parameters: id and angle required");
    return;
  }
  int id    = server.arg("id").toInt();
  int angle = server.arg("angle").toInt();

  if (!scheduleMove(id, angle)) {
    server.send(400, "text/plain", "Invalid id (0-4) or angle (0-180)");
    return;
  }

  Serial.printf("[HTTP] setServo: joint=%d target=%d°\n", id, angle);
  server.send(200, "text/plain", "OK");
}

/**
 * GET /getState
 * Returns a JSON object with the current physical angle of every joint.
 * Example: {"angles":[90,45,120,60,90]}
 */
void handleGetState() {
  String json = "{\"angles\":[";
  for (int i = 0; i < NUM_SERVOS; i++) {
    json += String(joints[i].current);
    if (i < NUM_SERVOS - 1) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

/**
 * GET /resetAll
 * Sends all servos back to 90° and returns the new state as JSON.
 */
void handleResetAll() {
  resetAllServos();
  String json = "{\"angles\":[";
  for (int i = 0; i < NUM_SERVOS; i++) {
    json += String(DEFAULT_ANGLE);
    if (i < NUM_SERVOS - 1) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

/** Catch-all 404 */
void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// ─── Route Registration ───────────────────────────────────────────────────────

void initRoutes() {
  server.on("/",          HTTP_GET, handleRoot);
  server.on("/setServo",  HTTP_GET, handleSetServo);
  server.on("/getState",  HTTP_GET, handleGetState);
  server.on("/resetAll",  HTTP_GET, handleResetAll);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("[HTTP] Server started");
}

// ─── Arduino Entry Points ─────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== 5DOF Robotic Arm Controller ===");

  initServos();
  initWiFi();
  initRoutes();
}

void loop() {
  // Handle incoming HTTP requests
  server.handleClient();

  // Incrementally move each servo toward its target
  static unsigned long lastSmoothTick = 0;
  if (millis() - lastSmoothTick >= SMOOTH_DELAY_MS) {
    lastSmoothTick = millis();
    updateServos();
  }
}
