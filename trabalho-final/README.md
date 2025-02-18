# Smart Cane System for Assisting Visually Impaired Individuals

## Overview

This repository contains the source code and documentation for the **Smart Cane System for Assisting Visually Impaired Individuals**, developed as part of the final project for the **LAB-SISMIC** course. The system aims to provide an assistive device for visually impaired people using a distance sensor and a vibration motor to alert users about nearby obstacles.

## Features

- Object detection using the **VL53L0X** distance sensor.
- Tactile alert via **vibration motor**, controlled by PWM.
- Adjustable alert sensitivity using a **linear potentiometer**.
- Data processing using the **MSP430F5529** microcontroller.
- Implementation of **I2C communication** for sensor interaction.

## Components Used

### Hardware

- **Microcontroller**: MSP430F5529
- **Distance Sensor**: VL53L0X
- **Transistor**: BC547
- **Vibration Motor**
- **Linear Potentiometer**

### Software

- Programming Language: **C**
- [Official API](https://www.st.com/en/imaging-and-photonics-solutions/vl53l0x.html) from **STMicroelectronics** for the VL53L0X sensor.

## Documentation

For more technical details, refer to the full report [final-report](./relatorio-final.pdf) (in portuguese).
