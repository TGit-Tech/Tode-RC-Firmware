# Tode-RC-Firmware

21C9 Remote On/Off Devices tested and working

221N Local Device Polling, Pressure & Distance working

### 2314 Release Candidate ( works well )
* OnOff Device
* Analog Input Devices with configured mathematics
* Sonic Distance Sensor with Trig Echo Pins
* Start/Stop 3-Wire Device

### 231H User Test (works)
* Added Device Control Logic (DCL)
* Input now can control output via Setpoint or Math
* Todes and Devices are Numbered for the DCL selection

### 232H MQTT Support (works)
* Added homie MQTT control support
* System Submenu for resets and KB by Serial Control

### 241E Multi-Model Support
* Please set Radio and Tode-RC hardware model in config.h

* ---

# 🌐 **Tode Ecosystem**

The Tode Ecosystem offers a robust, flexible, and secure solution for managing devices in an environment where **infrastructure is not required**. It’s designed to seamlessly integrate with the **RF-DITS** protocol, providing a communication framework for device control, alarm propagation, and security.

## 🚀 **Overview**

The Tode Ecosystem empowers devices to function autonomously or in **P2P** (peer-to-peer) networks, providing an **infrastructure-free solution** for industrial, home, and remote applications. With its **5V EMT-resistant hardware** and powerful **virtual device** logic for alarm processing, it brings resilience and versatility to the edge of your network.

---

## 🛠️ **Key Features**

* **Non-Infrastructure Communication**: Operates **without** requiring an internet connection, server, or cloud dependencies. Devices communicate **peer-to-peer** between nodes, allowing for seamless data exchange and control.

* **RF-DITS Protocol**: The core communication protocol used by the Tode ecosystem. This protocol allows secure, fast, and efficient communication between Master and Remote nodes using **Device Index Tables (DIT)** with integrated security.

* **5V EMT Resistance**: Built to handle harsh environments, the Tode ecosystem is engineered to withstand **electromagnetic interference (EMI)**, providing reliable operation in industrial settings.

* **Virtual Device Logic**: The **Virtual Device Logic** allows users to implement custom control logic directly on the device, enabling real-time alarm processing, device manipulation, and other control functions.

* **Quick Firmware Updates**: The firmware can be quickly updated by plugging the Tode node into a wiring interface, making updates straightforward without requiring specialized equipment.

---

## 🔒 **Security**

* **SecNet (Security Network)**: The Tode ecosystem utilizes **SecNet**, a user-set security code embedded in each packet. This ensures data integrity and security. The **SecNet** code is also used as part of a challenge-response mechanism during **SETVAL** operations, preventing replay attacks and ensuring secure value updates.

* **Packet Integrity**: Every packet transmitted in the Tode network is verified for authenticity with the **SecNet** code. Only authorized devices can send or receive data securely. **SETVAL** operations are particularly secured to ensure legitimate communication.

* **Alarm Security**: Alarm messages are pushed to the designated **Remote nodes** when a condition is triggered. The **SecNet** code ensures that only authorized recipients receive the alarm notifications.

---

## ⚡ **Quick Setup & Control**

* **Device Setup**: The Tode ecosystem doesn’t require centralized infrastructure or cloud servers. Each **Master node** stores the configuration of its attached devices and is capable of sharing this configuration with **Remote nodes** using the **RF-DITS protocol**. **Remote nodes** can pull configurations and control devices according to those settings.

* **Time-Sensitive Updates**: Alarm conditions are pushed immediately to all **Remote nodes** without unnecessary overhead. Value updates can be requested by the remote node through the **RF-DITS protocol**, ensuring efficient communication.

---

## 📈 **Key Advantages Over Common IoT Platforms**

| **Key Advantage**             | **Tode Ecosystem (RF-DITS)**                                                                           | **Common IoT Platforms**                                                        |
| ----------------------------- | ------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------- |
| **Infrastructure**            | Operates **without** internet or server dependencies, functioning in **peer-to-peer** mode.            | Requires cloud, servers, or centralized hubs for device control                 |
| **Portability**               | **Portable** control with remote nodes in P2P communication.                                           | Typically dependent on fixed infrastructure                                     |
| **Data Integrity**            | Secured communication with **SecNet** protecting packets and ensuring valid data exchange.             | Often lacks robust security or packet verification                              |
| **Low Airtime Communication** | Optimized communication with **DIT** table, reducing airtime usage for value exchanges.                | Typically uses more airtime due to polling systems and continuous communication |
| **Alarm Handling**            | **One-time push** of alarms to Remote nodes, minimizing network traffic.                               | Continuous polling for alarm conditions, leading to higher network usage        |
| **Device Control**            | Full **control** over devices with direct value setting, updating, and logic processing.               | Remote devices may be read but not easily controlled depending on the platform  |
| **EMT Resistance**            | Designed to handle **electromagnetic interference (EMI)** in industrial environments.                  | Sensitive to environmental EMI interference, which can disrupt performance      |
| **Firmware Updates**          | **Easy and fast** firmware updates via a wiring interface—no internet or specialized equipment needed. | Over-the-air updates typically require internet access and cloud dependencies   |
| **Flexible Control Logic**    | **Virtual device logic** allows for custom control and alarm processing locally on the device.         | Often lacks flexibility for local control and relies on cloud logic             |

---

## ⚙️ **Protocol Breakdown: RF-DITS**

### **Device Index Table with Security (DITS)**

Each node maintains a **Device Index Table (DIT)** that lists the connected devices by index. This table helps the **Master node** store and transmit configurations for each attached device to **Remote nodes**.

* **Master Node**: Stores and manages the configuration of devices, pushes alarm notifications, and shares device settings with **Remote nodes**.
* **Remote Node**: Pulls device configurations from the **Master node** and can manipulate values according to the stored configuration.

### **Alarms**

* **Alarm Handling**: When an alarm condition is triggered on the **Master node**, an alarm packet is sent to all **Remote nodes** on the recipient list.
* **No Acknowledgment Required**: The alarm is a **one-time push** with no response required from the **Remote nodes**.
* **Alarm Recipient List**: The **Master node** stores a list of **Remote node** addresses, which is dynamically updated with each communication, including value requests. Manual pruning of the list is also possible.

---

## 🔗 **Integration with External Systems**

While the Tode ecosystem operates **without the need for internet infrastructure**, it can optionally integrate with systems such as **MQTT**, **OpenHAB**, or **Home Assistant** through an MQTT bridge.

This optional feature allows:

* **Pulling Device Values**: Remote nodes can request device values from **Master nodes** on a regular schedule or based on events.
* **Pushing Alarms**: When an alarm condition is triggered, the **Master node** can push the alarm data to an MQTT broker, which allows external systems to act on the alarm.

However, it's important to note that **Tode's core functionality** works without cloud connectivity, and the MQTT integration is entirely optional.

---

## 🔑 **Hardware Advantages**

Tode nodes are designed to operate in **industrial environments** where **electromagnetic interference (EMI)** is common, and devices need to withstand harsh conditions. The **5V EMT-resistant hardware** ensures the Tode ecosystem works reliably even in settings that may disrupt other IoT systems.

---

## 🛠️ **Getting Started**

1. **Set Up the Master Node**: Connect and configure devices on the **Master node**. Store configurations and define which devices you want to control.
2. **Add Remote Nodes**: Manually enter the **Master node's** address into the **Remote nodes** to establish communication and pull device configurations.
3. **Manage Alarms and Control**: Configure alarm conditions on the **Master node**. Alarms will be **pushed** to all **Remote nodes**.
4. **Firmware Updates**: Simply plug the Tode node into the wiring interface to update its firmware without requiring internet access.

---

## 📚 **Conclusion**

The **Tode Ecosystem** offers a resilient, flexible, and secure solution for edge-device control in environments where **infrastructure** is not an option. Its core **RF-DITS protocol** and **5V EMT resistance** provide reliable communication, while its optional integration with systems like **MQTT** makes it versatile for a range of applications.

---
