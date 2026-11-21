# Tiến độ

| # | Milestone | Skill | Hoàn thành |
| --- | --- | --- | --- |
| 1 | Cửa sổ + vòng lặp + build chạy được | 1 | [x] |
| 2 | Shader + vẽ cube có màu | 2, 3 | [x] |
| 3 | Camera FPS + sàn + vài khối | 4, 7 | [x] |
| 4 | Ánh sáng Blinn-Phong + texture | 5, 6 | [x] |
| 5 | Va chạm với level + nhảy | 8 | [x] |
| 6 | Súng, bắn hitscan, HUD cơ bản | 8, 9, 11 | [x] |
| 7 | Địch + wave + điểm + Game Over | 10, 11 | [x] |
| 8 | Hiệu ứng nâng cao (shadow, particle...) | 12 | [x] |
| 9 | Hoàn thiện, tối ưu, tài liệu | 13 | [x] |

## Kiểm thử Milestone 1

Ngày 18/11/2026, đã build sạch trên Windows với MinGW-w64 GCC 16.2.0 và CMake 4.4.2. Hai cấu hình Release và Debug biên dịch với `-Wall -Wextra` cho mã dự án và không có cảnh báo.

| Kiểm tra trên Release và Debug | Kết quả |
| --- | --- |
| Khởi động | Vùng vẽ ban đầu 1280×720 pixel. Console ghi `OpenGL: 3.3.0 - Build 32.0.101.7026`. |
| Xóa bộ đệm màu | Sau khi nền ổn định, một pixel trong vùng vẽ có RGB (25, 46, 64). |
| Đổi cỡ, thu nhỏ, khôi phục | Vùng vẽ đổi thành 882×553 pixel. Cửa sổ tiếp tục nhận Esc sau khi khôi phục. |
| Thoát bằng Esc | Cả hai tiến trình trả mã 0. Bản Debug không ghi lỗi từ `GL_CHECK()`. |

## Kiểm thử Milestone 2

Ngày 18/11/2026 trên Windows, MinGW-w64 và OpenGL 3.3.0.

| Kiểm tra | Kết quả |
| --- | --- |
| Build sạch Release và Debug | Cả hai cấu hình biên dịch với `-Wall -Wextra` cho mã dự án, không có cảnh báo. |
| Vẽ và quay cube | Lấy mẫu màu trực tiếp từ cửa sổ 1280×720. Hai lần lấy mẫu cách nhau 0,55 giây cho thấy vùng màu của các mặt đã di chuyển. |
| Đổi cỡ, thu nhỏ, khôi phục, Esc | Vẫn đo được nhiều màu mặt trên vùng vẽ 882×553 sau khi khôi phục. Cả hai bản thoát với mã 0 và stderr trống. |
| Bốn primitive | Đã kiểm tra chỉ số trong phạm vi, pháp tuyến đơn vị, UV, diện tích và chiều của từng tam giác. Cube, plane, sphere và cylinder lần lượt có 12, 2, 960 và 128 tam giác. Mỗi hình đều tạo mẫu qua depth test khi vẽ riêng với culling bật. |
| Chẩn đoán shader | Trên bản Release, thử riêng ba lỗi trong thư mục tạm. Thiếu fragment shader báo đúng đường dẫn. Lỗi cú pháp báo dòng 3. Lỗi kiểu dữ liệu giữa hai stage báo hai tên file và biến `faceColor`. Cả ba trả mã 1. |

## Kiểm thử Milestone 3

Ngày 18/11/2026 trên Windows với MinGW-w64, cả Release và Debug build sạch với `-Wall -Wextra`. Hai bản chạy trên OpenGL 3.3.0.

| Kiểm tra | Kết quả |
| --- | --- |
| Camera và bước đi | Kiểm tra số học ở cả hai giới hạn pitch ±89°, đổi yaw 90°, phối cảnh và độ dài hướng đi chéo. Mô phỏng đi bộ một giây với 30, 60 và 144 bước cho cùng quãng đường 4 đơn vị. |
| Biên đấu trường | Đoạn di chuyển dài 1000 đơn vị bị chặn ở cả bốn hướng và ở góc tường. Kiểm tra riêng việc chạm mặt thùng rồi tiếp tục trượt dọc cạnh. |
| Input và hình ảnh thực | Cả hai bản cho ảnh khác khi nhấn W, di chuột và nhảy. Ảnh sau tiếp đất khớp với ảnh trước khi nhảy. D không làm thay đổi ảnh trong lúc pause và hoạt động lại sau khi tiếp tục. |
| Đổi cỡ và vòng đời | Đã đổi kích thước, thu nhỏ, khôi phục và kiểm tra cảnh tiếp tục hiển thị. Thoát từ pause bằng Q trả mã 0, stderr trống ở cả hai cấu hình. |

## Kiểm thử Milestone 4

Ngày 18/11/2026, build sạch Release và Debug trên Windows với MinGW-w64, `-Wall -Wextra` và không có cảnh báo. Hai bản chạy trên OpenGL 3.3.0.

| Kiểm tra | Kết quả |
| --- | --- |
| Đèn pin trên cửa sổ game | F làm đổi 63/713 điểm lấy mẫu tại vị trí xuất phát ở cả hai bản. F lần nữa khôi phục ảnh. F trong pause giữ nguyên trạng thái. |
| Chớp sáng | Nhấp chuột trái làm sàn sáng lên rồi trở về màu ban đầu. Phép thử riêng với renderer thấy chớp còn sau 49 ms và tắt sau 51 ms mô phỏng. |
| Specular | Cô lập thành phần bóng trên plane và đọc lại kênh đỏ 8 bit ở tâm. Chiếu vuông góc cho 66 khi nhìn thẳng, 0 khi nhìn xiên 45° và 0 khi nguồn ở phía sau mặt. |
| Texture và fallback | Đọc lại GPU xác nhận PNG RGB rộng 3 pixel được lật đúng hàng. File hỏng và bốn đường dẫn thiếu đều dùng ảnh thủ tục. Mipmap của ảnh 128×128 xuống tới 1×1, wrap lặp trên cả hai trục và lọc dị hướng đạt 8× trên máy thử. |
| Cubemap | Sáu mặt đều có kích thước 64×64. Khi hướng camera lên trời rồi dịch vị trí, sai khác tối đa là 1 mức trên kênh màu 8 bit. |
| Kiểm tra lại Milestone 3 | Đi, xoay, nhảy, pause, resize và thu nhỏ/khôi phục hoạt động ở cả hai bản. Cả hai thoát với mã 0, stderr trống. |

## Kiểm thử Milestone 5

Ngày 18/11/2026 trên Windows với MinGW-w64 và OpenGL 3.3.0. Release và Debug đều build sạch với `-Wall -Wextra`, không có cảnh báo. CTest chạy đạt bộ `physics` ở cả hai cấu hình; các phép kiểm tra vẫn hoạt động khi Release định nghĩa NDEBUG.

| Kiểm tra | Kết quả |
| --- | --- |
| Va chạm theo trục | Đoạn quét 100 đơn vị dừng ở mặt gần nhất của tường dày 0,02. Kiểm tra trượt cạnh, chạm góc, rời tường và đi ngang trên mặt trên đều đạt. |
| Nhảy và mặt đỡ | Từ điểm xuất phát thật, player bị chặn khi đi bộ vào thùng thấp, nhảy lên nắp, đứng vững, nhảy tiếp từ nắp và rơi về sàn khi bước qua mép. Một lần nhấn Space giữa không trung không tăng độ cao cực đại. |
| Chạm trần | Với hộp thử đặt cao hơn đầu 0,30 đơn vị, đỉnh thân dừng đúng tại mặt dưới. Player vẫn ở trạng thái trên không rồi rơi về sàn. |
| Tia–AABB | Unit test kiểm tra hướng âm, gốc trong hộp, tia trên cạnh, song song ngoài hộp, hộp phía sau, các khoảng không giao nhau và hướng bằng 0. |
| Cửa sổ game | Đã thử trực tiếp đứng trên nắp thùng, nhảy lại và đi ngang xuống sàn ở cả hai bản. Khi giữ hướng nhìn và tọa độ Z, mép trên của tường xa dịch 20 pixel giữa hai độ cao. Đèn pin, di chuột, pause, resize, thu nhỏ và khôi phục hoạt động; thoát với mã 0, stderr trống. |

## Kiểm thử Milestone 6

Ngày 18/11/2026 trên Windows/MinGW-w64, build sạch Release và Debug với `-Wall -Wextra`, không có cảnh báo. CTest đạt 2/2 (`physics`, `combat`) ở cả hai cấu hình.

| Kiểm tra | Kết quả |
| --- | --- |
| Trạng thái súng | Kiểm tra phát đơn, giữ cò theo nhịp, nạp khi băng đầy, cấm bắn lúc nạp, chuyển đạn đúng một lần, băng cuối thiếu và hết dự trữ. |
| Hitscan và sát thương | Bắn từ sau thùng giữ nguyên HP bia. Bốn phát trực tiếp làm bia từ 100 về 0. Tia ra trời trượt, tia xuống sàn trả về pháp tuyến hướng lên. Các mẫu hướng bắn có độ dài 1 và nằm trong góc tản. |
| HUD trên cửa sổ | Đọc các chữ số bằng pixel ở cả hai bản. Sau phát đầu: 11/42. Tạm dừng hơn 1,2 giây giữa lần nạp vẫn giữ 11/42. Tiếp tục đến 12/41, giữ chuột đến 0/41, nạp thành 12/29. Đổi cỡ vẫn đọc được 12/29. |
| Kết xuất và thoát | Đã đọc lại framebuffer để đối chiếu cảnh trước phát bắn, khung có chớp/tracer và khung còn vết trúng. Kiểm tra F3, thu nhỏ/khôi phục, Esc/Q trên OpenGL 3.3.0. Cả hai bản thoát mã 0, stderr trống. |

## Kiểm thử Milestone 7

Windows, MinGW-w64 và OpenGL 3.3 Core. Build sạch cả Release và Debug với `-Wall -Wextra`. Cả hai cấu hình đạt 3/3 bộ CTest (`physics`, `combat`, `waves`).

| Kiểm tra | Kết quả |
| --- | --- |
| Dẫn đường và tấn công | Từ ba điểm xuất phát đến bốn vị trí người chơi, robot đến tầm đánh ở cả 12 đường và thân không lọt vào hộp. Bia nằm giữa hai bên chặn đòn cận chiến. Robot chết ngừng gây sát thương. |
| Hitscan và vòng lặp wave | Kiểm tra thùng che robot, robot che bia, điểm chỉ cộng ở phát kết liễu. Mô phỏng dọn chín đợt liên tiếp và xác nhận thời gian nghỉ, giới hạn tám địch và chuyển đợt. |
| HUD khi chơi | Đọc pixel chữ trên cửa sổ Release và Debug sau khi hạ hai robot. Điểm lần lượt là 100 và 300, sau đó HUD hiện `WAVE 2   SCORE 300`. F3 hiển thị 60 FPS trong mẫu ở 1280×720. |
| Vòng đời ván | Đã mở menu, vào chơi, pause và tiếp tục. Bản Debug hiện `SCORE 300` khi mất hết máu ở wave 2. Enter bắt đầu lại ở wave 1, điểm về 0. Đã xác minh HP về 100 và đạn về 12 / 42 khi chơi lại, đổi kích thước, thu nhỏ/khôi phục và thoát bằng Q. Cả hai bản trả mã 0, stderr trống. |

## Kiểm thử Milestone 8

Build sạch bản Release và Debug trên Windows/MinGW-w64 với `-Wall -Wextra`, không có cảnh báo. Cả hai cấu hình đạt 3/3 bộ CTest. Đã chạy game bằng OpenGL 3.3.0.

| Kiểm tra | Kết quả |
| --- | --- |
| F6 trên cùng khung cảnh | Ở menu 1280×720, tắt bóng làm 7.312 pixel sáng lên ít nhất 3 mức ở một kênh RGB. Bật lại cho ảnh trùng từng pixel. Ảnh sau khi chỉnh độ bù cho thấy bóng cột và robot trên sàn, mặt bia hết hoa văn tự che. |
| F7 sau một phát trúng bia | Pause giữ nguyên tuổi cụm. Tắt tia lửa làm 190 pixel giảm ít nhất 2 mức ở một kênh. Bật lại khôi phục đúng ảnh. Sau khi tiếp tục 0,65 giây rồi pause, hai trạng thái F7 cho ảnh như nhau. |
| Resize, thu nhỏ và khôi phục | Ở 882×553, F6 vẫn thay đổi 3.691 pixel theo cùng ngưỡng. Bật lại khôi phục đúng ảnh cả trước và sau khi thu nhỏ cửa sổ. Bản Debug thoát với mã 0, stderr trống. |
| Vòng chơi trên Release | Khi bật cả hai hiệu ứng, F3 hiển thị 60 FPS trong mẫu ở 1280×720. HUD đạt 100 rồi 300 điểm ở wave 1, giữ 300 khi vào wave 2. Game Over hiện 300 điểm và Enter đưa ván mới về wave 1, điểm 0. Thoát mã 0, stderr trống. |

## Kiểm thử Milestone 9

Nghiệm thu trên Windows/MinGW-w64, Intel UHD Graphics 770, OpenGL 3.3.0 (driver 32.0.101.7026).

| Kiểm tra | Kết quả |
| --- | --- |
| Build và CTest | Build sạch Release và Debug với `-Wall -Wextra`, không có cảnh báo. Cả hai đạt 3/3 bộ `physics`, `combat` và `waves`. Đích tùy chọn `render_benchmark` cũng biên dịch ở cả hai cấu hình. |
| HUD trước/sau | Cảnh wave 7, tám robot, 3.900 điểm, HUD nạp đạn, hiển thị FPS. Đếm lệnh vẽ HUD trong màn chơi được 56 trước và 20 sau. Đọc ảnh RGBA ở Menu, Playing, Pause và Game Over tại 1280×720 và 882×553. Cả tám cặp ảnh trùng từng pixel. |
| Vòng chơi Release | Đọc HUD trên cửa sổ thật, thấy 60 FPS ở 1280×720. Từ menu vào chơi, tạm dừng, tiếp tục, hết máu và chơi lại. Ở ván kế, HUD đạt 100 rồi 300 điểm, chuyển wave 2, hiện 300 điểm ở Game Over và về 0 điểm khi nhấn Enter. Resize, thu nhỏ/khôi phục và thoát bằng Q thành công. |
| Hiệu ứng Debug | Tắt rồi bật lại F6/F7 ở một khung hình tạm dừng cho ảnh khôi phục đúng từng pixel. F7 hết tác dụng sau khi hạt đã tắt. Lặp lại F6 sau resize và thu nhỏ/khôi phục vẫn đạt. Hai bản game thoát mã 0, stderr trống. |
| Vòng đời GL | Trong một context còn hoạt động, tạo, vẽ và hủy các renderer ba lần. Mỗi lần theo dõi 16 buffer, 8 VAO, 7 texture, 1 framebuffer, 6 program và 12 shader. Sau khi bỏ gắn program và chờ GPU, toàn bộ tên đối tượng đã giải phóng theo `glIs*`. |

### Thời gian kết xuất ở wave 7

Mẫu Release của `render_benchmark` vẽ ở 1280×720, với đèn pin, một cụm tia lửa tuổi 0,1 giây và tám robot. Mỗi tổ hợp làm nóng 30 khung rồi lấy 180 mẫu. Đồng hồ đo cả thời gian CPU gửi lệnh và chờ GPU qua `glFinish`. F3 trong game còn tính mô phỏng và chờ đổi buffer.

| Bóng mặt trời (F6) | Tia lửa (F7) | Trung vị | P95 |
| --- | --- | ---: | ---: |
| Tắt | Tắt | 2,534 ms | 4,222 ms |
| Tắt | Bật | 2,244 ms | 3,015 ms |
| Bật | Tắt | 2,480 ms | 3,321 ms |
| Bật | Bật | 2,574 ms | 3,811 ms |

Các số đo thuộc một lần chạy trên máy trên. Có thể lặp lại bằng lệnh trong [README](../README.md#đo-thời-gian-vẽ-ở-wave-đông). Linux và GPU khác chưa được chạy thử trong đợt nghiệm thu này.

## Kiểm thử tính năng chọn map và đa bản đồ

Ngày 21/11/2026 trên Windows, MinGW-w64 GCC 16.2.0, Intel UHD Graphics 770, OpenGL 3.3.0 (driver 32.0.101.7026).

| Kiểm tra | Kết quả |
| --- | --- |
| Build sạch Release và Debug | Cả hai cấu hình biên dịch với `-Wall -Wextra`, 0 cảnh báo. |
| CTest Release và Debug | Đạt 3/3 bộ kiểm thử (`physics`, `combat`, `waves`) ở cả hai cấu hình, bao gồm `testAllMapsGeometry` (kiểm tra kích thước, điểm xuất phát, sinh địch không chồng khối và nhảy 2 bậc ở `Warehouse`) và `testAllMapsNavigationAndWaves` (dẫn đường tới đích và chuyển đợt trên cả 4 map). |
| Kết xuất 4 bản đồ & hit-test | Đã dựng và vẽ cả 4 bản đồ (`ARENA`, `WAREHOUSE`, `CITADEL`, `LABYRINTH`) qua đủ 4 trạng thái (`Menu`, `Playing`, `Paused`, `GameOver`) với kiểm tra `glGetError() == GL_NO_ERROR`, và kiểm tra `HUD::hitTestMenu` cho cả 4 thẻ map cùng nút Play tại 1280×720 và 882×553. |
| `render_benchmark` Release | Chạy thành công ở 1280×720 (wave 7, 8 robot): `F6=0 F7=0` trung vị 2,688 ms; `F6=0 F7=1` trung vị 11,232 ms; `F6=1 F7=0` trung vị 5,000 ms; `F6=1 F7=1` trung vị 6,142 ms. |

## Kiểm thử tính năng tạo map từ ảnh bất kỳ (Image-to-Map)

Ngày 21/11/2026 trên Windows, MinGW-w64 GCC 16.2.0, Intel UHD Graphics 770, OpenGL 3.3.0 (driver 32.0.101.7026).

| Kiểm tra | Kết quả |
| --- | --- |
| Build sạch Release và Debug | Cả hai cấu hình biên dịch với `-Wall -Wextra`, 0 cảnh báo (bao gồm `ImageMapGrid`, `ImageMapLoader`, `FileDialog` và liên kết `comdlg32`). |
| CTest Release và Debug | Đạt 3/3 bộ (`physics`, `combat`, `waves`), gồm `testImageMapBuilder` và `testImageMapsNavigationAndWaves` kiểm thử ảnh nền sáng, ảnh nền tối, gradient 16:9, ảnh đồng màu trắng, bàn cờ nhiễu `16×16` và hai file `assets/maps/sample_maze.png`, `assets/maps/sample_neon_fort.png`. |
| Kết xuất OpenGL ảnh mẫu & hit-test | Dựng và vẽ liên tiếp `sample_maze.png` (rev 1: `13×13`, 26 khối) và chuyển sang `sample_neon_fort.png` (rev 2: `16×12`, 15 khối) qua đủ 4 trạng thái với `glGetError() == GL_NO_ERROR`; kiểm tra `HUD::hitTestMenu` khớp cả 5 thẻ map và 2 nút `LOAD IMAGE`, `PLAY` ở `1280×720` và `882×553`. |
| `render_benchmark` Release | Chạy ổn định ở `1280×720` (wave 7, 8 robot): `F6=0 F7=0` 2,886 ms; `F6=0 F7=1` 2,203 ms; `F6=1 F7=0` 2,509 ms; `F6=1 F7=1` 2,585 ms. |
