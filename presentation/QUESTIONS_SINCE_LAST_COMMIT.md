# Hỏi đáp sau lệnh commit gần nhất trong cuộc trò chuyện

Các câu hỏi được ghi nguyên văn, theo thứ tự gửi sau tin nhắn `commit` đã tạo commit `4d8d234`. Mỗi câu hỏi có phần trả lời và liên kết tới mã nguồn liên quan.

## 1. thuật toán cho những con bot di chuyển

Bot dùng **đồ thị tầm nhìn** (*visibility graph*) kết hợp với **Floyd–Warshall** để tìm đường.

1. Khi khởi tạo bản đồ, game nới hộp va chạm theo bán kính bot, đặt các điểm dẫn đường gần góc thùng và cột, rồi nối hai điểm nếu đoạn thẳng giữa chúng thông thoáng. Floyd–Warshall tính trước khoảng cách ngắn nhất giữa mọi cặp điểm. [ArenaRoutes.cpp](../src/world/ArenaRoutes.cpp)
2. Khi đuổi người chơi, bot đi thẳng đến mục tiêu trên mặt sàn nếu đường thoáng. Nếu bị chắn, bot chọn một góc có thể tới được sao cho tổng **khoảng cách từ bot tới góc + khoảng cách từ góc đến mục tiêu qua đồ thị** nhỏ nhất. Ví dụ, nếu một chiếc thùng chắn đường, bot chọn đi qua phía cho tổng quãng đường ngắn hơn. [ArenaRoutes.cpp](../src/world/ArenaRoutes.cpp)
3. Mỗi bước, bot tiến về điểm đã chọn tối đa một đoạn bằng `tốc độ × dt`. Game quét va chạm riêng trên X và Z để bot có thể trượt dọc vật cản. Khi người chơi vào tầm và đường nhìn thông thoáng, bot chuyển từ `Chase` sang `Attack`. [Enemy.cpp](../src/world/Enemy.cpp)

## 2. thuật toán nào mô phỏng đạn bắn ra và nhận biết đạn bắn trúng mục tiêu

Game dùng **hitscan** để tính kết quả phát bắn ngay khi bắn và **slab method** để kiểm tra giao giữa tia và hộp bao AABB.

1. Game tạo tia từ mắt người chơi theo hướng camera, có thêm độ tản nhỏ. Tia có dạng `P(t) = O + tD`, trong đó `O` là điểm xuất phát và `D` là hướng đã chuẩn hóa. Khi đó `t` là khoảng cách dọc tia. [Game.cpp](../src/core/Game.cpp), [Weapon.cpp](../src/world/Weapon.cpp)
2. Với mỗi hộp, slab method tính khoảng `t` mà tia nằm giữa hai mặt đối diện trên từng trục X, Y và Z. Nếu ba khoảng giao nhau với `t ≥ 0`, đầu khoảng giao cho biết điểm chạm đầu tiên. [AABB.cpp](../src/physics/AABB.cpp)
3. Game chọn điểm chạm gần nhất trong tầm bắn. Nếu thùng nằm trước robot trên đường tia, thùng chặn phát bắn. Nếu robot là đối tượng chạm đầu tiên, game trừ máu ngay. Vệt đạn (*tracer*) được vẽ từ nòng đến điểm chạm để hiển thị kết quả. [Level.cpp](../src/world/Level.cpp), [WaveManager.cpp](../src/world/WaveManager.cpp), [WeaponRenderer.cpp](../src/gfx/WeaponRenderer.cpp)

## 3. trong dự án dùng thuật toán gì để mô phỏng súng vật cản và cái npc

| Thành phần | Thuật toán và cách áp dụng |
| --- | --- |
| **Súng** | Hitscan và slab method xác định điểm trúng. Mô hình súng ghép từ các khối trong hệ tọa độ camera và có hiệu ứng lắc khi đi. [Weapon.cpp](../src/world/Weapon.cpp), [WeaponRenderer.cpp](../src/gfx/WeaponRenderer.cpp), [WaveManager.cpp](../src/world/WaveManager.cpp) |
| **Vật cản** | Game dùng cùng vị trí và kích thước để vẽ tường, thùng, cột và tạo hộp bao AABB. Quét va chạm từng trục cho phép nhân vật trượt dọc mép tường. [Level.cpp](../src/world/Level.cpp), [AABB.cpp](../src/physics/AABB.cpp) |
| **NPC robot** | Đồ thị tầm nhìn và Floyd–Warshall giúp robot chọn đường vòng. Máy trạng thái hữu hạn điều khiển `Idle`, `Chase`, `Attack` và `Dead`. Mô hình robot dùng biến đổi phân cấp để tay chân xoay quanh khớp. [ArenaRoutes.cpp](../src/world/ArenaRoutes.cpp), [Enemy.cpp](../src/world/Enemy.cpp), [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp) |

## 4. ý tôi là thuật toán nào để dựng lên mô hình súng các vật cản và các npc

Game dùng **sinh lưới thủ tục** (*procedural mesh generation*). C++ tạo các đỉnh và chỉ số tam giác, rồi dùng phép co giãn, xoay và tịnh tiến để dựng mô hình. `makeCube()` chia 6 mặt thành 12 tam giác. `makeCylinder()` dùng `sin` và `cos` để tạo hai vòng đỉnh, nối chúng thành mặt bên rồi ghép tam giác cho hai nắp. [Primitives.cpp](../src/gfx/Primitives.cpp)

| Mô hình | Cách dựng |
| --- | --- |
| **Súng** | Ba khối hộp làm thân, tay cầm và chi tiết trên thân. Một khối trụ xoay 90° làm nòng. Tất cả dùng chung một ma trận gốc trong hệ tọa độ camera nên chuyển động cùng nhau. [WeaponRenderer.cpp](../src/gfx/WeaponRenderer.cpp) |
| **Vật cản** | Mỗi tường, thùng hoặc cột dùng lại một lưới lập phương đơn vị. Ma trận `T × S` co giãn lưới theo kích thước từng vật rồi tịnh tiến đến tâm. [Level.cpp](../src/world/Level.cpp), [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp) |
| **NPC robot** | Đầu và chân là khối hộp, tay là khối trụ. Thân Runner dùng khối trụ, thân Guard dùng khối hộp. Ma trận gốc đặt và xoay cả robot. Các ma trận con tại vai và hông cho phép tay chân xoay quanh từng khớp. [LevelRenderer.cpp](../src/gfx/LevelRenderer.cpp) |

**Cách trả lời khi bảo vệ.** “Nhóm em tự sinh lưới tam giác cho khối lập phương và khối trụ, rồi co giãn, xoay và tịnh tiến các khối để dựng mô hình. Với súng và robot, nhóm em dùng ma trận cha–con để các bộ phận gắn đúng vị trí khi chuyển động.”

## Yêu cầu chạy thử sau các câu hỏi

> chạy thử tao coi
