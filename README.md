# SmartHome
A course project: IoT for smart home, with STM32 nodes, a PC daemon, and HTML pages for control.

这是清华“制造工程体验”的智能家居小组项目。

STM32核心板负责读取传感器或驱动执行器，通过WiFi与MQTT服务器交换数据。

Daemon是服务器上的常驻程序，作为一个中间件，维护传感器的列表和状态，为上层应用提供简单的操作接口。

在此之上，用jQuery编写的网页可以在手机上显示，读取和控制各节点。可以设计自动控制规则。可以通过某些语音命令来更简单地操作。

我们在房屋模型上布置了核心板及相关电路，包括温湿度、热释电、读卡器、土壤湿度、光敏等传感器，以及摄像头；
控制舵机、继电器（灯、加湿器）等电器的工作。
