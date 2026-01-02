## 连接wifi

```
AT+CWMODE=1           // 设为 STA 模式（1=STA，2=AP，3=AP+STA）
AT+CWJAP="ZTE-45c476","88888888"     // 连接 Wi-Fi（仅本次有效）
AT+CIFSR             // 查询本机 IP
```

![image-20251031233734990](https://cloud.rocketpi.club/cloud/image-20251031233734990.png)

## 连接mqtt

![image-20251031233833138](https://cloud.rocketpi.club/cloud/image-20251031233833138.png)

配置并连接 MQTT（明文 TCP 1883，公共测试允许匿名）：

```
AT+MQTTUSERCFG=0,1,"esp8266-emqx","","",0,0,""    // link=0, scheme=1=TCP, clientID=esp8266-emqx
AT+MQTTCONN=0,"broker.emqx.io",1883,0              // 连接：host/port，0=不自动重连（不同固件略有出入）
```

订阅与发布：

```
AT+MQTTSUB=0,"/test/esp8266",1
AT+MQTTPUB=0,"/test/esp8266","hello from esp8266",1,0
```

断开（可选）：

```
AT+MQTTCLEAN=0
```



![image-20251031234718267](https://cloud.rocketpi.club/cloud/image-20251031234718267.png)

