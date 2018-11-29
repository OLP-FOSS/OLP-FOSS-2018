# Câu 3

## Giải pháp cơ bản cố định mã code thiết bị cần điều khiển

Sử dụng cảm biến mpu6050 để đo gia tốc sau đó dùng để nhận dạng hành động. Khi nhận dạng được hành động là gạt tay lên sẽ sử dụng bộ phát IR để gửi tiến mã đến thiết bị cần điều khiển để đưa ra hành động.

## Giải pháp tự học mã lệch mới không cần cố định mã trong code

Thêm bộ thu IR trên esp32 để có thể thu tín hiệu hồng ngoại. Khi nào cần thêm một mã lệch để điều khiển 1 thiết bị mới sẽ sử dụng thiết bị đó để phát mã hồng ngoại vào cảm biến IR gắn trên ESP32. Khi đó ESP32 sẽ lưu thêm mã hồng ngoài của thiết bị đó vào. Từ đó ESP32 có thể điều khiển được thiết bị đã thêm vào. Nói cách khác sẽ có một bước để thiết bị đăng ký tín hiệu hồng ngoại của mình với ESP32.