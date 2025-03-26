# Autonomous Food Delivery Robot 🤖

This project demonstrates an **Autonomous Food Delivery Robot** designed to streamline restaurant operations by automating food delivery. The robot navigates autonomously, avoids obstacles, and delivers food to designated tables while being controlled and monitored through a web-based interface.

[![Watch the Demo Video](https://img.youtube.com/vi/yourvideoid/0.jpg)](https://www.youtube.com/watch?v=yourvideoid)

---

## 🌟 Features

- **Autonomous Navigation**: Uses encoder feedback, orientation data from the MPU6050 IMU, and trigonometric calculations to navigate to target locations.
- **Obstacle Avoidance**: Equipped with an ultrasonic sensor to detect and avoid obstacles in real-time.
- **Web-Based Interface**:
  - Customer-facing interface for selecting table destinations.
  - Real-time monitoring of the robot's location, battery percentage, and status.
- **Battery Monitoring**: Continuously measures and displays the battery charge level.
- **Scalable Design**: Can be expanded to handle multiple robots or larger restaurant environments.

---

## 🛠️ Hardware Components

The robot is built using the following components:

- **ESP32 Microcontroller**: Acts as the brain of the robot.
- **L298N Motor Driver**: Controls the DC gear motors.
- **DC Gear Motors**: Provide motion for the robot.
- **Ultrasonic Sensor**: Detects obstacles in the robot's path.
- **MPU6050 IMU**: Provides orientation data for navigation.
- **Encoders**: Measure wheel rotation for precise movement control.
- **18650 Batteries and Buck Converter**: Power the entire system and ensure stable voltage.

---

## 💻 Software Components

The software integrates both hardware control and web-based interfaces:

- **ESP32 Web Server**: Hosts a lightweight web page for controlling the robot and monitoring its status.
- **Navigation Logic**: Implements algorithms for autonomous navigation and obstacle avoidance.
- **Web Interface**: Built using HTML, CSS, and JavaScript for user interaction.

---

## 🚀 How to Set Up

### Prerequisites

- Install the [Arduino IDE](https://www.arduino.cc/en/software).
- Add the ESP32 board support to the Arduino IDE ([Instructions here](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide/)).
- Install required libraries:
  - `Wire.h`
  - `MPU6050_light.h`
  - `WiFi.h`
  - `WebServer.h`

### Steps

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/autonomous-food-delivery-robot.git
   ```

2. Open the `.ino` file in the Arduino IDE.

3. Update the Wi-Fi credentials in the code:
   ```cpp
   const char* ssid = "Your_SSID";
   const char* password = "Your_PASSWORD";
   ```

4. Upload the code to your ESP32 microcontroller.

5. Power up the robot and connect to the ESP32's Wi-Fi network.

6. Access the web interface by navigating to the ESP32's IP address in your browser.

---

## 📺 Demonstration

Check out the demo video to see the robot in action:

[Watch the Demo Video on YouTube](https://www.youtube.com/watch?v=yourvideoid)

---

## 🤝 Contact Me

For inquiries, collaborations, or feedback, feel free to reach out to me via LinkedIn:

[Connect with me on LinkedIn](https://www.linkedin.com/in/yourlinkedinprofile)

---

## 📜 License

This project is licensed under the [MIT License](LICENSE). Feel free to use, modify, and distribute the code as needed.

---

## 🙏 Acknowledgments

- Special thanks to the open-source community for providing libraries and tools that made this project possible.
- Inspiration drawn from advancements in robotics and automation.

---

Made with ❤️ by [Ruchira Silva](https://www.linkedin.com/in/yourlinkedinprofile)  
© 2023

---

### Notes:
- Replace `yourvideoid` with the actual ID of your YouTube video.
- Replace `yourlinkedinprofile` with your actual LinkedIn profile URL.
- Optionally, add a `LICENSE` file if you want to include the full text of the MIT License.
