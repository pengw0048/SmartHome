####调试步骤

1. 打开工程文件Project/MDK/IoT.uvproj
2. 修改func.h中WiFi的名称和密码
3. 在board.h中选取需要的外设类型
4. 编译工程并下载进单片机
5. 单片机断电并重新上电，靠外侧的LED点亮说明联网成功
6. 通过API Demo网页测试功能