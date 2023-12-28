Gồm 2 file:
    *app.exe: là chương trình để làm việc với file dữ liệu.
    *data.bin: là file dữ liệu

Tính năng:
    - Có khả năng thêm, xóa, sửa dữ liệu. Khi xóa file thì file không mất hẳn, vẫn có thể khôi phục được
    - Trường sđt và cccd sẽ bị mã hóa bằng aes256, các trường còn lại không được mã hóa.

Cấu trúc mỗi phần tử học sinh/giáo viên như sau:
-4 bytes tiếp theo: lưu vị trí của phần tử tiếp theo trong mảng
-4 bytes tiếp theo: lưu vị trí của phần tử trước đó trong mảng
-1 byte đầu tiên: dùng để phân biệt giáo viên và học sinh, 1 là giáo viên (-1 khi bị xóa), 2 là học sinh (-2 khi bị xóa)
-8 bytes tiếp theo: là mã sinh viên/giáo viên (8 kí tự ascii)
-32 bytes tiếp theo: là họ tên, để đơn giản thì tên chỉ lưu kiểu ascii (mỗi kí tự là 1 byte)
-4 bytes tiếp theo: lưu ngày sinh
    +1 byte lưu ngày
    +1 byte lưu tháng
    +2 bytes lưu năm
-4 bytes tiếp theo: lưu ngày tham gia, cấu trúc tương tự như trên
-Số điện thoại: 16 bytes đã mã hóa bằng aes (bản rõ là chuỗi ascii không quá 15 bytes)
-cccd: 16 bytes đã mã hóa bằng aes (bản rõ là chuỗi ascii không quá 15 bytes)
=> Độ dài mỗi phần tử là 80 bytes

Phần đầu của file là 32 bytes giá trị đã hash của key
Sau đó là 2 metadata của 2 mảng. Mỗi metadata là 12 bytes:
    +4 bytes đầu lưu vị trí phần tử đầu tiên trong mảng
    +4 bytes tiếp theo lưu vị trí phần tử cuối cùng trong mảng
    +4 phần tử tiếp theo lưu số lượng phần tử trong mảng

Mô tả kịch bản:
Khi người dùng chạy chương trình, nếu chưa có database tên là data.bin thì chương trình sẽ tạo file database và yêu cầu người dùng đặt mật khẩu.
Mật khẩu sẽ được hash để tạo ra key, tiếp tục hash giá trị key để tạo ra HashKey, lưu hashKey và 2 metadata vào file database (tổng là 56 bytes như mô tả ở trên).
Khi đã có database, chương trình sẽ yêu cầu người dùng nhập mật khẩu, sau đó sẽ tính ra key, để giúp người dùng kiểm tra mật khẩu đúng hay không, chương trình sẽ
tính lại HashKey và so sánh với HashKey được lưu trong database rồi thông báo cho người dùng, nếu sai thì yêu cầu nhập lại.
Nếu nhập đúng mật khẩu thì key sẽ được chương trình sử dụng để mã hóa / giải mã với thuật toán aes.
sẽ có 9 lựa chọn cho người dùng:
    - in danh sách học sinh
    - in danh sách giáo viên
    - thêm học sinh
    - thêm giáo viên
    - xóa học sinh
    - xóa giáo viên
    - sửa học sinh
    - sửa giáo viên
    - tìm kiếm các thông tin đã xóa

Mô tả cấu trúc tệp tin:
Các mảng là các linked list 2 chiều. Từ 12 bytes metadata ta lấy được địa chỉ của 2 phần tử đầu và cuối của mảng (và số lượng phần tử nữa).
Thực ra chỉ cần 1 chiều là đã có thể sử dụng được, nhưng nếu 1 phần tử bị lỗi thì sẽ mất cấu trúc từ phần tử đó về sau.
Nên ta thực hiện linked list 2 chiều để chiều thứ 2 có thể làm backup.
khi xóa hay thêm 1 phần tử nào đó thì chỉ cần cập nhật lại giá trị con trỏ địa chỉ của 2 node cạnh nó.
(thứ tự trong mảng tuân theo thứ tự tăng dần của id)
Khi xóa thì giá trị của role sẽ chuyển sang giá trị âm, thời gian tồn tại của phần tử khi bị xóa không biết được,
nếu có phần tử được add vào mảng thì có thể nó sẽ bị ghi đè.
Khi thêm phần tử thì chương trình sẽ tìm khoảng trống trong database (là vị trí của các phần tử đã bị xóa), nếu không còn
khoảng trống nào thì nó sẽ ghi vào cuối file.