# Công nghệ, kiến thức đồ họa và thuật toán

Đồ án là game FPS 3D cho môn Đồ họa máy tính. Người chơi di chuyển trong đấu trường, chiến đấu với robot theo từng đợt và theo dõi máu, đạn, điểm trên HUD. Tài liệu này đi cùng [slide tiếng Anh](OpenGL-FPS-White-Blue-EN.pptx) và [kịch bản thuyết trình](SPEAKER_SCRIPT.md).

## Công nghệ sử dụng

| Công nghệ | Vai trò trong đồ án |
| --- | --- |
| **C++17** | Cài đặt vòng lặp game, mô phỏng, quản lý tài nguyên và gửi lệnh vẽ. |
| **OpenGL 3.3 Core / GLSL 330 core** | OpenGL quản lý bộ đệm, texture và lệnh vẽ. Vertex shader biến đổi đỉnh. Fragment shader tính màu, ánh sáng và bóng. |
| **GLFW 3.4** | Tạo cửa sổ và OpenGL context, đọc chuột và bàn phím, nhận thay đổi kích thước framebuffer và hoán đổi bộ đệm. |
| **GLAD** | Nạp địa chỉ các hàm OpenGL sau khi GLFW tạo context. |
| **GLM 1.0.1** | Tính vector, tích vô hướng, tích có hướng và các ma trận biến đổi, camera, phép chiếu. |
| **stb_image** | Đọc texture ảnh tùy chọn từ đĩa. Với ảnh thiếu hoặc lỗi, game tự sinh họa tiết bề mặt. |
| **CMake ≥ 3.16 / CTest** | CMake dùng `FetchContent` cho GLFW và GLM, còn GLAD và stb_image nằm trong `third_party/`. CTest chạy các bộ kiểm thử physics, combat và waves. |

Xem cấu hình thực tế trong [CMakeLists.txt](../CMakeLists.txt), [Window.cpp](../src/core/Window.cpp) và [Texture.cpp](../src/gfx/Texture.cpp).

## Kiến thức đồ họa áp dụng

| Kiến thức | Cách áp dụng | Mã nguồn |
| --- | --- | --- |
| **Sinh lưới và vẽ theo chỉ số** | Tạo cube, plane, sphere và cylinder từ đỉnh và chỉ số tam giác. VBO chứa vị trí, pháp tuyến và UV. VAO mô tả cách đọc thuộc tính. EBO chứa chỉ số. | [Primitives.cpp](../src/gfx/Primitives.cpp), [Mesh.cpp](../src/gfx/Mesh.cpp) |
| **Biến đổi Model–View–Projection** | Với vector cột, `p_clip = P × V × M × p_local`. Model đặt vật thể vào thế giới, View chuyển về hệ mắt và Projection đưa điểm vào không gian clip. Viewport ánh xạ kết quả lên vùng pixel của framebuffer. | [phong.vert](../assets/shaders/phong.vert), [Window.cpp](../src/core/Window.cpp) |
| **Camera FPS và chiếu phối cảnh** | Chuột điều khiển yaw và pitch. Pitch giới hạn ở ±89°. `lookAt` dựng View, `perspective` dựng Projection với FOV dọc 75°. Tỉ lệ chiếu lấy từ kích thước framebuffer. Chuẩn hóa đầu vào đường chéo giữ cùng tốc độ trên mặt phẳng XZ. | [Camera.cpp](../src/gfx/Camera.cpp), [Config.h](../src/core/Config.h) |
| **Biến đổi phân cấp và hoạt ảnh** | Thân robot mang theo các ma trận con. Tay và chân quay quanh khớp vai và hông. Hàm sin nhận pha tăng theo quãng đường thực tế để tạo dáng đi. | [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp), [Enemy.cpp](../src/world/Enemy.cpp) |
| **Pháp tuyến và lấy mẫu texture** | Ma trận `transpose(inverse(mat3(model)))` giữ pháp tuyến vuông góc với bề mặt khi co giãn không đều. UV lặp tạo họa tiết sàn và tường. Lọc trilinear dùng mipmap để giảm nhấp nháy ở xa, còn lọc dị hướng giúp nhìn bề mặt xiên rõ hơn khi GPU hỗ trợ. | [phong.vert](../assets/shaders/phong.vert), [Texture.cpp](../src/gfx/Texture.cpp) |
| **Cubemap và sương mù** | Lấy mẫu bầu trời bằng hướng nhìn. Bỏ phần tịnh tiến của View để bầu trời trông như ở xa vô hạn khi camera di chuyển. Shader trộn màu cảnh với màu sương theo khoảng cách. | [skybox.vert](../assets/shaders/skybox.vert), [skybox.frag](../assets/shaders/skybox.frag), [phong.frag](../assets/shaders/phong.frag) |
| **Depth test, back-face culling và blending** | Depth test xử lý che khuất. Culling bỏ mặt sau. Vệt đạn, vết trúng và tia lửa dùng độ sâu của cảnh. Sau đó game xóa riêng bộ đệm độ sâu để vẽ mô hình súng trong hệ mắt. HUD vẽ cuối với chiếu trực giao và alpha blending. | [Window.cpp](../src/core/Window.cpp), [WeaponRenderer.cpp](../src/gfx/WeaponRenderer.cpp), [HUD.cpp](../src/ui/HUD.cpp) |
| **Billboard và instancing** | Thanh máu quay mặt về camera nhưng vẫn chịu depth test. Game vẽ nhiều hạt hoặc ký tự bằng các bản sao của một quad, chọn dữ liệu cho từng bản sao qua `gl_InstanceID`. | [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp), [sparks.vert](../assets/shaders/sparks.vert), [hud.vert](../assets/shaders/hud.vert) |

## Các thuật toán chính

### Kết xuất

- **Blinn–Phong.** Mỗi fragment nhận ánh sáng nền, khuếch tán và phản chiếu bóng. Với `N` là pháp tuyến, `L` là hướng tới đèn và `V` là hướng tới mắt, phần khuếch tán dùng `max(N·L, 0)`. Phần bóng dùng `max(N·H, 0)^shininess` với `H = normalize(V + L)` trên mặt hướng về đèn. Đèn điểm suy giảm theo khoảng cách và đèn pin dùng `smoothstep` để làm mềm biên nón sáng. [phong.frag](../assets/shaders/phong.frag)
- **Shadow mapping hai lượt.** Lượt đầu chiếu trực giao từ mặt trời, ghi độ sâu gần nhất vào texture 1024 × 1024 trong một framebuffer riêng. Lượt camera đưa điểm đang tô màu về hệ của đèn rồi so sánh độ sâu. `glPolygonOffset` giảm lỗi tự đổ bóng, còn lấy mẫu so sánh với lọc tuyến tính làm mềm rìa bóng. [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp), [ShadowMap.cpp](../src/gfx/ShadowMap.cpp), [phong.frag](../assets/shaders/phong.frag)
- **Hạt thủ tục.** Vertex shader dùng chỉ số instance và góc vàng để phân bố hướng tia lửa quanh pháp tuyến bề mặt. Vị trí theo tuổi hạt là `p(t) = p₀ + v₀t + ½gt²`. Mỗi quad kéo dài theo vận tốc chiếu lên màn hình và mờ dần theo thời gian. [ImpactSparks.cpp](../src/gfx/ImpactSparks.cpp), [sparks.vert](../assets/shaders/sparks.vert)

### Va chạm và gameplay

- **Quét AABB theo từng trục.** Hộp bao AABB có các cạnh song song với trục tọa độ. `sweepAxis` nới hộp tĩnh theo kích thước thân rồi quét vị trí chân dọc một trục đến mặt cản gần nhất. Giải riêng X và Z cho phép trượt dọc tường. Quét theo Y xử lý tiếp đất và chạm trần. [AABB.cpp](../src/physics/AABB.cpp), [Player.cpp](../src/world/Player.cpp)
- **Giao tia–AABB bằng slab method.** Với tia `p(t) = o + td`, mỗi cặp mặt song song của hộp xác định khoảng `t` mà tia nằm giữa hai mặt. Giao ba khoảng X, Y, Z với `t ≥ 0` cho tham số chạm đầu tiên. Hitscan chọn giao điểm gần nhất giữa vật cản và robot, nên thùng có thể che robot phía sau. Pháp tuyến tại mặt chạm dùng để đặt vết trúng và định hướng tia lửa. [AABB.cpp](../src/physics/AABB.cpp), [Level.cpp](../src/world/Level.cpp), [WaveManager.cpp](../src/world/WaveManager.cpp)
- **Bước mô phỏng cố định.** Vòng lặp tích lũy thời gian từ mỗi khung hình rồi chạy các bước mô phỏng dài `1/60` giây. Chuyển động đứng dùng `Δy = v·Δt − ½g·Δt²`, cập nhật vận tốc và kiểm tra va chạm trong từng bước. [Game.cpp](../src/core/Game.cpp), [Player.cpp](../src/world/Player.cpp)
- **Lấy mẫu đều trên đĩa.** Độ tản của phát bắn trong game lấy mẫu trên mặt phẳng vuông góc với hướng nhìn. Với `u` ngẫu nhiên đều trong `[0, 1]`, bán kính tỉ lệ với `√u` để mật độ mẫu đều theo diện tích. [Weapon.cpp](../src/world/Weapon.cpp)

### Điều khiển robot và đợt chơi

- **Đồ thị tầm nhìn và Floyd–Warshall.** Game nới vật cản theo bán kính robot và đặt nút gần các góc. Hai nút nối với nhau khi đoạn thẳng giữa chúng đủ thông thoáng. Lúc khởi tạo, Floyd–Warshall tính khoảng cách ngắn nhất giữa mọi cặp nút bằng phép cập nhật `d[i][j] = min(d[i][j], d[i][k] + d[k][j])`. Với `n` nút, bước này tốn `O(n³)` thời gian và `O(n²)` bộ nhớ. Khi chơi, robot đi thẳng tới mục tiêu nếu đường thoáng. Nếu bị chắn, nó chọn góc có tổng khoảng cách nhỏ nhất từ vị trí hiện tại tới mục tiêu. [ArenaRoutes.cpp](../src/world/ArenaRoutes.cpp)
- **Máy trạng thái hữu hạn.** Robot chuyển giữa `Idle`, `Chase`, `Attack` và `Dead`. Nó chỉ tấn công khi người chơi trong tầm và đoạn kiểm tra tầm nhìn không bị vật cản che. Khi mất điều kiện này, robot quay lại `Chase`. [Enemy.cpp](../src/world/Enemy.cpp)
- **Sinh robot theo đợt.** Game duyệt danh sách điểm xuất hiện, bỏ điểm quá gần người chơi và xen kẽ hai loại Runner và Guard. Số robot bắt đầu từ 2, tăng dần đến tối đa 8. Đợt tiếp theo bắt đầu sau 1,8 giây kể từ khi hạ hết robot. [WaveManager.cpp](../src/world/WaveManager.cpp), [Config.h](../src/core/Config.h)

## Liên hệ với slide và demo

| Slide | Kiến thức có thể giải thích bằng hình hoặc demo |
| --- | --- |
| **4–5** | Chỉ khớp vai trên hình robot để giải thích ma trận cha–con. Di chuyển quanh thùng để thấy phối cảnh và hệ tọa độ camera. |
| **6** | So sánh ba quả cầu để chỉ ra vai trò của ánh sáng nền, khuếch tán và phản chiếu. Nhấn **F** để quan sát đèn pin đi theo camera. |
| **7** | Dùng sơ đồ tia đi tới thùng để giải thích slab method và nguyên tắc chọn giao điểm gần nhất. |
| **8–9** | So sánh hai ảnh cùng góc nhìn khi tắt và bật bóng. Dùng ảnh phát bắn để phân biệt hiệu ứng trong cảnh, mô hình súng và HUD. |
| **10** | Đọc riêng số liệu benchmark render và FPS khi chơi. Benchmark chờ GPU bằng `glFinish`, còn FPS khi chơi bao gồm toàn bộ vòng lặp. |

Xem thêm [ghi chú đồ họa theo milestone](../docs/GRAPHICS_NOTES.md), [kết quả kiểm thử và benchmark](../docs/PROGRESS.md) và [cách chạy, điều khiển](../README.md).
