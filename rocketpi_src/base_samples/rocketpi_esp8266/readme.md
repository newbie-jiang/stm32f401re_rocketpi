## 效果展示

![esp01s](https://cloud.rocketpi.club/cloud/esp01s.gif)

## 功能说明

- 对基本的wifi指令测试
- 测试前请参考目录中的esp01s固件烧录文档，先给esp01s烧录支持mqtt的固件（rocketpi_esp8266 在此demo下）

测试前请在工程中更改wifi配置  driver_esp8266_at_test.c  （注意只支持家中2.4g频段的WIFI）

![image-20251223045328170](https://cloud.rocketpi.club/cloud/image-20251223045328170.png)

测试流程如下

- 串口与 AT 基础连通性
- 固件/版本信息读取
- WIFI STA 模式配置（IP信息获取，网关以及掩码）
- WIFI AP 模式配置验证（IP信息获取，网关以及掩码）
- MQTT配置订阅
- MQTT配置发布
- MQTT断开
- WIFI STA断开
- 结束

```shell
=== ESP8266 AT smoke test ===
[ESP8266][init] OK
[ESP8266][echo_off] OK
[ESP8266][event][ok] OK (cmd=ATE0)
[ESP8266][AT] OK
[ESP8266][event][ok] OK (cmd=AT)
[ESP8266][AT+GMR] OK
[ESP8266][event][info] AT version:2.3.0.0-dev(s-bcd64d2 - ESP8266 - Jun 23 2021 11:42:05) (cmd=AT)
[ESP8266][event][info] SDK version:v3.4-22-g967752e2 (cmd=AT)
[ESP8266][event][info] compile time(b498b58):Jun 30 2021 11:28:20 (cmd=AT)
[ESP8266][event][info] Bin version:2.2.0(ESP8266_1MB) (cmd=AT)
[ESP8266][event][ok] OK (cmd=AT+GMR)
[ESP8266][AT+CMD?] OK
[ESP8266][event][resp] +CQTTUSERCFG",0,0,1,0
        arg0(q): +CQTTUSERCFG,0,0,1,0
[ESP8266][event][ok] OK (cmd=AT+CMD)
[ESP8266][CWMODE] OK
[ESP8266][event][ok] OK (cmd=AT+CWMODE)
[ESP8266][CWJAP] OK
[ESP8266][event][ind] WIFI CONNECTED (cmd=AT)
[ESP8266][event][ind] WIFI GOT IP (cmd=AT)
[ESP8266][event][ok] OK (cmd=AT+CWJAP)
[ESP8266][CIPSTA?] OK
        STA ip=192.168.1.8 gateway=192.168.1.1 netmask=255.255.255.0
        AP ip=192.168.4.1 gateway=192.168.4.1 netmask=255.255.255.0
[ESP8266][event][ok] OK (cmd=AT+CIPSTA)
[ESP8266][event][ok] OK (cmd=AT+CIPAP)
[ESP8266][cmd] AT+MQTTUSERCFG=0,1,"esp8266-emqx","","",0,0,""
[ESP8266][MQTTUSERCFG] OK
[ESP8266][event][ok] OK (cmd=AT+MQTTUSERCFG)
[ESP8266][cmd] AT+MQTTCONN=0,"broker.emqx.io",1883,0
[ESP8266][MQTTCONN] OK
[ESP8266][event][resp] +MQTTCONNECTED:0,1,"broker.emqx.io","1883","",0
        arg0: 0
        arg1: 1
        arg2(q): broker.emqx.io
        arg3(q): 1883
        arg4(q):
        arg5: 0
[ESP8266][event][ok] OK (cmd=AT+MQTTCONN)
[ESP8266][MQTTSUB] OK
[ESP8266][event][ok] OK (cmd=AT+MQTTSUB)
[ESP8266][MQTTPUB] OK
[ESP8266][event][resp] +MQTTSUBRECV:0,"/test/esp8266",19,hello from rocketpi
        arg0: 0
        arg1(q): /test/esp8266
        arg2: 19
        arg3: hello from rocketpi
[ESP8266][event][ok] OK (cmd=AT+MQTTPUB)
[ESP8266][MQTTUNSUB] OK
[ESP8266][event][ok] OK (cmd=AT+MQTTUNSUB)
[ESP8266][MQTTCLEAN] OK
[ESP8266][event][ok] OK (cmd=AT+MQTTCLEAN)
[ESP8266][CWQAP] OK
[ESP8266][event][ind] WIFI DISCONNECT (cmd=AT)
[ESP8266][event][ok] OK (cmd=AT+CWQAP)
=== ESP8266 AT smoke test done ===

```

MQTT订阅主题  /test/esp8266 可收到测试消息

![image-20251223045045679](https://cloud.rocketpi.club/cloud/image-20251223045045679.png)

## 硬件连接

![21d9ed63e5a5b7fe8a2262601f20eb92](https://cloud.rocketpi.club/cloud/21d9ed63e5a5b7fe8a2262601f20eb92.jpg)
