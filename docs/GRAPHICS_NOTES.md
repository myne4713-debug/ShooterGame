# Ghi chú đồ họa

Khi trình bày một khung hình, bắt đầu ở `runGame` trong `src/core/Game.cpp`. Sau các bước mô phỏng 1/60 giây, `LevelRenderer::draw` vẽ độ sâu từ mặt trời (F6), cảnh nhìn từ camera và bầu trời. `WeaponRenderer::draw` dùng độ sâu của cảnh cho hạt, decal và tracer, rồi xóa riêng depth để vẽ súng trong hệ mắt. Cuối cùng, `HUD::draw` chiếu trực giao và trộn giao diện lên ảnh. Các bảng dưới đây chỉ tới mã thực hiện từng bước.

| Tính năng | Kiến thức đồ họa | File / hàm cài đặt |
| --- | --- | --- |
| Tạo cửa sổ OpenGL | GLFW tạo context 3.3 Core và gắn nó với luồng hiện tại trước khi GLAD nạp địa chỉ hàm. | `src/core/Window.cpp` · `Window::Window` |
| Thay đổi kích thước | Viewport ánh xạ tọa độ thiết bị chuẩn hóa lên vùng pixel của framebuffer. Dùng kích thước framebuffer để xử lý đúng mật độ điểm ảnh. | `src/core/Window.cpp` · `resizeViewport` |
| Nền và độ sâu | Mỗi khung hình xóa bộ đệm màu và đặt lại độ sâu. Nhờ vậy, khi depth test so sánh một fragment với độ sâu đã lưu tại cùng pixel, kết quả chỉ phụ thuộc vào khung hình đang vẽ. | `src/core/Game.cpp` · `runGame`, `src/core/Window.cpp` · `Window::Window` |
| Loại mặt sau | Bật face culling. OpenGL mặc định coi thứ tự đỉnh ngược chiều kim đồng hồ là mặt trước và loại mặt sau. | `src/core/Window.cpp` · `Window::Window` |
| Chuyển màu khi khởi động | Cộng thời gian giữa các khung hình để tính độ sáng theo giây. | `src/core/Game.cpp` · `runGame` |
| Trình bày khung hình | Vẽ vào back buffer rồi hoán đổi hai buffer. Swap interval 1 yêu cầu đồng bộ với chu kỳ quét màn hình. | `src/core/Window.cpp` · `glfwSwapInterval`, `src/core/Game.cpp` · `glfwSwapBuffers` |

## Milestone 2 — Từ đỉnh đến màu pixel

| Tính năng | Kiến thức đồ họa | File / hàm cài đặt |
| --- | --- | --- |
| Tạo program | Driver biên dịch từng stage rồi kiểm tra giao diện giữa hai stage khi link. Trạng thái và info log cho biết lỗi nằm ở bước nào. | `src/gfx/Shader.cpp` · `compileStage`, `Shader::Shader` |
| Truyền ma trận | Vị trí uniform gắn với program đã link nên có thể tra một lần và dùng lại khi giá trị thay đổi. | `src/gfx/Shader.cpp` · `uniformLocation`, `setMat4` |
| Vẽ theo chỉ số | VBO chứa vị trí, pháp tuyến và UV. VAO ghi cách đọc từng thuộc tính và EBO chứa chỉ số. Mỗi mặt cube dùng 4 đỉnh cho 2 tam giác, cả khối cần 24 đỉnh và 36 chỉ số. | `src/gfx/Mesh.cpp` · `Mesh::Mesh`, `draw` · `src/gfx/Primitives.cpp` · `makeCube` |
| Biến đổi và chiếu cube | Với vector cột, tích P × V × M biến đổi đỉnh lần lượt từ hệ cục bộ sang hệ thế giới, hệ mắt, rồi không gian clip. Trong bản cuối, có thể quan sát phép quay trên các khớp robot khi chúng di chuyển. | `src/gfx/LevelRenderer.cpp` · `drawEnemies`, `assets/shaders/phong.vert` · `main` |
| Màu của sáu mặt | Ánh xạ từng thành phần pháp tuyến từ [−1, 1] sang [0, 1] tạo sáu màu RGB. Fragment shader ghi màu nhận được vào framebuffer. | `assets/shaders/unlit.vert`, `assets/shaders/unlit.frag` · `main` |
| Pháp tuyến và UV | Mặt phẳng dùng UV từ 0 tới tiling. Mặt cầu có pháp tuyến hướng theo bán kính. Mặt trụ tách vành nắp khỏi thân để giữ cạnh sắc. Ở đường nối vòng tròn, hai đỉnh trùng vị trí mang U = 0 và U = 1. | `src/gfx/Primitives.cpp` · `makePlane`, `makeSphere`, `makeCylinder` |

## Milestone 3 — Camera FPS và đấu trường

| Tính năng | Kiến thức đồ họa | File / hàm cài đặt |
| --- | --- | --- |
| Nhìn bằng chuột | Yaw đổi hướng ngang, pitch đổi góc ngẩng. Kẹp pitch giữ hướng nhìn khác phương với trục Y để tích có hướng trong `lookAt` xác định được trục phải. | `src/core/Input.cpp` · `mouseDelta`, `src/gfx/Camera.cpp` · `look`, `getViewMatrix` |
| Đi và chạy theo hướng nhìn | Tính hướng tiến trên XZ từ yaw và hướng phải bằng tích có hướng. Chuẩn hóa tổ hợp hai phím trước khi nhân tốc độ với dt để đi chéo và đi thẳng cùng tốc độ. | `src/gfx/Camera.cpp` · `groundDirection`, `src/core/Game.cpp` · `runGame`, `src/world/Player.cpp` · `step` |
| Phối cảnh khi resize | FOV dọc 75° cố định. Tỉ lệ rộng/cao lấy theo pixel framebuffer để cạnh khối hộp giữ đúng tỉ lệ. | `src/gfx/Camera.cpp` · `getProjection`, `src/core/Game.cpp` · `runGame` |
| Sàn, thùng và cột | Tái sử dụng một plane và một cube trên GPU. Ma trận T × S biến cube đơn vị thành từng hộp. AABB lấy từ tâm ± nửa kích thước để chặn chuyển động ngang tại đúng vị trí vẽ. | `src/gfx/LevelRenderer.cpp` · `drawSurface`, `src/world/Level.cpp` · `Level::Level`, `src/physics/AABB.cpp` · `sweepAxis` |
| Ô sàn và sương mù | UV chọn texel của mẫu sàn lặp. Fragment shader tính khoảng cách từ vị trí thế giới nội suy tới mắt, rồi trộn dần màu bề mặt với màu trời. | `src/gfx/Texture.cpp` · `surfacePixels`, `assets/shaders/phong.vert`, `phong.frag` · `main` |

## Milestone 4 — Ánh sáng và texture

| Tính năng | Kiến thức đồ họa | File / hàm cài đặt |
| --- | --- | --- |
| Pháp tuyến của tường và cột | Nhân pháp tuyến với nghịch đảo chuyển vị của phần tuyến tính của Model để giữ vuông góc với mặt khi co giãn không đều. Chuẩn hóa lại sau nội suy. | `assets/shaders/phong.vert`, `phong.frag` · `main` |
| Vật liệu Blinn-Phong | Màu nền và khuếch tán lấy từ texel nhân màu vật liệu. Thành phần khuếch tán phụ thuộc N·L. Thành phần bóng phụ thuộc N·H, với H là hướng giữa L và V. Shininess lớn làm vệt bóng hẹp hơn. | `src/gfx/Material.h`, `src/world/Level.cpp`, `assets/shaders/phong.frag` · `blinnPhong` |
| Mặt trời và đèn điểm | Mặt trời có cùng hướng tới mọi fragment. Với mỗi đèn điểm, tính hướng từ fragment tới đèn và nhân cường độ với 1/(c + l·d + q·d²). | `src/gfx/LevelRenderer.cpp` · `LevelRenderer`, `assets/shaders/phong.frag` · `attenuation`, `main` |
| Đèn pin và chớp | Đèn pin lấy vị trí và hướng từ camera, làm mềm biên giữa hai góc cắt bằng smoothstep. Chớp dùng một điểm trong hệ trục của mắt, chuyển sang tọa độ thế giới lúc bắn và bật theo dt. | `src/gfx/Camera.cpp` · `front`, `src/gfx/WeaponRenderer.cpp` · `recordShot`, `flashPosition`, `src/gfx/LevelRenderer.cpp` · `draw` |
| Tiling, mipmap và lọc dị hướng | Chọn hai kích thước nằm trong mặt phẳng của từng mặt để nhân UV. GL_REPEAT lặp họa tiết. Hai mức mip liền kề được nội suy để giảm nhấp nháy ở xa. Lọc dị hướng giữ thêm chi tiết khi nhìn dọc sàn. | `src/gfx/Texture.cpp` · `Texture`, `src/gfx/LevelRenderer.cpp` · `drawSurface`, `assets/shaders/phong.vert` |
| Cubemap | Lấy mẫu sáu mặt bằng vector hướng. Bỏ tịnh tiến trong View để bầu trời ở xa vô hạn. Đặt z clip bằng w và tắt ghi depth khi vẽ từ bên trong cube để bầu trời chỉ phủ phần nền. | `src/gfx/Texture.cpp` · `makeSky`, `src/gfx/LevelRenderer.cpp` · `draw`, `assets/shaders/skybox.vert`, `skybox.frag` |

## Milestone 5 — Từ hình học vẽ đến va chạm 3D

| Tính năng | Kiến thức đồ họa và hình học | File / hàm cài đặt |
| --- | --- | --- |
| Biên vật cản | Cube đơn vị sau phép co giãn và tịnh tiến có hai góc đối diện ở tâm ± nửa kích thước. Chính hai góc này tạo AABB trong hệ thế giới. | `src/world/Level.cpp` · `Level::Level`, `src/gfx/LevelRenderer.cpp` · `drawSurface` |
| Trượt dọc tường | Nới hộp tĩnh theo kích thước thân rồi quét điểm chân. Chỉ trục gặp mặt cản bị chặn. Hai trục kia vẫn có thể dịch chuyển. | `src/physics/AABB.cpp` · `sweepAxis`, `src/world/Player.cpp` · `step` |
| Nhảy, tiếp đất, chạm trần | Quãng đường đứng trong một bước là v·dt − g·dt²/2. Khi quét xuống gặp nắp hộp, đặt chân đúng lên mặt; khi quét lên gặp mặt dưới, dừng ở độ cao vừa đủ cho cả thân. | `src/world/Player.cpp` · `step` |
| Camera theo người chơi | Vị trí mắt lấy từ chân sau khi giải va chạm cộng độ cao mắt theo trục Y thế giới. View vì thế phản ánh độ cao thực cả khi đứng trên thùng. | `src/world/Player.cpp` · `eyePosition`, `src/core/Game.cpp` · `runGame` |
| Tia và hộp | Mỗi cặp mặt song song giới hạn một khoảng tham số t. Giao ba khoảng với nửa trục t ≥ 0 cho tiếp điểm đầu tiên. | `src/physics/AABB.cpp` · `rayVsAABB`, `tests/PhysicsTests.cpp` · `testRay` |

## Milestone 6 — Một phát bắn từ camera đến HUD

| Tính năng | Kiến thức đồ họa | File / hàm cài đặt |
| --- | --- | --- |
| Ngắm và che khuất | Lấy mẫu trên đĩa trong hệ trục phải/lên của camera rồi chuẩn hóa hướng tia. Chọn t nhỏ nhất trong phạm vi bắn. Trục của mặt vào cho pháp tuyến để định hướng decal, kể cả trên sàn. | `src/world/Weapon.cpp` · `shotDirection`, `src/world/Level.cpp` · `hitScan`, `src/physics/AABB.cpp` · `rayVsAABB` |
| Súng góc nhìn thứ nhất | Thân, tay cầm và nòng dùng ma trận con của cùng một gốc trong hệ mắt. Quãng đường đi thật trên sàn điều khiển pha lắc. Xóa depth sau khi đã vẽ cảnh để súng luôn hiển thị và các bộ phận vẫn che nhau đúng. | `src/gfx/WeaponRenderer.cpp` · `advance`, `draw` |
| Tia và vết trúng | Định hướng khối mảnh dọc đoạn nòng–điểm trúng. Xoay quad theo pháp tuyến mặt và dịch nhẹ ra ngoài để tránh hai mặt có cùng độ sâu. Khi vẽ hiệu ứng trong cảnh, giữ depth test và tắt ghi depth. | `src/gfx/WeaponRenderer.cpp` · `frame`, `decalModel`, `draw` |
| Chớp nòng | Quad song song màn hình có mặt nạ theo UV, dùng blending cộng. Đèn điểm trong cảnh và tracer dùng cùng vị trí nòng đã đổi sang hệ thế giới. | `src/gfx/WeaponRenderer.cpp` · `recordShot`, `draw`, `src/gfx/LevelRenderer.cpp` · `draw`, `assets/shaders/weapon.frag` |
| HUD | Chiếu trực giao, tắt depth test và bật alpha blending. Mỗi ký tự lấy mẫu một ô bitmap trong texture, dùng cùng tỉ lệ X/Y khi resize. | `src/ui/HUD.cpp` · `makeFont`, `draw`, `src/gfx/Texture.cpp` · `makeMask`, `assets/shaders/hud.vert`, `hud.frag` |

## Milestone 7 — Robot trong thế giới và giao diện ván chơi

| Tính năng | Kiến thức đồ họa | File / hàm cài đặt |
| --- | --- | --- |
| Quay mặt và đuổi theo người chơi | Góc yaw lấy từ atan2 của độ lệch X/Z. Đoạn nối hai điểm được so với các AABB đã nới để chọn lối đủ rộng. | `src/world/Enemy.cpp` · `step`, `src/world/ArenaRoutes.cpp` · `clear`, `waypoint` |
| Dáng đi và hoạt ảnh chết | Vai và hông là các gốc quay con của ma trận thân. Pha sin tăng theo quãng đường đi thật, tay và chân vung ngược nhau. Co ma trận gốc làm mọi bộ phận nhỏ dần về điểm chân. | `src/gfx/LevelRenderer.cpp` · `drawEnemies`, `src/world/Enemy.cpp` · `deathScale` |
| Thanh máu billboard | Chuyển hai trục màn hình về hệ thế giới bằng chuyển vị phần quay của View. Hai quad đặt trên đầu địch chịu phép chiếu phối cảnh và depth test nên bị tường che. | `src/gfx/LevelRenderer.cpp` · `drawEnemies` |
| Chọn địch trúng tia | So sánh khoảng cách giao với AABB của địch sống và mặt tĩnh gần nhất trước khi trừ HP. | `src/world/WaveManager.cpp` · `hitScan`, `src/world/Level.cpp` · `hitScan` |
| Viền mất máu và màn hình ván chơi | Vẽ quad trực giao theo kích thước framebuffer. Alpha của viền đỏ giảm theo thời gian, còn lớp nền tối giúp chữ menu và điểm cuối dễ đọc trên cảnh 3D. | `src/ui/HUD.cpp` · `draw`, `overlay` |

## Milestone 8 — Bóng mặt trời và tia lửa tại điểm trúng

| Tính năng | Kiến thức đồ họa | File / hàm cài đặt |
| --- | --- | --- |
| Thu độ sâu từ phía mặt trời | Chiếu trực giao giữ các tia song song. Depth test trong FBO giữ mặt gần đèn nhất cho mỗi texel. Sau lượt này khôi phục viewport cửa sổ để phối cảnh giữ đúng tỉ lệ khi resize. | `src/gfx/ShadowMap.cpp` · `ShadowMap`, `src/gfx/LevelRenderer.cpp` · `drawShadowMap`, `assets/shaders/shadow.vert` |
| Bóng của vật thể và robot chuyển động | Dùng chung ma trận bộ phận ở lượt độ sâu và lượt màu. Đổi tọa độ clip của đèn về [0,1], so sánh độ sâu có bias theo góc mặt. Sampler so sánh lọc tuyến tính nội suy độ sáng giữa các texel ở rìa bóng. | `src/gfx/LevelRenderer.cpp` · `drawEnemies`, `assets/shaders/phong.vert`, `phong.frag` · `main` |
| Hạt thủ tục bằng instancing | Một lần vẽ lặp quad cho mỗi cụm. Chỉ số instance chọn vận tốc trong bán cầu theo pháp tuyến. Vị trí tại tuổi t bằng p₀ + v₀t + ½gt². | `src/gfx/ImpactSparks.cpp` · `emit`, `advance`, `draw`, `src/gfx/Mesh.cpp` · `drawInstanced`, `assets/shaders/sparks.vert` |
| Vệt sáng có độ sâu | Cộng độ lệch quad trong hệ mắt theo vận tốc chiếu lên màn hình. Mặt nạ elip theo UV và alpha theo tuổi làm hạt mờ dần. Blending cộng cho phép hạt chồng sáng lên nhau, còn depth test giữ chúng sau vật che. | `src/gfx/ImpactSparks.cpp` · `draw`, `assets/shaders/sparks.vert`, `sparks.frag` · `main` |

## Milestone 9 — Vẽ chữ và đo chi phí một khung

| Tính năng | Kiến thức đồ họa | File / hàm cài đặt |
| --- | --- | --- |
| Chữ HUD theo nhóm | Mỗi instance dùng chung quad, atlas, màu và kích thước. Chỉ số instance chọn mã glyph và dịch quad theo bước sáu pixel của font. Chuỗi dài tiếp tục ở gốc của nhóm kế tiếp. Giữ thứ tự vẽ giữa các lớp giúp blending cho cùng kết quả. | `src/ui/HUD.cpp` · `text`, `drawQuads`, `assets/shaders/hud.vert`, `hud.frag` · `main` |
| Đo chi phí kết xuất | Vẽ lại cùng khung với từng tổ hợp bóng và hạt. Đồng bộ bằng `glFinish` trước khi dừng đồng hồ để phép đo bao gồm thời gian GPU hoàn tất lệnh. | `tests/RenderBenchmark.cpp` · `runBenchmark` |
