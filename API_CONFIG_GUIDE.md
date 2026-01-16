# API密钥配置指南

## 配置方法

### 1. 获取API密钥

访问 [SiliconFlow](https://cloud.siliconflow.cn/) 注册并获取API密钥。

### 2. 配置客户端

客户端程序首次运行时会在可执行文件目录下自动创建配置文件：

```
Client/build/Desktop_Qt_6_10_1_MinGW_64_bit-Debug/debug/config/api.conf
```

### 3. 编辑配置文件

打开 `config/api.conf` 文件，填入你的API密钥：

```
# AI-ChatRoom API Configuration
# Get your API key from: https://cloud.siliconflow.cn/
# This file is ignored by git and won't be uploaded to GitHub

SILICONFLOW_API_KEY=sk-your-actual-api-key-here
```

### 4. 重启客户端

保存配置文件后，重启AI-ChatRoom客户端即可使用AI功能。

## 安全说明

- ✅ `config/` 文件夹已添加到 `.gitignore`，不会被上传到GitHub
- ✅ 配置文件保存在本地，每个用户使用自己的密钥
- ✅ 开发者可以安全地分享代码，不用担心泄露密钥

## 故障排除

如果AI功能无法使用，请检查：

1. 配置文件路径是否正确
2. API密钥是否正确填写（不要有多余的空格）
3. 网络连接是否正常
4. API密钥是否有效（未过期/未超额）
