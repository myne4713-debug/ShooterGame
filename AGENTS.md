# AGENTS.md — Dự án game bắn súng 3D (OpenGL + C++)

Tài liệu này là hướng dẫn bắt buộc cho mọi agent (Codex, Claude Code, v.v.) làm việc trong repo này.
Đọc toàn bộ file trước khi viết code. Khi có mâu thuẫn giữa prompt và file này, hỏi lại hoặc ghi rõ giả định trong `docs/DECISIONS.md`.

## 1. Bối cảnh

- Đây là đồ án môn **Đồ họa máy tính**. Mục tiêu: mô phỏng game bắn súng góc nhìn thứ nhất (FPS) bằng **OpenGL + C++**.
- Giảng viên chấm dựa trên việc **áp dụng kiến thức đồ họa**: biến đổi hình học, camera, chiếu, ánh sáng, texture, depth/blending, shader. Vì vậy code phải **rõ ràng, có chú thích giải thích kiến thức đồ họa**, không giấu logic trong thư viện lớn.
- Nhóm sinh viên phải hiểu và bảo vệ được code. Ưu tiên đơn giản, dễ đọc hơn là "thông minh".

## 2. Công nghệ cố định

| Hạng mục            | Lựa chọn                                                                               |
| ------------------- | -------------------------------------------------------------------------------------- |
| Ngôn ngữ            | C++17                                                                                  |
| Đồ họa              | OpenGL 3.3 Core Profile (shader GLSL 330 core)                                         |
| Cửa sổ/input        | GLFW 3.3+                                                                              |
| Loader              | GLAD (GL 3.3 core)                                                                     |
| Toán học            | GLM                                                                                    |
| Ảnh texture         | stb_image (header-only)                                                                |
| Chữ HUD             | Tự vẽ bằng bitmap font texture hoặc stb_easy_font (không bắt buộc ImGui)               |
| Âm thanh (tùy chọn) | miniaudio (header-only)                                                                |
| Build               | CMake ≥ 3.16, dùng `FetchContent` cho GLFW/GLM, vendor sẵn glad/stb vào `third_party/` |

**Không dùng**: OpenGL fixed-function (`glBegin/glEnd`, `glMatrixMode`...), engine game có sẵn (Unity, Unreal, Irrlicht, Ogre...), thư viện vật lý lớn (Bullet, PhysX). Tự viết va chạm đơn giản.

## 3. Cấu trúc thư mục

```
.
├── AGENTS.md
├── CMakeLists.txt
├── README.md                # cách build/chạy/điều khiển (tiếng Việt)
├── docs/
│   ├── DECISIONS.md         # quyết định thiết kế + giả định
│   ├── GRAPHICS_NOTES.md    # mapping: tính năng game -> kiến thức môn học
│   └── PROGRESS.md          # checklist tiến độ theo milestone
├── assets/
│   ├── shaders/             # *.vert, *.frag
│   ├── textures/
│   └── sounds/
├── third_party/             # glad, stb
└── src/
    ├── main.cpp
    ├── core/                # Window, Input, Timer, Game (vòng lặp chính)
    ├── gfx/                 # Shader, Mesh, Texture, Camera, Renderer, Skybox, Light
    ├── world/               # Level, Entity, Player, Enemy, Weapon, Projectile, Pickup
    ├── physics/             # AABB, Ray, collision helpers
    └── ui/                  # HUD, Crosshair, TextRenderer, Menu
```

Mỗi module có `.h` + `.cpp` riêng. Mỗi class một file. Không file nào quá ~400 dòng; nếu vượt thì tách.

## 4. Quy ước code

- Tên class `PascalCase`, hàm/biến `camelCase`, hằng `kPascalCase`, thành viên private `m_camelCase`.
- Dùng RAII cho tài nguyên GL: destructor gọi `glDelete*`. Cấm copy với class giữ GL handle (xóa copy ctor, cho phép move).
- Không dùng biến toàn cục (trừ hằng). Truyền phụ thuộc qua constructor/tham số.
- Không `new/delete` thô; dùng `std::unique_ptr`, `std::vector`.
- Mọi lời gọi GL quan trọng (tạo shader, framebuffer) phải kiểm tra lỗi và log rõ ràng. Có macro `GL_CHECK()` bật trong Debug.
- Cập nhật logic theo **delta time** (`dt` giây), không phụ thuộc FPS. Dùng fixed timestep (1/60) cho va chạm/vật lý nếu cần.
- Chú thích bằng **tiếng Việt** ở những chỗ áp dụng kiến thức đồ họa (ví dụ: "Nhân ma trận theo thứ tự Projection _ View _ Model vì GLM dùng cột-chủ đạo").
- Không để magic number: gom vào `src/core/Config.h`.
- Mỗi lần commit nhỏ, thông điệp theo dạng `feat(gfx): ...`, `fix(world): ...`.

## 5. Build & kiểm tra

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/shooter            # Windows: build\Release\shooter.exe
```

Trước khi báo hoàn thành bất kỳ milestone nào, agent phải:

1. Build sạch, **không warning** với `-Wall -Wextra` (MSVC: `/W4`).
2. Chạy được game (nếu môi trường không có display, ghi rõ không chạy được và liệt kê cách tự kiểm tra).
3. Cập nhật `docs/PROGRESS.md` và `docs/GRAPHICS_NOTES.md`.
4. Đường dẫn asset dùng tương đối từ thư mục chạy, có `ASSET_DIR` định nghĩa qua CMake.

## 6. Các skill (playbook) của agent

Mỗi skill là một quy trình lặp lại được. Khi làm việc liên quan, agent làm theo đúng các bước.

### Skill 1 — `setup-project`

**Khi dùng**: khởi tạo repo.

1. Tạo cấu trúc thư mục mục 3 và `CMakeLists.txt` (FetchContent GLFW + GLM, glad/stb trong `third_party`).
2. Tạo cửa sổ 1280×720, context GL 3.3 core, clear màu, vòng lặp có delta time, ESC để thoát.
3. Bật `GL_DEPTH_TEST`, `GL_CULL_FACE`, vsync, callback resize cập nhật viewport.
4. Done khi: cửa sổ hiện ổn định, in phiên bản GL ra log.

### Skill 2 — `shader-pipeline`

**Khi dùng**: thêm hoặc sửa shader.

1. Viết class `Shader` (load từ file, compile, link, báo lỗi kèm tên file + log).
2. Có helper `setMat4/setVec3/setFloat/setInt`, cache uniform location.
3. Shader nằm ở `assets/shaders/`, đặt tên theo mục đích: `phong.vert/.frag`, `skybox.*`, `hud.*`, `unlit.*`.
4. Hỗ trợ hot-reload bằng phím F5 ở chế độ Debug (tùy chọn).
5. Done khi: lỗi shader được in rõ dòng/lý do, không crash im lặng.

### Skill 3 — `mesh-and-model`

**Khi dùng**: cần hình học mới.

1. Class `Mesh` (VAO/VBO/EBO) với layout cố định: `location 0 = position`, `1 = normal`, `2 = uv`.
2. Hàm sinh sẵn: `makeCube`, `makePlane(tiling)`, `makeSphere`, `makeCylinder`. Tính normal đúng, UV hợp lý.
3. Nhân vật/địch/súng dựng từ **primitive ghép phân cấp** (cube + cylinder có scene graph đơn giản) để thể hiện phép biến đổi phân cấp. Tải OBJ chỉ là tùy chọn, không bắt buộc.
4. Done khi: vẽ được mọi primitive với ánh sáng đúng, không bị lật mặt.

### Skill 4 — `camera-fps`

**Khi dùng**: điều khiển người chơi.

1. `Camera` lưu `position`, `yaw`, `pitch`; tính `front/right/up`; `getViewMatrix()` dùng `glm::lookAt`; `getProjection()` dùng `glm::perspective` (FOV 70–90°).
2. Chuột: `glfwSetInputMode(GLFW_CURSOR_DISABLED)`, nhạy chuột cấu hình được, **kẹp pitch** ±89°.
3. Phím: WASD di chuyển trên mặt phẳng XZ (không bay), Shift chạy, Space nhảy (trọng lực đơn giản), ESC mở menu pause.
4. Chuẩn hóa vector di chuyển chéo để không nhanh hơn.
5. Done khi: di chuyển mượt, không bị lật camera, không trượt kỳ lạ.

### Skill 5 — `lighting-phong`

**Khi dùng**: ánh sáng/vật liệu.

1. Cài **Blinn-Phong** trong fragment shader: ambient + diffuse + specular, có `Material` (màu, shininess).
2. Hỗ trợ: 1 directional light (mặt trời) + tối đa 4 point light (suy giảm constant/linear/quadratic) + 1 spot light (đèn pin gắn camera, bật tắt bằng F).
3. Dùng **normal matrix** = `transpose(inverse(mat3(model)))` khi biến đổi normal; ghi chú lý do trong code.
4. Hiệu ứng nòng súng nổ: tạo point light tạm thời ngắn (~50 ms) tại đầu nòng khi bắn.
5. Done khi: vật thể sáng/tối đúng theo hướng ánh sáng, specular di chuyển theo góc nhìn.

### Skill 6 — `texturing`

**Khi dùng**: gắn texture.

1. Class `Texture` dùng stb_image, `stbi_set_flip_vertically_on_load(true)`, sinh mipmap, wrap `GL_REPEAT`, filter `GL_LINEAR_MIPMAP_LINEAR`, bật anisotropic nếu có.
2. Sàn/tường dùng tiling; skybox dùng cubemap (`GL_TEXTURE_CUBE_MAP`).
3. Nếu chưa có asset: tạo texture thủ tục (checkerboard, noise) trong code để game vẫn chạy được.
4. Done khi: không méo UV, không đen texture, có fallback khi file thiếu.

### Skill 7 — `level-and-world`

**Khi dùng**: dựng bản đồ.

1. Level là một đấu trường kín (phòng + vài khối che chắn, cột, thùng) mô tả bằng danh sách box (`position`, `size`, `texture`) hoặc mảng grid 2D.
2. Mỗi box có `AABB` để va chạm. Có skybox hoặc màu trời + sương mù (fog) trong shader.
3. Có thể thêm bóng đổ bằng **shadow mapping** cho directional light (mục nâng cao, xem Milestone 8).
4. Done khi: người chơi đi lại, không xuyên tường, không rơi khỏi map.

### Skill 8 — `collision-physics`

**Khi dùng**: va chạm và bắn trúng.

1. Người chơi/địch: AABB hoặc capsule đơn giản vs AABB của level, giải va chạm theo từng trục (trượt dọc tường).
2. Hitscan: `Ray` (origin + direction từ camera) vs `AABB` (slab method) lấy `t` nhỏ nhất, so sánh với tường để chặn đạn xuyên tường.
3. Đạn vật lý (nếu có, ví dụ lựu đạn): tích phân Euler/semi-implicit với trọng lực, nảy khi chạm.
4. Done khi: bắn trúng địch đúng vị trí, không bắn xuyên tường, có unit test nhỏ cho `rayVsAABB`.

### Skill 9 — `weapons-and-combat`

**Khi dùng**: súng và sát thương.

1. Class `Weapon`: sát thương, tốc độ bắn, băng đạn, nạp đạn (R), độ giật (recoil) làm lệch pitch, độ tản đạn (spread).
2. Súng hiển thị ở góc nhìn thứ nhất: vẽ **view model** bằng ma trận riêng cố định trước camera (xóa depth hoặc dùng FOV riêng để không xuyên tường), có lắc nhẹ khi đi (weapon bob).
3. Hiệu ứng: tia đạn/tracer, flash nòng súng (quad billboard blending cộng), vết đạn (decal quad nhỏ dán vào tường).
4. Done khi: bắn/nạp/đổi trạng thái đúng, đếm đạn chính xác trên HUD.

### Skill 10 — `enemy-ai`

**Khi dùng**: kẻ địch.

1. State machine: `Idle → Chase → Attack → Dead`. Địch quay mặt về phía người chơi (tính `atan2`), di chuyển theo vector hướng.
2. Né vật cản đơn giản (trượt theo tường); không cần A\* ở bản cơ bản.
3. Spawn theo đợt (wave), tăng độ khó dần. Có thanh máu trên đầu (billboard).
4. Hoạt ảnh bằng biến đổi phân cấp (tay/chân lắc bằng `sin(time)`), chết thì ngã hoặc co lại.
5. Done khi: ít nhất 2 loại địch, wave chạy vòng lặp được, không kẹt tường vĩnh viễn.

### Skill 11 — `hud-ui`

**Khi dùng**: giao diện.

1. Vẽ 2D bằng `glm::ortho` với shader riêng, tắt depth test khi vẽ HUD, bật blending.
2. HUD gồm: tâm ngắm, máu, đạn (băng/dự trữ), điểm, wave hiện tại, FPS (phím F3).
3. Màn hình: Menu chính, Pause, Game Over (hiện điểm, nhấn Enter chơi lại).
4. Hiệu ứng nhận sát thương: viền đỏ mờ dần (blending).
5. Done khi: chữ rõ nét, không nhấp nháy, scale đúng khi đổi kích thước cửa sổ.

### Skill 12 — `postprocess-advanced` (nâng cao, tùy chọn)

**Khi dùng**: sau khi các milestone chính xong.

- Shadow mapping, framebuffer + bloom/vignette, MSAA (`glfwWindowHint(GLFW_SAMPLES, 4)`), normal mapping, particle (khói/tia lửa) bằng instancing.
- Mỗi tính năng bật/tắt được bằng phím hoặc config để dễ trình bày.

### Skill 13 — `docs-and-report`

**Khi dùng**: cuối mỗi milestone.

1. Cập nhật `README.md`: cách cài đặt, build trên Windows/Linux, bảng phím điều khiển.
2. Cập nhật `docs/GRAPHICS_NOTES.md` theo bảng: _Tính năng → Kiến thức đồ họa → File/hàm cài đặt_. Ví dụ: "Camera FPS → lookAt, perspective, Euler angles → `gfx/Camera.cpp`".
3. Ghi lại các lỗi đã gặp và cách sửa để nhóm dùng viết báo cáo.

## 7. Danh sách milestone (thứ tự bắt buộc)

| #   | Milestone                               | Skill    |
| --- | --------------------------------------- | -------- |
| 1   | Cửa sổ + vòng lặp + build chạy được     | 1        |
| 2   | Shader + vẽ cube có màu                 | 2, 3     |
| 3   | Camera FPS + sàn + vài khối             | 4, 7     |
| 4   | Ánh sáng Blinn-Phong + texture          | 5, 6     |
| 5   | Va chạm với level + nhảy                | 8        |
| 6   | Súng, bắn hitscan, HUD cơ bản           | 8, 9, 11 |
| 7   | Địch + wave + điểm + Game Over          | 10, 11   |
| 8   | Hiệu ứng nâng cao (shadow, particle...) | 12       |
| 9   | Hoàn thiện, tối ưu, tài liệu            | 13       |

Không nhảy cóc milestone. Mỗi milestone phải chạy được trước khi sang milestone tiếp theo.

## 8. Tiêu chí hoàn thành (Definition of Done)

- Build sạch trên ít nhất một nền tảng, không warning.
- Chạy ≥ 60 FPS trên máy phổ thông ở 1280×720.
- Không rò rỉ tài nguyên GL, không crash khi đổi kích thước/minimize.
- Có thể chơi trọn một vòng: Menu → chơi → chết → chơi lại.
- Tài liệu đầy đủ (mục 6, Skill 13).

## 9. Điều agent KHÔNG được làm

- Không thêm thư viện ngoài danh sách mục 2 khi chưa ghi vào `docs/DECISIONS.md` kèm lý do.
- Không sao chép nguyên khối code từ nguồn khác mà không ghi nguồn trong chú thích. Tham khảo (LearnOpenGL, v.v.) thì phải viết lại theo cấu trúc dự án và ghi nguồn.
- Không dùng tài nguyên (texture, âm thanh, model) có bản quyền không rõ ràng; ưu tiên tự tạo thủ tục hoặc nguồn CC0 và ghi vào `assets/CREDITS.md`.
- Không refactor lớn ngoài phạm vi milestone đang làm.
- Không xóa/ghi đè file của thành viên khác nếu chưa đọc và hiểu mục đích.
