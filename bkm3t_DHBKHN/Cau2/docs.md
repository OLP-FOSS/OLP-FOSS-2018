# Câu 2

## Giải pháp

Firmware:

Node-red: Sử dụng node mqtt để lắng nghe data được module esp32 gửi lên qua topic `m3t/mpu6050` sau đó sử dụng node function để tiến hành parse data nhận được sau đó gửi tới node chart để vẽ đồ thị theo thời gian thực.

Docker: Đóng gói node-red,MQTT broker vào docker-compose.

## Thiết kế kiến trúc hệ thống

![../images/cau2.png](../images/cau2.png)

## Cấu trúc node-red

![../images/cau2_nodered.png](../images/cau2_nodered.png)

- Node bkm3t subcrible topic chứa data gửi lên từ esp32
- Node Parse tiến hành parse data
- Node Chart nhận data để vẽ đồ thị