# ESP8266 OLED Integrated Projects 🚀

This repository contains various embedded systems and Internet of Things (IoT) projects developed for the **IdeasSpark ESP8266 (ESP-12E) NodeMCU** development board with an integrated **0.96" Dual-Color (Yellow/Blue) OLED Display (SSD1306)**.

---

## 📸 Project Showcase

<p align="center">
  <img src="./images/oled (3).jpg" width="45%" alt="NOZ-7 Branding" />
  <img src="./images/oled (5).jpg" width="45%" alt="Digital Clock" />
</p>

<p align="center">
  <img src="./images/oled (1).jpg" width="30%" alt="Exchange Rates" />
  <img src="./images/oled (2).jpg" width="30%" alt="Temperature" />
  <img src="./images/oled (4).jpg" width="30%" alt="Smart Schedule" />
</p>

---

## 📁 Projects Index

| # | Project Name | Description | Key Features |
|---|---|---|---|
| 01 | **[Desktop Smart Assistant](./01-desktop-smart-assistant)** | Real-time information hub & desk display | NTP Sync, Live Exchange Rates, Weather API, Dynamic Schedule, Display Freeze Button |

---

## 📌 Featured Project: Desktop Smart Assistant

An intelligent desktop assistant that displays live data in a 4-second cycling loop, optimized for dual-color (yellow/blue) OLED displays.

### 🌟 Key Features
* **NTP Sync Digital Clock:** Precise time synchronization using NTP (Network Time Protocol) with hourly blink alerts.
* **Live Financial Data:** Real-time tracking for Exchange Rates (USD, EUR, GBP) using REST APIs.
* **Smart Weather Assistant:** Real-time temperature fetching for Nicosia via Open-Meteo API with automated clothing/weather advice.
* **Context-Aware University Schedule:** Automatically detects the current day/hour and displays the next upcoming course code and room location (e.g., `17.00 - ST235 CMP415`).
* **Display Freeze Control:** Single-click interaction using the built-in `FLASH` button to freeze/unfreeze any screen on demand.
* **Dual-Color OLED Layout Optimization:** Custom-aligned interface partitioning the top yellow bar and bottom blue zone.

---

## 🛠️ Hardware Requirements
* **Board:** IdeasSpark ESP8266 (ESP-12E MOD) with Integrated 0.96" OLED
* **Display:** SSD1306 Dual-Color OLED (128x64 pixels, I2C: SDA=GPIO12, SCL=GPIO14)
* **Control:** On-board `FLASH` Button (GPIO0)
* **Power:** Micro-USB Cable

---

## 💻 Software & Libraries
* **Framework:** ESP8266 Core for Arduino
* **Libraries Used:**
  * `ESP8266WiFi` & `ESP8266HTTPClient`
  * `ArduinoJson`
  * `NTPClient`
  * `Adafruit_GFX` & `Adafruit_SSD1306`

---

## 📝 License & Author
* Developed by **NOZ-7**
