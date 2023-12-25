Gồm 2 file:
    *app.exe: là chương trình để làm việc với file dữ liệu.
    *data.bin: là file dữ liệu

Tính năng:
    - Có khả năng thêm, xóa, sửa dữ liệu (khi xóa thì không xóa hẳn, vẫn có khả năng khôi phục)
    - Trường sđt và cccd sẽ bị mã hóa bằng aes256, khi người dùng sử dụng thì cần nhập pass để giải mã lại thành bản rõ

Cấu trúc mỗi phần tử học sinh/giáo viên như sau:
-1 byte đầu tiên: dùng để phân biệt giáo viên và học sinh, 1 là giáo viên (-1 khi bị xóa), 2 là học sinh (-2 khi bị xóa)
-4 bytes tiếp theo: lưu vị trí của phần tử tiếp theo trong mảng
-4 bytes tiếp theo: lưu vị trí của phần tử trước đó trong mảng
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

Phần đầu của file là 32 bytes giá trị đã hash của mật khẩu
Sau đó là 2 metadata của 2 mảng. Mỗi metadata là 12 bytes:
    +4 bytes đầu lưu vị trí phần tử đầu tiên trong mảng
    +4 bytes tiếp theo lưu vị trí phần tử cuối cùng trong mảng
    +4 phần tử tiếp theo lưu số lượng phần tử trong mảng