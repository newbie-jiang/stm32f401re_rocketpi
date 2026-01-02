## 效果展示

![mbedtls](https://cloud.rocketpi.club/cloud/mbedtls.gif)

集成 mbed TLS 并在 `Core/Src/main.c` 中加入以下三个示例用例，用于验证加密库可用性：

- **AES-128 ECB**：使用 NIST 标准向量执行一次加密和解密，串口打印密文并给出 PASS/FAIL。
- **RSA-1024 PKCS#1 v1.5**：导入 mbed TLS 官方自测用的 1024 位密钥对，完成加密和解密，串口输出解密得到的明文。
- **SHA-256**：对固定字符串 `RocketPi MBEDTLS` 求哈希，串口打印 32 字节摘要并校验是否吻合。

打开 `USART2` 串口（115200 8-N-1），复位 MCU 后即可看到如下输出：

```text
==== mbed TLS sample tests ====
AES cipher: 3AD77BB40D7A3660A89ECAF32466EF97
AES-128 ECB: PASS
RSA plaintext: RocketPi RSA demo
RSA-1024 PKCS#1 v1.5: PASS
SHA-256: B92F7CAEA14F3B71FEBB57987565177DB103116193D79E851E176B139188B363
SHA-256 digest: PASS
==== tests finished ====
```
