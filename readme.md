---
# 🤖 5DOF Robotic Arm Controlled via ESP32 Web Interface

This project demonstrates a **5 Degrees of Freedom (5DOF) robotic arm** operated using an **ESP32** microcontroller. The ESP32 hosts an embedded HTML web interface served over its own Wi-Fi Access Point (AP), allowing real-time control of five servo motors via sliders.
---

## 📌 Features

- Control **5 servo motors** (SG90 and MG995) using ESP32.
- Built-in **web interface** served directly by the ESP32 via Wi-Fi AP.
- Interactive slider-based GUI to adjust servo angles in real-time.
- Uses **native HTML/CSS/JavaScript** for responsive UI.
- Fully self-contained — **no internet connection or router required**.

---

## 🧰 Hardware Components

| Component                      | Description                           |
| ------------------------------ | ------------------------------------- |
| **ESP32 (NodeMCU-32, 38 Pin)** | Microcontroller with built-in Wi-Fi   |
| **SG90 Servo Motors ×3**       | Lightweight servo for small joints    |
| **MG995 Servo Motors ×2**      | High-torque servo for base/heavy load |
| **GL12 840-Point Breadboard**  | Solderless prototyping board          |
| **Jumper Wires**               | Male-to-male and male-to-female wires |
| **External Power Supply**      | To power servos independently         |
| **Ice-Cream Sticks**           | DIY arm structure                     |

> ⚠️ **Note**: It is highly recommended to power the servo motors with an external 5V power source. The ESP32 alone cannot provide enough current to drive multiple servos reliably.

---

## 💾 Software & Libraries

### 🔧 Arduino Libraries

Make sure the following libraries are installed in the Arduino IDE:

1. **ESP32Servo**

   - For PWM control of servo motors using ESP32.
   - Install via Library Manager or GitHub: [ESP32Servo](https://github.com/madhephaestus/ESP32Servo)

2. **WiFi.h**

   - Built into the ESP32 board package. Used to configure the device in **Access Point mode**.

3. **WebServer.h**

   - Also included in the ESP32 core. Handles incoming HTTP requests and serves HTML pages.

---

## 🌐 Web Control Technique

### 1. **Access Point (AP) Mode**

The ESP32 is configured to act as a Wi-Fi Access Point with SSID `ESP32_Servo_AP` and password `12345678`. When a device connects to this network, it can access the servo control panel via the ESP32’s IP address (usually `192.168.4.1`).

### 2. **Embedded HTML Interface**

A complete HTML/CSS/JS page is embedded in the ESP32 firmware. It includes:

- Sliders for each servo (range 0°–180°)
- JavaScript `fetch()` calls that send angle updates to the ESP32’s server

### 3. **Dynamic Slider Rendering**

The HTML page is dynamically generated in the firmware using placeholders (`%SLIDER_BLOCKS%`) and repeated blocks of code for each servo.

### 4. **HTTP GET for Control**

Each slider triggers a URL like:

```
/setServo?id=3&angle=120
```

The ESP32 parses this and updates the appropriate servo accordingly.

---

## 🧠 Code Overview

- **`setup()`** initializes servos and starts the access point and web server.
- **`loop()`** continuously listens for HTTP client requests.
- **`handleRoot()`** serves the main UI page.
- **`handleSetServo()`** receives angle updates and moves the corresponding servo.

---

## 📸 Project Assembly

You may use **ice-cream sticks and glue** to build a DIY robotic arm frame. Secure servos at each joint, starting from the base (MG995) to the gripper (SG90).

---

## 📶 How to Use

1. Upload the code to your ESP32 using the Arduino IDE.
2. Connect to the Wi-Fi network:
   **SSID**: `ESP32_Servo_AP`
   **Password**: `12345678`
3. Open a browser and go to:
   **[http://192.168.4.1](http://192.168.4.1)**
4. Use the sliders to control each servo's angle.

---

## 🛡️ Safety & Power Tips

- Do **not power servos directly from the ESP32's 3.3V pin.**
- Use a **dedicated 5V power supply** for servos (with shared GND to ESP32).
- Be cautious with MG995 motors—they can draw significant current under load.

---

## 📖 Future Improvements

- Add position feedback using potentiometers or encoders.
- Implement inverse kinematics for precise point-to-point control.
- Add preset pose memory or recording/playback of sequences.
- Upgrade UI with real-time feedback (e.g., angles, load).

---

## 🧾 License

This project is released under the MIT License.

---
