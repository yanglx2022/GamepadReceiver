[说明]
2.4G接收器[v0.2]
2.4G转USB Gamepad(游戏手柄)

STM32F103C8T6

电路：https://lceda.cn/yanglx/gamepad_receiver

与RCTransmitter 2.4G航模遥控器配对使用，连接电脑为USB Gamepad设备，使遥控器作为无线游戏手柄使用

支持U盘固件升级(按住按键插入USB口识别为U盘)

支持配对(连接电脑且USB识别后长按按键2s进入配对模式, 等待处于配对模式的遥控器发来配对地址完成配对)

支持跳频(每次新的连接前接收遥控器发来的随机跳频图案)

上电：
上电蓝色LED闪一下
无线模块初始化失败红色LED闪5s停机
USB连接超时红色LED闪10s停机

接收模式：
接收跳频图案成功后进入接收模式
接收连续失败超时(1S)进入接收跳频图案模式
此模式遥控器有动作时蓝色LED亮

接收跳频图案模式：
上电成功读取配对地址则进入接收跳频图案模式
此模式红蓝LED一起闪烁

配对模式：
上电读取配对地址失败则进入配对模式
其他模式下长按按键2s进入配对模式
此模式红蓝LED常亮










