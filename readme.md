# 🤖 5DOF-Robotic-Arm-Control-System

![Arduino](https://img.shields.io/badge/Arduino-IDE-00979D?style=flat&logo=arduino&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-NodeMCU--32-E7352C?style=flat&logo=espressif&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-green?style=flat)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue?style=flat)

> A fully self-contained robotic arm controller powered by an ESP32 microcontroller, featuring a real-time browser-based GUI served directly over Wi-Fi — no internet or router required.

---

## 📌 Features

- Control **5 servo motors** (SG90 and MG995) via ESP32 PWM signals
- **Built-in web interface** served directly from the ESP32 over its own Wi-Fi Access Point
- **Slider-based GUI** for real-time servo angle adjustment (0°–180°)
- Pure **HTML/CSS/JavaScript** frontend — lightweight and responsive
- **Fully self-contained** — no cloud, no router, no external dependencies

---

## 🧰 Hardware Components

| Component | Description |
|---|---|
| ESP32 (NodeMCU-32, 38 Pin) | Microcontroller with built-in Wi-Fi |
| SG90 Servo Motors ×3 | Lightweight servos for small joints |
| MG995 Servo Motors ×2 | High-torque servos for base and heavy loads |
| GL12 840-Point Breadboard | Solderless prototyping board |
| Jumper Wires | Male-to-male and male-to-female |
| External Power Supply | Powers servos independently from ESP32 |
| Ice-Cream Sticks | DIY arm structure frame |

> ⚠️ **Power Warning:** Always power servo motors using a **dedicated external 5V supply**. The ESP32 cannot supply sufficient current for multiple servos and may be damaged if overloaded.

---

## 💾 Software & Libraries

### 🔧 Arduino Libraries

Install the following via Arduino IDE Library Manager:

1. **ESP32Servo**
   - Enables PWM-based servo control on ESP32 GPIO pins
   - Install via Library Manager or [GitHub](https://github.com/madhephaestus/ESP32Servo)

2. **WiFi.h**
   - Bundled with the ESP32 Arduino core
   - Configures the ESP32 in Access Point (AP) mode

3. **WebServer.h**
   - Also included in the ESP32 core
   - Handles HTTP requests and serves the embedded HTML interface

---

## 🌐 How the Web Control Works

### 1. Access Point (AP) Mode
The ESP32 broadcasts its own Wi-Fi network. Connect any device to it using:
- **SSID:** `ESP32_Servo_AP`
- **Password:** `12345678`

Once connected, open a browser and navigate to `http://192.168.4.1`.

### 2. Embedded HTML Interface
The full HTML/CSS/JS page is stored directly in the ESP32 firmware. It includes:
- A slider for each of the 5 servos (range: 0°–180°)
- JavaScript `fetch()` calls that instantly send angle updates to the ESP32 server

### 3. Dynamic Slider Rendering
The firmware generates the HTML page dynamically using a `%SLIDER_BLOCKS%` placeholder, which is replaced at runtime with individual slider blocks for each servo.

### 4. HTTP GET Control
Every slider movement triggers a URL call like:

```
GET /setServo?id=3&angle=120
```

The ESP32 parses the `id` and `angle` parameters and commands the corresponding servo.

---

## 🧠 Code Overview

| Function | Description |
|---|---|
| `setup()` | Initializes servos, starts the Wi-Fi AP, and launches the web server |
| `loop()` | Listens for incoming HTTP client requests |
| `handleRoot()` | Generates and serves the main slider UI |
| `handleSetServo()` | Parses angle updates and moves the target servo |

---

## 📸 Assembly Notes

Build the arm frame using ice-cream sticks and hot glue:

1. **Base joint** — MG995 (high torque, handles full arm rotation)
2. **Shoulder joint** — MG995
3. **Elbow, wrist, gripper joints** — SG90 (×3)

Secure each servo firmly at its joint before connecting wires. Route all servo signal wires back to the ESP32 GPIO pins.

---

## 📶 Getting Started

1. Open the project in **Arduino IDE**
2. Install all required libraries (see above)
3. Select your ESP32 board and correct COM port
4. Upload the sketch to the ESP32
5. Connect your phone or laptop to Wi-Fi:
   - **SSID:** `ESP32_Servo_AP`
   - **Password:** `12345678`
6. Open a browser and visit: **http://192.168.4.1**
7. Use the sliders to control each servo in real time

---

## 🛡️ Safety & Power Tips

- ❌ Do **not** power servos from the ESP32's 3.3V or 5V pins
- ✅ Use a **dedicated 5V power supply** with adequate current (≥2A recommended)
- ✅ Always connect the **GND of the external supply to the ESP32 GND**
- ⚠️ MG995 motors can draw **1–2A under load** — ensure your supply can handle it
- Keep wiring tidy to avoid accidental shorts during movement

---

## 🔮 Future Improvements

- [ ] Add **position feedback** using potentiometers or encoders on each joint
- [ ] Implement **inverse kinematics** for precise end-effector positioning
- [ ] Add **pose memory** — save and recall servo angle presets
- [ ] Enable **sequence recording and playback** for automated motion
- [ ] Upgrade the UI with **real-time angle display** and load indicators
- [ ] Add **OTA firmware updates** over the same Wi-Fi AP
---

📜 Acknowledgment
This project was initially developed as a team effort and has been further modified and enhanced with additional features.

---

## 🧾 License

This project is released under the [MIT License](LICENSE).

---

*Built with ESP32, servo motors, ice-cream sticks, and a lot of patience. 🍦🤖*
