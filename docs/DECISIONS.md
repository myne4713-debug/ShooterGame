# Quyết định thiết kế

## Milestone 1

- **Phụ thuộc.** FetchContent tải GLFW 3.4 và GLM 1.0.1 theo commit cố định, kiểm tra SHA-256 trước khi giải nén. GLM dùng ở chế độ header-only. GLAD 0.1.36 sinh mã C cho OpenGL 3.3 Core từ đặc tả đi kèm bộ sinh, với danh sách extension rỗng. GLFW cung cấp địa chỉ hàm qua `glfwGetProcAddress`. Cả mã sinh và `stb_image.h` được lưu trong repo để máy build chỉ cần CMake và toolchain C/C++.
- **Vòng đời.** `main` khởi tạo GLFW. `Window` sở hữu cửa sổ bằng `std::unique_ptr` với hàm hủy `glfwDestroyWindow`. Khi rời phạm vi, cửa sổ và context được giải phóng trước `glfwTerminate`.
- **Thời gian.** Vòng lặp lấy hiệu hai lần đọc `glfwGetTime()` theo giây. Nền sáng dần khi khởi động để minh họa delta time. Khi thu nhỏ, chương trình chờ sự kiện và đặt lại mốc thời gian trước khung hình kế tiếp.
- **Asset.** CMake định nghĩa `ASSET_DIR` là đường dẫn tương đối `assets` và sao chép thư mục này cạnh executable sau khi liên kết. Có thể chạy từ thư mục gốc của repo hoặc từ thư mục chứa executable.
- **Linux.** Cấu hình mặc định dùng X11 để máy build chỉ cần một bộ header hệ thống cửa sổ.
- **Tiến độ.** Bảng ban đầu để trống theo yêu cầu khởi tạo. Milestone 1 được đánh dấu sau khi build sạch và chạy lại cả Release lẫn Debug trên Windows.

## Milestone 2

- Dùng góc nhìn cố định để nhìn thấy nhiều mặt cube. Model quay quanh tâm theo vận tốc góc nhân với delta time. Projection lấy tỉ lệ pixel thực của framebuffer ở mỗi lần vẽ.
- Dùng shader unlit và sinh màu từ pháp tuyến cục bộ. Vì pháp tuyến của bốn đỉnh trên một mặt giống nhau, mặt đó có màu đồng nhất trong suốt phép quay.
- Skill 3 yêu cầu bốn hàm sinh hình. Cả bốn dùng đơn vị kích thước 1 và được kiểm tra riêng. Cảnh Milestone 2 chỉ khởi tạo cube.
- Hai stage được biên dịch và liên kết lúc khởi động. Khi chỉnh shader, build để đồng bộ asset rồi chạy lại. Việc này cũng áp dụng khi chỉ có GLSL thay đổi.

## Milestone 3

- Đấu trường có sàn phẳng 28×28, bốn tường và bốn vật cản đứng. Mỗi hộp có tâm, kích thước và màu. Cùng dữ liệu đó xác định ma trận Model và hai góc của AABB, nên thay vị trí thùng sẽ dời cả hình vẽ lẫn biên chặn.
- Skill 4 yêu cầu Space và skill 7 yêu cầu giữ người chơi trong đấu trường. Mốc này dùng quỹ đạo nhảy trên sàn phẳng và kiểm tra biên hộp trên XZ. Chân nhảy cao nhất 0,96 đơn vị, trong khi hộp thấp nhất cao 1,6 đơn vị, nên tất cả hộp đều chặn đường ngang suốt cú nhảy. Va chạm 3D với mặt trên hộp và vật lý tổng quát thuộc Milestone 5.
- Esc mở lựa chọn tiếp tục hoặc thoát ngay trên thanh tiêu đề. Khi mất focus, game tạm dừng và trả con trỏ cho hệ điều hành. Lần lấy mẫu đầu tiên sau khi bắt lại chuột đặt mốc mới để giữ nguyên hướng nhìn.
- Màu theo từng loại khối, ô sàn thủ tục và sương mù khoảng cách giúp đọc chiều sâu của cảnh. FOV dọc là 75°. Thời gian mô phỏng mỗi lần cập nhật được giới hạn ở 0,1 giây để thao tác kéo cửa sổ lâu không làm người chơi bước vọt.

## Milestone 4

- Tính Blinn-Phong trong hệ thế giới. Pháp tuyến, hướng tới nguồn sáng và hướng tới mắt dùng cùng hệ trục. Vật liệu của từng hộp lưu loại bề mặt, màu và số mũ shininess. Sàn và mỗi hộp dùng cùng mật độ lặp, một chu kỳ ảnh trên hai đơn vị thế giới.
- Ba đèn điểm có vị trí cố định. Vị trí thứ tư dành cho chớp sáng 50 ms theo đồng hồ mô phỏng. Vì Milestone 4 chưa có mô hình súng, nhấp chuột trái đặt chớp tại điểm lệch phải, thấp và phía trước mắt. Đây là phép trình diễn ánh sáng của skill 5. Logic súng và đạn thuộc Milestone 6.
- Sàn, gạch, gỗ và kim loại có ảnh thủ tục dự phòng riêng. Cho phép thay độc lập bằng PNG. Sáu mặt của cubemap bầu trời được tính từ cùng một hàm theo độ cao hướng nhìn để đường chân trời liền mạch. Cubemap vẽ sau hình học với phép so sánh độ sâu LEQUAL.
- GLAD trong repo chỉ chứa API Core 3.3. Chương trình hỏi GLFW xem driver có extension lọc dị hướng EXT hoặc ARB, rồi giới hạn hệ số ở mức nhỏ hơn giữa 8 và giới hạn thiết bị. Trên driver khác, mipmap và lọc ba tuyến tính vẫn hoạt động.

## Milestone 5

- Thân người chơi là AABB rộng và sâu 0,64, cao 1,80 đơn vị. Điểm gốc đặt giữa hai chân, mắt cao hơn chân 1,65 đơn vị. Quay camera không đổi kích thước thân. Các AABB tĩnh lấy từ tâm và kích thước dùng để vẽ từng hộp. Sàn dùng đúng mặt phẳng hữu hạn của mesh ở y = 0.
- Quét đoạn di chuyển theo thứ tự X, Z rồi Y. Mỗi lần quét dừng ở mặt gần nhất trên đoạn nếu thân chồng lên vật cản ở hai trục còn lại. Tiếp xúc cạnh cho phép trượt. Va chạm khi đi xuống xác nhận có bề mặt đỡ, còn va chạm khi đi lên triệt tiêu vận tốc nhảy để người chơi rơi trở lại.
- Mô phỏng chạy ở bước cố định 1/60 giây. Vòng vẽ cộng dồn dt và chỉ lấy sự kiện Space khi có bước vật lý. Pause xóa thời gian dư. Trọng lực 12 và tốc độ bật nhảy 4,8 cho độ cao cực đại 0,96. Hạ riêng thùng gỗ trước–trái xuống 0,70 để kiểm tra nhảy lên một bề mặt có thể tiếp cận.
- Skill 8 yêu cầu unit test cho `rayVsAABB`. Hàm hình học này trả tham số t không âm đầu tiên trên tia, bằng 0 khi gốc đã nằm trong hộp. Việc nối tia với cơ chế bắn thuộc Milestone 6.

## Milestone 6

- Dùng một bia tập đứng yên để trình diễn sát thương và che khuất trước mốc AI. Bia có 100 HP, sẫm màu dần rồi chuyển xám sau bốn phát trúng. Bia đã hết HP vẫn là vật cản. Máu người chơi ở mốc này là 100. Điểm, wave, địch và Game Over thuộc Milestone 7.
- Súng hoạt động theo bước mô phỏng 1/60 giây. Băng có 12 viên, dự trữ ban đầu 42, hai phát liên tiếp cách nhau 0,15 giây. R ưu tiên bắt đầu nạp khi có chỗ trong băng và còn dự trữ. Chuyển đúng số viên còn thiếu sau 1,2 giây. Pause dừng cả nạp đạn và hiệu ứng.
- Hitscan bắt đầu ở mắt, dùng hướng có độ tản tại thời điểm bắn rồi mới tăng pitch. Hộp có giao điểm gần nhất quyết định điểm kết thúc và sát thương. Tracer nối từ nòng mô hình tới điểm đó. Khi nhìn vào trời, tia dừng ở giới hạn 60 đơn vị.
- Vẽ lần lượt cảnh, decal và tracer, mô hình góc nhìn rồi HUD. Giữ tối đa 48 decal trong cảnh. HUD dùng chữ bitmap thủ tục. Thanh tiêu đề vẫn giữ hướng dẫn Esc/Q của Milestone 3.

## Milestone 7

- Hai loại robot dùng cùng máy trạng thái. Robot cam có 50 HP, đi 2,6 đơn vị/giây và gây 8 sát thương. Robot xanh có 100 HP, đi 1,65 đơn vị/giây và gây 15 sát thương. Đòn đánh cần đủ gần và đoạn từ ngực robot tới mắt người chơi phải thông thoáng. Địch đứng trên sàn và có thể đi vòng thùng thấp.
- Các khối trong đấu trường đứng yên. Tính trước khoảng cách ngắn nhất giữa các góc của AABB đã nới theo bề rộng robot. Khi bị chắn, robot hướng tới góc nhìn thấy trên đường ngắn nhất. Quét X/Z vẫn chặn thân ở mặt hộp. Cách này xử lý được cả trường hợp robot đâm chính diện vào giữa một mặt tường.
- Đợt đầu có hai robot, mỗi đợt thêm một con đến giới hạn tám. Các điểm xuất hiện luân phiên quanh đấu trường và cách người chơi ít nhất năm đơn vị. Giữa hai đợt có 1,8 giây nghỉ và dự trữ được cấp lại thành 42 viên để có thể chơi nhiều đợt. Giữ nguyên băng hiện tại và HP qua các đợt.
- Bia tập của Milestone 6 vẫn cản tia. Chỉ lần chuyển một địch sang Dead mới ghi điểm. Robot chết co dần trong 0,55 giây và ngừng chặn tia ngay. Máu, điểm, đạn, vị trí, góc nhìn, bia và vết trúng được đặt lại khi chơi ván mới. F3 là tùy chọn hiển thị giữ qua các ván.

## Milestone 8

- Chọn bóng mặt trời và tia lửa tại điểm trúng để trình diễn hai kỹ thuật của skill 12. F6 và F7 là hai công tắc độc lập, mặc định bật và giữ lựa chọn qua các ván. Chúng hoạt động cả khi pause để nhóm có thể so sánh đúng cùng một tư thế và thời điểm.
- Các tia của nguồn định hướng song song. Một camera trực giao cố định bao trọn đấu trường ghi độ sâu 24 bit ở 1024×1024. Tường, thùng, cột, bia và các bộ phận robot dùng cùng ma trận Model trong hai lượt vẽ. Sàn nhận bóng. Thanh máu, súng góc nhìn và HUD không tham gia tạo bóng. Chỉ thành phần của mặt trời được nhân với độ sáng lấy từ shadow map, nên đèn pin vẫn chiếu sáng vùng khuất nắng.
- Mỗi va chạm có pháp tuyến tạo một cụm 18 hạt. CPU lưu điểm trúng, pháp tuyến và tuổi cụm theo đồng hồ mô phỏng. GPU sinh hướng từ chỉ số instance, tính vị trí theo vận tốc ban đầu và gia tốc hướng xuống. Giữ tối đa tám cụm, giải phóng sau 0,42 giây và xóa hết khi chơi lại. F7 chỉ điều khiển việc vẽ để có thể hiện lại cụm đang tạm dừng.

## Milestone 9

- Mỗi lệnh vẽ chữ gửi tối đa 16 mã ASCII bằng một mảng uniform trong vertex shader GL 3.3. Tất cả instance dùng chung quad và atlas đã có. Chuỗi dài hơn được chia thành nhiều nhóm. Tọa độ đầu nhóm được tính theo vị trí ký tự trong chuỗi để giữ đúng khoảng cách, kể cả dấu cách. Giá trị `kHudGlyphsPerBatch` trong C++ phải khớp độ dài `uGlyphs` trong GLSL.
- Đánh giá thay đổi HUD bằng cả số lệnh vẽ và ảnh đọc lại từ framebuffer. Trong cảnh đo, số lệnh giảm từ 56 xuống 20. Thời gian vẽ toàn cảnh còn phụ thuộc GPU và tải máy. Với cả hai hiệu ứng bật, trung vị của một lần đối chiếu là 2,409 ms trước và 2,525 ms sau. Lợi ích đã xác nhận là giảm số lệnh gửi từ CPU, còn phép đo này chưa chứng minh FPS tăng.
- Đích `render_benchmark` dùng chung thư viện tĩnh nội bộ `shooter_render` với game, giúp dựng lại đúng cảnh để so F6/F7. Khung đo ở wave 7 giữ nguyên tám robot, tuổi hạt và nội dung HUD. Bản chơi vẫn dùng bước mô phỏng cố định và VSync. Công cụ đo chỉ được build khi gọi đích danh.

## Các vấn đề đã xử lý

| Hiện tượng | Cách xử lý |
| --- | --- |
| Ảnh kiểm tra shadow map xuất hiện chấm tối lặp lại trên bia và mặt nghiêng của thùng, kể cả vùng có nắng. | Khi PCF so với texel lân cận, độ sâu ngay trên cùng một tam giác có thể thay đổi. Dùng `glPolygonOffset` trong lượt độ sâu để tăng độ bù theo độ dốc của tam giác, rồi tắt trước lượt màu. |
| CMake 4 cảnh báo GLM 1.0.1 yêu cầu chính sách tương thích quá cũ. | Khi cấu hình GLM, đặt mức chính sách tối thiểu 3.10 nếu người build chưa đặt. Đã cấu hình lại và build sạch. |
| Công cụ kiểm thử Windows đọc vùng vẽ là 1024×576 khi màn hình đặt tỉ lệ 125%. | Cho tiến trình đo nhận biết DPI rồi đọc lại kích thước vùng vẽ theo pixel vật lý. Kết quả là 1280×720. |
| Ảnh chụp desktop khi kiểm thử cube chứa màu của cửa sổ khác. | Lấy mẫu pixel trực tiếp từ vùng client của cửa sổ game và so sánh hai thời điểm để kiểm tra chuyển động. |
| So sánh tuyệt đối hai ảnh bầu trời sau khi dịch camera cho sai khác rất nhỏ. | Đọc lại framebuffer 8 bit và đo sai lệch từng kênh. Sai lệch lớn nhất là 1 mức màu ở phép thử này, phù hợp với độ chính xác số thực của ma trận nhìn và quá trình lượng tử hóa. |
| Phép thử cửa sổ trên Debug đôi lúc lấy mẫu trước khi nhân vật kịp đứng yên sát thùng. | Cho lộ trình nhảy lên thùng chờ hai mẫu ảnh ổn định khi đang giữ W. Chạy lộ trình kiểm tra di chuyển cơ bản riêng từ điểm xuất phát. Các tiếp xúc và độ cao chính xác được kiểm tra trực tiếp bằng CTest. |
| Khi trừ nhiều bước 1/60 khỏi thời gian chờ, phần dư rất nhỏ quanh 0 có thể làm trễ một phát thêm một bước. | Quy phần dư trong dung sai số thực về 0. Kiểm thử đếm đúng 10 phát ở các bước 0, 9, …, 81 khi giữ cò trong 90 bước. |
