# Capstone
Heartlink: IoT Health Monitoring System

This is a Capstone project for collecting physiological data via an ESP32 and transmitting it to the cloud.
  Note: The source code is located in the master branch.

Project Status (MVP)
Developed during a strict one-month timeline, the project achieved a functional end-to-end data pipeline:
  Sensor Integration: Successfully implemented the MAX30102 sensor for heart rate and SpO2 data collection.
  Cloud Connectivity: Established stable data transmission to the cloud via Wi-Fi.
  Architecture Pivot: Due to time constraints and hardware compatibility with the ESP32 Arduino Nano, the project shifted from a CMake-based structure to the Arduino framework.
  
Repository Structure (on master)
  /HeartLink: Functional Arduino project. This is the tested and verified version of the system.
  /capstoneHeartlink: Experimental CMake-based implementation. This folder remains untested and was part of the initial architecture exploration.
  
Tech Stack
  Hardware: ESP32 Arduino Nano, MAX30102 Sensor.
  Framework: Arduino.
  Cloud Integration: Google Cloud Platform (GCP).
  
Development Context
  This project represents a rapid "Proof of Concept" (PoC) completed as part of my graduation from Fanshawe College. It demonstrates the ability to pivot technical strategies under pressure   to deliver a working Minimum Viable Product (MVP).
