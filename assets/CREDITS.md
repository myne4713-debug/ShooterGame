# Nguồn tài nguyên

Màu nền do dự án chọn và tạo trực tiếp bằng OpenGL. Milestone 1 chưa dùng ảnh, âm thanh hoặc model bên ngoài.

Ở Milestone 2, nhóm tự tạo hình học trong `src/gfx/Primitives.cpp` và màu mặt trong `assets/shaders/unlit.vert`. Các màu giúp nhận biết từng mặt khi cube quay. Mốc này không cần tài nguyên thay thế.

Ở Milestone 3, sàn, tường, thùng và cột được dựng từ plane và cube của nhóm. Shader tự tạo ô sàn từ UV, tô màu theo hộp và pha sương với màu trời. Milestone 4 sẽ bổ sung vật liệu và texture cho các bề mặt này.

Ở Milestone 4, nhóm tự viết các mẫu ảnh lặp 128×128 trong `src/gfx/Texture.cpp` và hai cặp shader Phong, skybox. Sáu mặt cubemap 64×64 lấy màu theo độ cao của hướng lấy mẫu. Tất cả được sinh lúc chạy.

| Bề mặt | Mẫu mặc định | File có thể bổ sung trong `assets/textures/` |
| --- | --- | --- |
| Sàn | Đá lát vuông có hạt và đường ron | `floor.png` |
| Tường | Gạch xây so le | `wall.png` |
| Thùng | Ván gỗ có vân | `wood.png` |
| Cột | Tấm kim loại ghép có hạt mịn | `metal.png` |

Ở Milestone 6, mô hình súng và bia tập dùng các primitive tự sinh của dự án. Tracer dùng một khối mảnh; vòng vết trúng và chớp sáng được tô theo UV trên quad. Font HUD 5×7 được vẽ bằng bảng bit trong `src/ui/HUD.cpp` rồi đưa vào texture một kênh. Các tài nguyên này được tạo khi chạy và không cần tải tệp ngoài.

Ở Milestone 7, hai robot ghép từ cube và cylinder tự sinh. Thanh máu dùng quad của dự án, menu dùng thêm ký tự trong font bitmap 5×7. Màu, dáng đi và hoạt ảnh chết được tính trong code. Mốc này không cần thay asset.

Ở Milestone 8, ứng dụng tạo texture độ sâu từ hình học hiện tại của đấu trường. Tia lửa dùng quad của nhóm, quỹ đạo và mặt nạ sáng được tính trong shader `sparks`. Hai hiệu ứng chạy bằng dữ liệu sinh lúc vẽ và không cần bổ sung ảnh hoặc model.

| Thư viện | Phiên bản và giấy phép | Nguồn |
| --- | --- | --- |
| GLFW | 3.4, zlib/libpng | [glfw/glfw](https://github.com/glfw/glfw/tree/3.4) |
| GLM | 1.0.1, MIT hoặc Happy Bunny License | [g-truc/glm](https://github.com/g-truc/glm/tree/1.0.1) |
| GLAD | 0.1.36. Giấy phép bộ sinh và thông báo bản quyền Khronos đi kèm trong `third_party/glad/`. | [Dav1dde/glad](https://github.com/Dav1dde/glad/tree/v0.1.36) |
| stb_image | 2.32, MIT hoặc public domain. Giấy phép đầy đủ nằm cuối header. | [nothings/stb tại commit đã dùng](https://github.com/nothings/stb/blob/9600522de53973336fa033a289858a4354cbb04c/stb_image.h) |
