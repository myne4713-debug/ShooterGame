# Câu hỏi và trả lời khi bảo vệ đồ án FPS 3D

Câu trả lời dùng cách xưng hô «nhóm em» để có thể luyện nói trực tiếp. Phần cuối có một số câu hỏi và đáp mẫu bằng tiếng Anh. Các số slide bên dưới theo [bản PowerPoint trắng–xanh](OpenGL-FPS-White-Blue-EN.pptx).

## Tổng quan và công nghệ

### 1. Mục tiêu của đồ án là gì? Nhóm đã hoàn thành những chức năng nào?

Nhóm em dùng một game FPS 3D để áp dụng các kiến thức đồ họa máy tính. Người chơi có thể vào đấu trường, di chuyển, nhảy, bật đèn pin, bắn và nạp đạn, rồi chơi lại sau Game Over. Các phần thể hiện kiến thức môn học rõ nhất là camera phối cảnh, robot ghép từ các khối, chiếu sáng Blinn–Phong và bóng đổ theo hướng mặt trời. [README](../README.md)

### 2. Vì sao nhóm chọn OpenGL 3.3 Core? GLFW, GLAD và GLM có vai trò gì?

OpenGL 3.3 Core cho phép nhóm em viết shader và trực tiếp tổ chức dữ liệu đỉnh, texture, framebuffer và các lượt vẽ. GLFW mở cửa sổ, tạo OpenGL context và nhận input. Sau khi context hiện hành, GLAD nạp địa chỉ các hàm OpenGL. GLM cung cấp vector và ma trận. CMake cấu hình và build dự án C++17. [Window.cpp](../src/core/Window.cpp), [CMakeLists.txt](../CMakeLists.txt)

### 3. Điều gì xảy ra trong một khung hình?

Nhóm em đọc sự kiện, cập nhật mô phỏng nếu đang chơi, rồi xóa bộ đệm màu và độ sâu. Nếu bóng đang bật, game dựng bản đồ độ sâu từ phía mặt trời. Sau đó game vẽ cảnh và bầu trời từ camera, vẽ hiệu ứng trong cảnh, vẽ mô hình súng và vẽ HUD cuối cùng. GLFW đổi bộ đệm để đưa khung hình lên màn hình. [Game.cpp](../src/core/Game.cpp), [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp), [WeaponRenderer.cpp](../src/gfx/WeaponRenderer.cpp)

## Hình học, camera và pipeline

### 4. Vertex shader và fragment shader làm gì trong đồ án?

Vertex shader đưa vị trí đỉnh về không gian clip và xuất vị trí thế giới, pháp tuyến, UV cùng tọa độ nhìn từ đèn. GPU nội suy các dữ liệu đó trên tam giác. Fragment shader dùng kết quả nội suy để lấy mẫu texture, tính ánh sáng, kiểm tra bóng và trộn màu sương. [phong.vert](../assets/shaders/phong.vert), [phong.frag](../assets/shaders/phong.frag)

### 5. Vì sao thứ tự nhân ma trận là `P × V × M × p`?

Vì shader dùng vector cột, ma trận bên phải tác động trước. `M` đặt điểm từ hệ cục bộ vào thế giới, `V` đổi sang hệ mắt, rồi `P` đưa nó vào không gian clip. Sau khi cắt hình và chia cho `w`, viewport ánh xạ tọa độ đã chuẩn hóa sang vùng pixel. Camera dùng phép chiếu phối cảnh với FOV dọc 75°. [phong.vert](../assets/shaders/phong.vert), [Camera.cpp](../src/gfx/Camera.cpp), [Window.cpp](../src/core/Window.cpp)

### 6. Vì sao lập phương có 24 đỉnh trong VBO dù chỉ có 8 góc? VAO, VBO và EBO khác nhau thế nào?

Một góc nằm trên ba mặt, mỗi mặt cần pháp tuyến và UV riêng. Game lưu 4 đỉnh cho mỗi mặt, tổng cộng 24 đỉnh. Sáu mặt có 12 tam giác, dùng 36 chỉ số. VBO chứa thuộc tính đỉnh, EBO chứa chỉ số tam giác, còn VAO ghi cách OpenGL đọc các thuộc tính và bộ đệm chỉ số khi vẽ. [Primitives.cpp](../src/gfx/Primitives.cpp), [Mesh.cpp](../src/gfx/Mesh.cpp)

### 7. Tại sao phải dùng ma trận nghịch đảo chuyển vị cho pháp tuyến?

Pháp tuyến phải vuông góc với các tiếp tuyến của mặt sau biến đổi. Nếu lấy phần tuyến tính của Model nhân trực tiếp với pháp tuyến khi co giãn không đều, góc vuông có thể mất. Nhóm em dùng `transpose(inverse(mat3(model)))` và chuẩn hóa lại pháp tuyến sau nội suy để tính ánh sáng đúng. [phong.vert](../assets/shaders/phong.vert), [phong.frag](../assets/shaders/phong.frag)

### 8. Camera tránh lật như thế nào? Vì sao đi chéo không nhanh hơn?

Chuột đổi yaw và pitch. Nhóm em giới hạn pitch ở ±89° để hướng nhìn không song song với trục đứng, nhờ vậy `lookAt` vẫn dựng được trục phải. Hướng đi bộ chỉ dùng yaw trên mặt phẳng XZ. Khi giữ W và D, game chuẩn hóa tổ hợp hai thành phần để độ lớn vận tốc vẫn bằng tốc độ đi bộ. [Camera.cpp](../src/gfx/Camera.cpp), [Config.h](../src/core/Config.h)

### 9. Tay chân robot quay quanh đúng khớp bằng cách nào?

Ma trận gốc đặt và quay toàn bộ robot. Từ gốc đó, game tịnh tiến đến vai hoặc hông, áp dụng phép quay tại khớp, rồi đặt lưới tay hoặc chân lệch khỏi khớp. Khi robot đi, góc vung dùng hàm sin với pha tăng theo quãng đường thực tế. Có thể chỉ vào khớp vai trên slide 4 khi trả lời. [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp), [Enemy.cpp](../src/world/Enemy.cpp)

### 10. Depth test, back-face culling và blending khác nhau ở đâu?

Depth test dùng độ sâu tại pixel để giữ bề mặt gần camera. Back-face culling bỏ tam giác quay mặt sau theo thứ tự đỉnh. Blending kết hợp màu nguồn với màu đã lưu. HUD dùng alpha blending để chữ và lớp phủ có độ trong suốt, còn tia lửa dùng blending cộng để các vệt chồng lên nhau sáng hơn. [Window.cpp](../src/core/Window.cpp), [HUD.cpp](../src/ui/HUD.cpp), [ImpactSparks.cpp](../src/gfx/ImpactSparks.cpp)

## Ánh sáng, texture và bóng đổ

### 11. Blinn–Phong tính ánh sáng thế nào? Khác Phong ở điểm nào?

Nhóm em cộng thành phần nền, khuếch tán và phản chiếu bóng. Khuếch tán dùng `max(N·L, 0)`. Với Blinn–Phong, phần bóng dùng góc giữa pháp tuyến `N` và vector nửa góc `H = normalize(L + V)`. Phong dùng góc giữa hướng nhìn và hướng phản xạ của ánh sáng. Trong game, shininess lớn làm vùng phản chiếu hẹp hơn. [phong.frag](../assets/shaders/phong.frag)

### 12. Nhóm mô phỏng mặt trời, đèn điểm và đèn pin khác nhau ra sao?

Mặt trời là nguồn định hướng nên mọi fragment dùng cùng hướng đến đèn. Đèn điểm tính hướng và khoảng cách riêng cho từng fragment, với suy giảm `1/(c + l·d + q·d²)`. Đèn pin là spotlight gắn với camera. Shader dùng hai góc cắt và `smoothstep` để làm mềm phần rìa nón sáng. Có thể nhấn **F** khi nhìn vào tường để minh họa. [phong.frag](../assets/shaders/phong.frag), [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp)

### 13. Mipmap, lọc dị hướng và texture lặp giải quyết vấn đề gì?

UV vượt 1 kết hợp với `GL_REPEAT` giúp lặp họa tiết trên tường và sàn. Mipmap tạo các phiên bản texture nhỏ dần để giảm nhấp nháy khi nhìn xa. Lọc dị hướng giúp giữ chi tiết khi nhìn xiên dọc sàn nếu GPU hỗ trợ. Khi không có ảnh bề mặt hợp lệ để đọc bằng stb_image, game tự sinh họa tiết. [Texture.cpp](../src/gfx/Texture.cpp), [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp)

### 14. Bóng đổ trong game được tạo thế nào? Vì sao dùng phép chiếu trực giao cho mặt trời?

Ở lượt đầu, nhóm em dựng camera từ phía mặt trời và lưu độ sâu của bề mặt gần đèn nhất vào texture 1024 × 1024. Ở lượt camera, shader chiếu fragment về hệ của đèn rồi so sánh độ sâu với texture đó. Một bề mặt gần đèn hơn có thể che ánh sáng trực tiếp tới fragment. Vì mặt trời được mô hình hóa bằng các tia song song, lượt độ sâu dùng phép chiếu trực giao. Có thể so sánh hai ảnh trên slide 8 hoặc dùng **F6**. [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp), [ShadowMap.cpp](../src/gfx/ShadowMap.cpp), [phong.frag](../assets/shaders/phong.frag)

### 15. Nếu bóng bị răng cưa hoặc xuất hiện hoa văn trên mặt đang sáng thì sao?

Độ phân giải hữu hạn của bản đồ độ sâu có thể làm rìa bóng răng cưa. Game lọc tuyến tính kết quả so sánh độ sâu để làm mềm rìa. Sai số độ sâu cũng có thể khiến mặt tự che chính nó, gọi là *shadow acne*. Nhóm em dùng `glPolygonOffset` trong lượt vẽ bản đồ độ sâu để giảm lỗi đó. Nếu độ lệch quá lớn, bóng có thể tách khỏi chân vật thể. [ShadowMap.cpp](../src/gfx/ShadowMap.cpp), [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp)

## Va chạm, thời gian và AI

### 16. Vì sao dùng AABB? Khi người chơi đi chéo vào tường thì sao?

AABB là hộp bao có cạnh song song với các trục, phù hợp với tường, thùng và cột của đấu trường. Game nới hộp vật cản theo kích thước thân rồi quét vị trí chân dọc từng trục. Nếu trục X chạm tường, chuyển động theo X dừng ở mặt cản còn Z vẫn có thể tiếp tục. Nhờ vậy người chơi trượt dọc tường. [AABB.cpp](../src/physics/AABB.cpp), [Player.cpp](../src/world/Player.cpp)

### 17. Làm sao game biết phát bắn trúng thùng hay robot?

Hitscan kiểm tra tia xuất phát từ mắt người chơi với các hộp bao. Theo slab method, mỗi cặp mặt hộp cho một khoảng tham số `t`. Nếu các khoảng của ba trục giao nhau với `t ≥ 0`, tia chạm hộp. Game chọn điểm chạm gần nhất trong tầm. Trên slide 7, tia chạm thùng trước nên robot phía sau không mất máu. Vệt sáng được vẽ sau để biểu diễn kết quả đó. [AABB.cpp](../src/physics/AABB.cpp), [WaveManager.cpp](../src/world/WaveManager.cpp), [WeaponRenderer.cpp](../src/gfx/WeaponRenderer.cpp)

### 18. Mô phỏng 60 bước mỗi giây có nghĩa là game luôn chạy 60 FPS không?

Bước mô phỏng dài `1/60` giây. Vòng lặp tích lũy thời gian và có thể chạy 0, 1 hoặc nhiều bước trước khi vẽ một khung hình. FPS đo tốc độ vẽ khung hình thực tế. Bước cố định giúp giữ chuyển động và va chạm nhất quán khi tốc độ vẽ thay đổi. [Game.cpp](../src/core/Game.cpp), [Config.h](../src/core/Config.h)

### 19. Robot đi vòng qua cột bằng thuật toán nào? Vì sao chọn Floyd–Warshall?

Nhóm em nới các hộp vật cản theo bán kính robot, đặt nút gần góc hộp và chỉ nối những nút có đường đi thẳng thông thoáng. Với bản đồ tĩnh và ít nút, Floyd–Warshall tính trước khoảng cách ngắn nhất giữa mọi cặp góc, mất `O(n³)` thời gian và `O(n²)` bộ nhớ. Khi chơi, robot đi thẳng tới mục tiêu trên mặt sàn nếu đường thông thoáng. Nếu bị chắn, nó chọn góc trung gian theo tổng khoảng cách tới mục tiêu. Khi người chơi đứng trên thùng, game dịch mục tiêu ra sát mép thùng để robot có thể tiếp cận. [ArenaRoutes.cpp](../src/world/ArenaRoutes.cpp)

### 20. Điều kiện nào khiến robot chuyển từ đuổi sang tấn công?

Robot có bốn trạng thái `Idle`, `Chase`, `Attack` và `Dead`. Robot bắt đầu tấn công khi người chơi đủ gần và đoạn từ ngực robot tới mắt người chơi không bị hộp vật cản chặn. Khi hết tầm hoặc mất đường nhìn, robot quay lại đuổi theo. Trạng thái tấn công dùng bộ đếm thời gian giữa các lần gây sát thương. [Enemy.cpp](../src/world/Enemy.cpp)

## Hiệu ứng, tối ưu và kiểm thử

### 21. Vì sao vết trúng, mô hình súng và HUD được vẽ ở các lượt khác nhau?

Vết trúng và tia lửa nằm trong không gian 3D nên có thể bị cảnh che. Sau khi vẽ chúng, nhóm em xóa riêng bộ đệm độ sâu để vẽ mô hình súng trong hệ mắt mà không bị tường cắt vào. Các bộ phận của súng vẫn so sánh độ sâu với nhau. HUD vẽ cuối bằng chiếu trực giao, tắt depth test để luôn đọc được đạn, máu và điểm. [WeaponRenderer.cpp](../src/gfx/WeaponRenderer.cpp), [HUD.cpp](../src/ui/HUD.cpp)

### 22. Instancing đã giúp gì? Nhóm kiểm tra tối ưu HUD ra sao?

Instancing cho phép vẽ nhiều bản sao của một quad trong một lệnh. Vertex shader dùng `gl_InstanceID` để chọn ký tự hoặc tính hướng của từng tia lửa. Trong cảnh đo wave 7, nhóm em giảm từ 56 xuống 20 lệnh vẽ HUD. Tám cặp ảnh trước và sau tối ưu, gồm bốn màn hình ở hai kích thước, khớp từng pixel. [hud.vert](../assets/shaders/hud.vert), [sparks.vert](../assets/shaders/sparks.vert), [PROGRESS.md](../docs/PROGRESS.md)

### 23. Số 2,574 ms và 60 FPS trên slide 10 đo điều gì?

**2,574 ms** là trung vị thời gian kết xuất cảnh wave 7 với 8 robot khi bật bóng và tia lửa. **P95 là 3,811 ms**, tức ít nhất 95% số mẫu trong lần đo hoàn thành trong khoảng đó. Benchmark đo phần gửi lệnh vẽ và chờ GPU bằng `glFinish`. **60 FPS** là số đọc từ game đang chạy thật. Cả hai kết quả ghi nhận ở 1280 × 720 trên Intel UHD Graphics 770, với thiết lập đo khác nhau. [RenderBenchmark.cpp](../tests/RenderBenchmark.cpp), [PROGRESS.md](../docs/PROGRESS.md)

### 24. Nhóm kiểm thử những gì? Đồ án có giới hạn nào?

Nhóm em chạy CTest cho physics, combat và waves. Phần kiểm tra đồ họa có resize, thu nhỏ và khôi phục cửa sổ, bật tắt bóng và tia lửa, và đi hết vòng chơi từ menu đến Game Over rồi chơi lại. AABB xấp xỉ thân robot bằng một hộp, còn tìm đường hiện dựa trên các vật cản tĩnh của một đấu trường nhỏ. Số đo hiệu năng trong báo cáo là kết quả trên máy nghiệm thu. [PROGRESS.md](../docs/PROGRESS.md), [Enemy.cpp](../src/world/Enemy.cpp), [ArenaRoutes.cpp](../src/world/ArenaRoutes.cpp)

## Câu trả lời mẫu bằng tiếng Anh

**What computer graphics techniques does your project demonstrate?**

We generate meshes for the arena and the robots, render them through a first-person perspective camera, and shade the surfaces with Blinn–Phong lighting. We also use shadow mapping for the sun and an orthographic projection for the HUD.

**How do you render the sun's shadows?**

We first render a depth texture from the sun's point of view. In the camera pass, we transform each visible point into light space and compare its depth with that texture. A closer surface can block the direct sunlight.

**How does the game decide whether a shot hits a robot?**

The game casts a ray from the camera and checks it against the boxes around the arena objects and robots. The nearest intersection decides what the shot hits. If a crate is closer than the robot, the crate blocks the shot.

**How does a robot find its way around a crate?**

We build a small visibility graph around the corners of the obstacles and precompute the shortest distances between them with Floyd–Warshall. During play, the robot moves directly toward its goal on the floor if the path is clear. If the way is blocked, it picks a reachable corner that gives the shortest route to the goal.

**How did you measure performance?**

We measured frame rendering in a wave-seven scene at 1280 by 720 on Intel UHD Graphics 770. The median render time was about 2.6 milliseconds with shadows and sparks enabled. We recorded 60 frames per second in a separate live gameplay check.
