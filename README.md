# Đồ án Đồ họa máy tính — FPS 3D

Dự án dùng C++17 và OpenGL 3.3 Core. Nhấn **Enter** ở menu để vào đấu trường 1280×720. Robot cam chạy nhanh và có 50 HP, robot xanh đi chậm hơn và có 100 HP. Hạ một robot được lần lượt 100 hoặc 200 điểm. Khi hạ hết địch, đợt kế tiếp bắt đầu sau 1,8 giây và đạn dự trữ được cấp lại. HUD hiển thị máu, đạn, wave và điểm. Khi HP về 0, màn hình Game Over hiện điểm cuối ván.

## Build và chạy

Cần CMake từ 3.16, trình biên dịch C/C++ hỗ trợ C++17 và driver hỗ trợ OpenGL 3.3 Core. Chạy lệnh từ thư mục gốc của repo. Lần cấu hình đầu cần kết nối mạng để CMake tải GLFW 3.4 và GLM 1.0.1. GLAD và stb_image đã có trong `third_party/`.

### Windows với MinGW-w64

Đưa thư mục chứa `gcc`, `g++` và `mingw32-make` vào `PATH`. Với MSYS2 UCRT64, dùng thư mục `ucrt64\bin` của bản cài.

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
.\build\shooter.exe
```

### Windows với Visual Studio

Cài workload **Desktop development with C++**. Dùng thư mục build riêng để CMake lưu cấu hình của generator Visual Studio.

```powershell
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
cmake --build build-vs --config Release --parallel
.\build-vs\Release\shooter.exe
```

### Linux với X11

Trên Debian/Ubuntu, cài trình biên dịch, CMake và các header X11. Chạy chương trình trong phiên đồ họa có X11 hoặc XWayland.

```sh
sudo apt install build-essential cmake xorg-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/shooter
```

## Điều khiển và kiểm tra nhanh

| Thao tác | Kết quả mong đợi |
| --- | --- |
| Enter | Bắt đầu ở menu chính hoặc chơi ván mới từ Game Over. |
| Di chuột | Nhìn quanh. Góc ngẩng và cúi giới hạn ở ±89°. |
| W / S, A / D | Tiến / lùi, sang trái / phải trên sàn. Thử giữ W+D để đi chéo hoặc đi xiên vào cạnh thùng để trượt dọc cạnh. |
| Giữ Shift khi di chuyển | Chạy với tốc độ 7 đơn vị/giây. Tốc độ đi bộ là 4 đơn vị/giây. |
| Space | Nhảy khi đang đứng trên sàn hoặc nắp thùng. Nhả rồi nhấn lại sau khi tiếp đất để nhảy tiếp. |
| F | Bật hoặc tắt đèn pin. Đến gần tường rồi di chuột để thấy tâm và viền mềm của vùng sáng di chuyển theo hướng nhìn. |
| Nhấp / giữ chuột trái | Bắn một phát / bắn liên tiếp. Tâm đổi sang màu cam khi trúng địch hoặc bia tập. Thanh máu phía trên robot ngắn lại sau mỗi phát trúng. |
| R | Nạp đạn trong 1,2 giây. HUD hiện RELOAD và thanh tiến độ; số đạn chuyển từ dự trữ sang băng khi nạp xong. |
| F3 | Bật hoặc tắt số FPS ở góc trên bên trái. |
| F6 | Bật/tắt bóng do mặt trời tạo ra. Quan sát bóng chân robot và bóng của cột trên sàn. |
| F7 | Bật/tắt tia lửa tại điểm trúng. Bắn vào bia gần để thấy các vệt sáng tỏa ra rồi rơi và tắt dần. |
| Esc | Tạm dừng và thả chuột. Nhấn Esc lần nữa để tiếp tục. |
| Q | Thoát từ menu chính, màn hình tạm dừng hoặc Game Over. |
| Chuyển sang cửa sổ khác | Game tự tạm dừng. Khi quay lại, nhấn Esc để lấy lại chuột. |
| Kéo cạnh cửa sổ | Phối cảnh cập nhật theo tỉ lệ vùng vẽ mới. |
| Nút đóng cửa sổ | Thoát ở cả trạng thái chơi và tạm dừng. |

Có thể chỉnh độ nhạy chuột ở `kMouseDegreesPerPixel` và góc nhìn ở `kVerticalFovDegrees` trong `src/core/Config.h`, rồi build lại.

**So sánh hiệu ứng Milestone 8.** Hai hiệu ứng mặc định bật. Nhấn Enter, bắn một phát vào bia trước mặt rồi nhấn Esc ngay khi thấy tia lửa. Trong lúc tạm dừng, nhấn F6 hoặc F7 để so sánh riêng từng hiệu ứng trên cùng khung hình. Nhấn Esc để quan sát hạt tiếp tục chuyển động. Dùng F3 để đọc FPS ở kích thước cửa sổ đang thử.

**Thử va chạm.** Từ điểm xuất phát, tiến đến sát thùng gỗ thấp phía trước bên trái. Giữ hướng tiến và nhấn Space. Khi đã qua cạnh thùng, nhả phím để đứng trên nắp. Nhấn Space lần nữa để nhảy từ nắp, hoặc đi ngang qua mép để rơi về sàn. Thử đi chéo vào một cột cao để quan sát chuyển động trượt dọc mặt cột.

**Thử bắn và nạp.** Từ điểm xuất phát, bắn một phát vào bia. HUD đổi từ `12 / 42` sang `11 / 42`. Nhấn R rồi đợi thanh tiến độ đầy để thấy `12 / 41`. Mỗi phát trúng làm bia mất 25 trong 100 HP và sẫm màu hơn. Giữ chuột để quan sát súng giật lên. Bắn vào mặt thùng để xem tia dừng ở mặt gần và để lại vết trúng.

**Thử trọn một ván.** Hạ robot cam và robot xanh ở wave 1. HUD sẽ hiện 300 điểm rồi chuyển sang wave 2 với ba địch. Đứng trong tầm đánh để thấy HP giảm và viền đỏ mờ dần. Khi Game Over, nhấn Enter để kiểm tra điểm về 0, wave về 1, HP về 100 và đạn về 12 / 42.

**Chạy kiểm thử.** Sau khi build, chuyển vào thư mục build rồi chạy CTest. Với Visual Studio, dùng `build-vs` và chọn cấu hình đã build ở tùy chọn `-C`. Ba bộ `physics`, `combat` và `waves` chạy được khi không có màn hình.

```sh
cd build
ctest -C Release --output-on-failure
cd ..
```

Bản Debug bật `GL_CHECK()` để ghi mã lỗi OpenGL cùng tên file và dòng gọi. Để build Debug, đổi `Release` thành `Debug` trong lệnh của generator đang dùng.

### Đo thời gian vẽ ở wave đông

Sau khi cấu hình Release với `BUILD_TESTING=ON` (mặc định), có thể build riêng công cụ đo. Từ thư mục gốc trên Windows với MinGW:

```powershell
cmake --build build --target render_benchmark --parallel
.\build\render_benchmark.exe
```

Trên Linux chạy `./build/render_benchmark`. Với Visual Studio, thêm `--config Release` vào lệnh build, dùng thư mục `build-vs` và chạy `.\build-vs\Release\render_benchmark.exe`. Công cụ cần phiên đồ họa có OpenGL 3.3, dùng renderer của game để vẽ cùng một cảnh wave 7 có tám robot, HUD đang nạp và một cụm tia lửa. Mỗi tổ hợp F6/F7 có 30 khung làm nóng và 180 khung lấy mẫu ở 1280×720.

Đọc hai cột thời gian theo mili giây. Trung vị mô tả khung điển hình. P95 là thời gian mà ít nhất 95% số khung hoàn thành trong phép đo. Công cụ chờ GPU hoàn tất từng khung, bỏ qua AI, xử lý input và đổi buffer. Khi chơi trực tiếp, **F3** hiển thị FPS của toàn bộ vòng lặp với VSync bật. [Số đo và kết quả nghiệm thu](docs/PROGRESS.md#kiểm-thử-milestone-9)

Chạy từ thư mục gốc của repo hoặc thư mục chứa executable để chương trình tìm thấy các cặp shader `phong`, `skybox`, `shadow`, `sparks`, `weapon` và `hud` trong `assets/shaders/`. Sau khi sửa GLSL, build lại để đồng bộ asset, rồi mở lại chương trình. Lỗi biên dịch hiện tên file và log của driver trong terminal. Lỗi liên kết hiện tên cả hai file.

Bản mặc định tự sinh bốn texture bề mặt và sáu mặt cubemap bầu trời. Muốn thử ảnh riêng, đặt `floor.png`, `wall.png`, `wood.png` hoặc `metal.png` vào `assets/textures/`, build và chạy lại. Nên dùng ảnh vuông có mép ghép liền để họa tiết lặp đều. Dòng `Procedural texture for ...` trong console cho biết bề mặt nào đang dùng ảnh thủ tục vì file thiếu hoặc đọc lỗi.

Để lặp lại phép kiểm tra build sạch với MinGW, chạy `cmake --build build --clean-first --parallel` sau bước cấu hình. Mở `.\build\shooter.exe`, thử các thao tác trong bảng, nhấn Esc rồi Q. Sau khi thoát, nhập `$LASTEXITCODE` trong PowerShell. Giá trị mong đợi là `0`.

[Tiến độ](docs/PROGRESS.md) · [Ghi chú đồ họa](docs/GRAPHICS_NOTES.md) · [Quyết định thiết kế](docs/DECISIONS.md) · [Nguồn thư viện và tài nguyên](assets/CREDITS.md)
