# Docs OLP BKM3T

## Sơ đồ lắp mạch 
<!-- ![](so_do_lap_mach_iot-maker.png) -->

![](lap_mach_1.jpg)

![](lap_mach_2.jpg)

"""TODO"""

## Hướng dẫn cài đặt Arduino IDE
- Dowload Arduino IDE tại địa chỉ
https://www.arduino.cc/en/Main/OldSoftwareReleases#previous, version 1.8.6

-  Mở chương trình Arduino chọn file-> Preferences. nhập https://dl.espressif.com/dl/package_esp32_index.json  vào Additional Board Manager URLs, rồi nhần "OK" Như hình  ![](preferences.png)

- Vào boards manager, Tools > Board > Boards Manager…
![](boardsManager.png)

- Tìm "ESP32" và nhần cài đặt "ESP32 by Espressif Systems"
![](installing.png)

- Để chạy file firmware, mở Arduino IDE, vào Tools -> Board và chọn board "DOIT ESP32 DEVKIT V1"
![](windows-select-board.png)

- Chọn cổng COM rồi nhấn nút Upload hoặc phím tắt Ctrl + U

- Đợi đến khi xuất hiện thông báo "Done Uploading" tức là đã nạp code thành công
![](windows-arduino-ide-done-uploading.png)




## Các thư viện liên quan

- Gửi nhận dữ liệu hồng ngoại: (ESP32-IRremote) https://github.com/SensorsIot/Definitive-Guide-to-IR/tree/master/ESP32-IRremote (liences GNU)

- Nhận thời gian từ server với NTP Client và RTC: https://github.com/arduino-libraries/NTPClient, 
https://github.com/Makuna/Rtc

- Hiển thị dữ liệu lên OLED: https://github.com/ThingPulse/esp8266-oled-ssd1306

<!-- - Cảm biến nhịp tim : https://github.com/oxullo/Arduino-MAX30100 -->

- Cảm biến gia tốc: https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050 

## Note
- Broker: 10.170.46.196
- Topic đồng hồ gửi dữ liệu: bkm3t/mpu6050
- Topic đồng hồ nhận lệnh : bkm3t/olp