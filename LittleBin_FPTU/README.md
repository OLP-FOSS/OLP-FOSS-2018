# FPTU LittleBin - Smartwatch

## Members
- Duong Xuan Hoa
- Le Dinh Duy
- Nguyen Minh Duc

## Environment
- Docker [see here](https://www.docker.com/).
- Setup MQTT Server [see here](https://mosquitto.org/download/).
- Setup NodeRed for Docker [see here](https://nodered.org/docs/platforms/docker).
- ESP32, MPU6050 drivers [see here](https://github.com/makerhanoi/meo-guide).
- Scikit-learn for Motion detection [see here](https://scikit-learn.org/)

## Configument
1. Setup wifi for ESP32 on arduino file
2. Import NodeRed flow in source directory
3. Download and import Arduino Third-party Library [i2cdevlib](https://github.com/jrowberg/i2cdevlib), [pubsubclient](https://github.com/knolleary/pubsubclient)

## Folder Structure:
- motionData: this folder contains motion data we record by MPU6050 sensor
- nodeRedFlow: this folder contains flow for node-red
- motion-detection.py: this file is used for motion detection with data receive from MQTT Broker
- arduinoSrc: this folder contains code and library for ESP32
- sketch_nov29a: this is Arduino Source for collecting data from MPU6050 (draw chart, ...)

