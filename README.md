# Đồ án Đồ họa máy tính — FPS 3D

Dự án mô phỏng game bắn súng góc nhìn thứ nhất (FPS 3D) viết bằng **C++17** và **OpenGL 3.3 Core** (GLSL 330 core). Tại màn hình menu chính, người chơi chọn một trong **4 bản đồ dựng sẵn** hoặc **nạp một bức ảnh bất kỳ** để sinh bản đồ 3D trực tiếp với khung cảnh xem trước phía sau, sau đó nhấn **Enter** (hoặc nhấp nút **ENTER PLAY**) để bước vào đấu trường 1280×720.

## Tính năng chính

- **Chọn map & sinh map từ ảnh bất kỳ (Image-to-Map)**: Hỗ trợ chuyển đổi giữa 4 bản đồ dựng sẵn (`1`–`4`, phím mũi tên/`WASD`, nhấp chuột) hoặc tạo bản đồ từ ảnh PNG/JPG/JPEG/BMP/TGA bằng cách **kéo thả ảnh vào cửa sổ**, nhấn **`O` / nhấp `O LOAD IMAGE`**, chọn mục **`5. IMAGE MAP`**, hoặc truyền đường dẫn ảnh qua tham số dòng lệnh (`.\build\shooter.exe assets\maps\sample_maze.png`).
- **Đường ống đồ họa 3D**: Chiếu sáng **Blinn-Phong** (1 nguồn định hướng mặt trời/trăng, 3 đèn điểm theo bản đồ, 1 đèn chớp nòng súng 50 ms và đèn pin gắn camera), **shadow mapping** trực giao co giãn theo từng bản đồ (`F6`), **cubemap bầu trời** 6 mặt sinh theo chủ đề từng map hoặc bảng màu của ảnh, sương mù khoảng cách, **tia lửa va chạm** bằng instancing (`F7`) và vết đạn (decal).
- **Va chạm & chiến đấu**: Va chạm AABB trượt dọc tường và nhảy lên bệ/thùng nhiều bậc theo bước vật lý cố định 1/60 giây; bắn **hitscan** kèm độ tản, độ giật camera, tracer và mô hình súng góc nhìn thứ nhất có lắc theo bước chạy.
- **Địch & vòng lặp wave**: Hai loại robot (`Runner` cam 50 HP / 100 điểm, `Guard` xanh 100 HP / 200 điểm) dẫn đường vòng vật cản qua đồ thị góc `ArenaRoutes`, tăng dần số lượng theo đợt (nghỉ 1,8 giây và cấp lại đạn dự trữ giữa các wave).
- **HUD trực giao 2D**: Font bitmap 5×7 tự sinh hiển thị HP, đạn (`12 / 42`), tên bản đồ đang chơi, wave, điểm, FPS (`F3`), viền đỏ nhận sát thương và các màn hình Menu / Pause / Game Over.

## Danh sách 4 bản đồ dựng sẵn & ảnh mẫu

Cấu hình 4 bản đồ dựng sẵn nằm trong `src/world/MapCatalog.h` và `src/world/MapCatalog.cpp`; bộ chuyển ảnh sang bản đồ nằm trong `src/world/ImageMapGrid.h/.cpp` và `src/world/ImageMapLoader.h/.cpp`:

| Phím | Bản đồ | Kích thước (Cao tường) | Chủ đề ánh sáng & bầu trời | Đặc điểm bố cục |
| --- | --- | --- | --- | --- |
| `1` | `ARENA` | `28×28` (4,0) | Ban ngày — trời xanh nhạt, nắng chếch | Đấu trường cổ điển với thùng gỗ thấp (`0,70`), cột cao và bia tập trung tâm. |
| `2` | `WAREHOUSE` | `34×26` (5,0) | Hoàng hôn — trời cam tím, bóng đổ dài | Kho container chữ nhật, có bệ gỗ 2 bậc (`0,70` và `1,35`) nhảy parkour lên được. |
| `3` | `CITADEL` | `32×32` (5,5) | Ban đêm — ánh trăng xanh, đèn điểm tương phản | Pháo đài tối với trụ kim loại trung tâm, nổi bật đèn pin (`F`), chớp súng và tia lửa. |
| `4` | `LABYRINTH` | `30×30` (3,8) | Buổi trưa — cát sa mạc ấm, nắng gắt | Tàn tích nhiều tường ngăn có cửa thông khoang, xuất phát góc Tây-Nam nhìn chéo (`-45°`). |
| `5` | `IMAGE MAP` | `20..32` (tỉ lệ theo ảnh) | Tự động sinh từ bảng màu của ảnh | Đọc ảnh qua `stb_image`, lọc hộp về lưới chiến thuật, phân ngưỡng Otsu tách vật cản 3 tầng cao, đảm bảo liên thông 4-hướng (BFS) và gộp khối chữ nhật. |

Có sẵn 2 ảnh mẫu trong `assets/maps/` để thử nhanh:
- `assets/maps/sample_maze.png` (128×128): sơ đồ tường tối trên nền đá cát sáng.
- `assets/maps/sample_neon_fort.png` (128×96): bản vẽ tường sáng trên nền tối (tự đảo cực tính nhận diện tường sáng).

## Build và chạy

Cần CMake từ 3.16, trình biên dịch C/C++ hỗ trợ C++17 và driver hỗ trợ OpenGL 3.3 Core. Chạy lệnh từ thư mục gốc của repo. Lần cấu hình đầu cần kết nối mạng để CMake tải GLFW 3.4 và GLM 1.0.1. GLAD và `stb_image.h` đã có sẵn trong `third_party/`.

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
| `1`–`5` (hoặc Numpad `1`–`5`) ở Menu | Chọn trực tiếp 1 trong 4 bản đồ dựng sẵn hoặc bản đồ sinh từ ảnh (`5`); nếu chưa nạp ảnh thì mở hộp thoại chọn ảnh. |
| `O` / Nhấp `O LOAD IMAGE` (ở Menu) | Mở hộp thoại hệ điều hành (Windows) để chọn ảnh PNG/JPG/JPEG/BMP/TGA và dựng ngay map 3D từ ảnh đó. |
| Kéo thả file ảnh vào cửa sổ | Nạp ảnh ngay từ mọi màn hình và chuyển về Menu với nền 3D của map vừa tạo. |
| Mũi tên / `W` `S` `A` `D` / Nhấp thẻ ở Menu | Chuyển qua lại hoặc chọn thẻ bản đồ bằng chuột. |
| `Enter` / Numpad `Enter` / Nhấp `ENTER PLAY` | Bắt đầu chơi từ Menu trên bản đồ đang chọn, hoặc chơi lại ván mới từ màn hình Game Over. |
| `M` (khi Pause hoặc Game Over) | Quay lại Menu chính để chọn bản đồ khác. |
| Di chuột | Nhìn quanh. Góc ngẩng và cúi giới hạn ở ±89°. |
| `W` / `S`, `A` / `D` | Tiến / lùi, sang trái / phải trên sàn. Giữ `W+D` để đi chéo cùng tốc độ hoặc đi xiên vào cạnh thùng để trượt dọc cạnh. |
| Giữ `Shift` khi di chuyển | Chạy nhanh với tốc độ 7 đơn vị/giây (đi bộ là 4 đơn vị/giây). |
| `Space` | Nhảy khi đang đứng trên sàn hoặc nắp thùng/bệ. Nhả rồi nhấn lại sau khi tiếp đất để nhảy tiếp. |
| `F` | Bật hoặc tắt đèn pin gắn camera. Rõ nhất trong bản đồ `CITADEL` ban đêm hoặc khi soi sát tường. |
| Nhấp / giữ chuột trái | Bắn phát đơn / bắn liên tiếp. Tâm đổi sang màu cam khi trúng địch hoặc bia tập; thanh máu trên đầu robot ngắn lại sau mỗi phát trúng. |
| `R` | Nạp đạn trong 1,2 giây. HUD hiện `RELOAD` và thanh tiến độ; đạn chuyển từ dự trữ sang băng khi nạp xong. |
| `F3` | Bật hoặc tắt số FPS ở góc trên bên trái. |
| `F6` | Bật/tắt bóng đổ từ nguồn mặt trời/trăng (`ShadowMap`). Có thể bật/tắt cả khi đang tạm dừng để so sánh trên cùng khung hình. |
| `F7` | Bật/tắt tia lửa tại điểm trúng. Có thể bật/tắt cả khi đang tạm dừng. |
| `Esc` | Tạm dừng và thả con trỏ chuột. Nhấn `Esc` lần nữa để tiếp tục. |
| `Q` | Thoát khi đang ở Menu chính, màn hình tạm dừng hoặc Game Over. |
| Chuyển sang cửa sổ khác | Game tự tạm dừng. Khi quay lại, nhấn `Esc` để lấy lại chuột. |
| Kéo cạnh cửa sổ | Phối cảnh và giao diện HUD tự động cập nhật theo tỉ lệ vùng vẽ mới. |
| Nút đóng cửa sổ | Thoát sạch ở mọi trạng thái. |

Có thể chỉnh độ nhạy chuột ở `kMouseDegreesPerPixel` và góc nhìn ở `kVerticalFovDegrees` trong `src/core/Config.h`, rồi build lại.

**Thử nạp map từ ảnh bất kỳ.** Tại Menu chính, nhấn `O` (hoặc nhấp `O LOAD IMAGE`) và chọn `assets\maps\sample_maze.png` rồi chọn `assets\maps\sample_neon_fort.png`, hoặc kéo thả trực tiếp một bức ảnh bất kỳ vào cửa sổ game. Quan sát tên map trên thẻ số 5, kích thước lưới/số khối, màu bầu trời và bố cục 3D thay đổi theo bảng màu của ảnh, sau đó nhấn `Enter` để vào chơi thử (hoặc chạy `.\build\shooter.exe assets\maps\sample_neon_fort.png`).

**Thử chọn và đổi bản đồ.** Tại Menu chính, lần lượt nhấn `1`, `2`, `3`, `4` hoặc nhấp từng thẻ để xem cảnh 3D phía sau đổi tức thời sang ngày (`ARENA`), hoàng hôn (`WAREHOUSE`), đêm (`CITADEL`) và sa mạc trưa (`LABYRINTH`). Chọn `WAREHOUSE` rồi nhấn `Enter`, chạy tới bệ gỗ ở giữa sân để thử nhảy liên tiếp 2 bậc (`0,70` lên `1,35`). Nhấn `Esc` rồi nhấn `M` để quay lại Menu, chọn `CITADEL` và bấm `F` để kiểm tra đèn pin trong màn đêm.

**So sánh hiệu ứng Milestone 8.** Hai hiệu ứng mặc định bật. Vào trận, bắn một phát vào bia trước mặt rồi nhấn `Esc` ngay khi thấy tia lửa. Trong lúc tạm dừng, nhấn `F6` hoặc `F7` để so sánh riêng từng hiệu ứng trên cùng khung hình. Nhấn `Esc` để quan sát hạt tiếp tục chuyển động. Dùng `F3` để đọc FPS ở kích thước cửa sổ đang thử.

**Thử va chạm (`ARENA`).** Từ điểm xuất phát, tiến đến sát thùng gỗ thấp phía trước bên trái. Giữ hướng tiến và nhấn `Space`. Khi đã qua cạnh thùng, nhả phím để đứng trên nắp. Nhấn `Space` lần nữa để nhảy từ nắp, hoặc đi ngang qua mép để rơi về sàn. Thử đi chéo vào một cột cao để quan sát chuyển động trượt dọc mặt cột.

**Thử bắn và nạp.** Từ điểm xuất phát, bắn một phát vào bia. HUD đổi từ `12 / 42` sang `11 / 42`. Nhấn `R` rồi đợi thanh tiến độ đầy để thấy `12 / 41`. Mỗi phát trúng làm bia mất 25 trong 100 HP và sẫm màu hơn. Giữ chuột để quan sát súng giật lên. Bắn vào mặt thùng để xem tia dừng ở mặt gần và để lại vết trúng.

**Thử trọn một ván.** Hạ robot cam và robot xanh ở wave 1. HUD sẽ hiện 300 điểm rồi chuyển sang wave 2 với ba địch. Đứng trong tầm đánh để thấy HP giảm và viền đỏ mờ dần. Khi Game Over, nhấn `Enter` để chơi lại chính bản đồ đó (điểm về 0, wave về 1, HP về 100, đạn về `12 / 42`), hoặc nhấn `M` để đổi sang bản đồ khác.

## Kiểm thử và đo hiệu năng

**Chạy kiểm thử.** Sau khi build, chạy CTest (với Visual Studio, thêm `-C Release` hoặc cấu hình đã build). Ba bộ kiểm thử `physics`, `combat` và `waves` chạy không cần màn hình, kiểm tra vật lý va chạm, chiến đấu hitscan, cũng như hình học/tọa độ spawn, nhảy 2 bậc, dẫn đường `ArenaRoutes`, chuyển wave trên toàn bộ 4 bản đồ dựng sẵn và các bản đồ sinh từ ảnh (gồm ảnh tổng hợp lẫn `sample_maze.png`, `sample_neon_fort.png`):

```powershell
ctest --test-dir build --output-on-failure
```

Bản Debug bật `GL_CHECK()` để ghi mã lỗi OpenGL cùng tên file và dòng gọi. Để build Debug, đổi `Release` thành `Debug` trong lệnh của generator đang dùng.

### Đo thời gian vẽ ở wave đông

Sau khi cấu hình Release với `BUILD_TESTING=ON` (mặc định), có thể build riêng công cụ đo. Từ thư mục gốc trên Windows với MinGW:

```powershell
cmake --build build --target render_benchmark --parallel
.\build\render_benchmark.exe
```

Trên Linux chạy `./build/render_benchmark`. Với Visual Studio, thêm `--config Release` vào lệnh build, dùng thư mục `build-vs` và chạy `.\build-vs\Release\render_benchmark.exe`. Công cụ cần phiên đồ họa có OpenGL 3.3, dùng renderer của game để vẽ cùng một cảnh wave 7 có tám robot, HUD đang nạp và một cụm tia lửa. Mỗi tổ hợp F6/F7 có 30 khung làm nóng và 180 khung lấy mẫu ở 1280×720.

Đọc hai cột thời gian theo mili giây. Trung vị mô tả khung điển hình. P95 là thời gian mà ít nhất 95% số khung hoàn thành trong phép đo. Công cụ chờ GPU hoàn tất từng khung, bỏ qua AI, xử lý input và đổi buffer. Khi chơi trực tiếp, **F3** hiển thị FPS của toàn bộ vòng lặp với VSync bật. Xem thêm [Số đo Milestone 9](docs/PROGRESS.md#kiểm-thử-milestone-9) và [Kiểm thử đa bản đồ](docs/PROGRESS.md#kiểm-thử-tính-năng-chọn-map-và-đa-bản-đồ).

## Shader và tài nguyên

Chạy từ thư mục gốc của repo hoặc thư mục chứa executable để chương trình tìm thấy các cặp shader `phong`, `skybox`, `shadow`, `sparks`, `weapon` và `hud` trong `assets/shaders/`. Sau khi sửa GLSL, build lại để đồng bộ asset, rồi mở lại chương trình. Lỗi biên dịch hiện tên file và log của driver trong terminal. Lỗi liên kết hiện tên cả hai file.

Bản mặc định tự sinh bốn texture bề mặt (`floor`, `wall`, `wood`, `metal`) và bốn cubemap bầu trời (mỗi cubemap 6 mặt 64×64) cho 4 bản đồ. Muốn thử ảnh riêng, đặt `floor.png`, `wall.png`, `wood.png` hoặc `metal.png` vào `assets/textures/`, build và chạy lại. Nên dùng ảnh vuông có mép ghép liền để họa tiết lặp đều. Dòng `Procedural texture for ...` trong console cho biết bề mặt nào đang dùng ảnh thủ tục vì file thiếu hoặc đọc lỗi.

Để lặp lại phép kiểm tra build sạch với MinGW, chạy `cmake --build build --clean-first --parallel` sau bước cấu hình. Mở `.\build\shooter.exe`, thử các thao tác trong bảng, nhấn `Esc` rồi `Q`. Sau khi thoát, nhập `$LASTEXITCODE` trong PowerShell; giá trị mong đợi là `0`.

[Tiến độ](docs/PROGRESS.md) · [Ghi chú đồ họa](docs/GRAPHICS_NOTES.md) · [Quyết định thiết kế](docs/DECISIONS.md) · [Nguồn thư viện và tài nguyên](assets/CREDITS.md)
