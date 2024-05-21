# ![enter image description here](images/ticket.ico) 云映12306客户端

*由Qt构建的 12306 GUI购票工具*

![](https://img.shields.io/badge/GPL-v3-blue.svg)
![](https://img.shields.io/badge/Qt-6.0.0-blue.svg)
![](https://img.shields.io/badge/C++-11-deeppink.svg)
![](https://img.shields.io/badge/12306-App-red.svg)

![](https://img.shields.io/badge/-Windows-blue?logo=windows)
![](https://img.shields.io/badge/-MacOS-black?logo=apple)
![](https://img.shields.io/badge/-Linux-333?logo=ubuntu)

### 支持功能
- **即时下单**
- **候补**
- **定时抢票**
- **捡漏**
- **学生票**
- **车次/席别优先规则**
- **选座**
- **自动打开支付页面**
- **持邮件通知**
- **Server酱通知**

### 测试平台
![](https://img.shields.io/badge/Windows-10-pass.svg) ![](https://img.shields.io/badge/MACOS-12.5.3-pass.svg) ![](https://img.shields.io/badge/Ubuntu-20.04-pass.svg)

### 应用下载地址 - https://www.op9.top

### 软件截图
![](https://www.op9.top/img/running_snapshot.png)

**Qt版本要求**

``支持Qt 6.0.0之后的版本``

**操作系统要求**

``支持Windows/MacOS/Linux``

### 使用方法
#### 1. 安装Qt
搜索引擎搜索`Qt 安装`即可找到安装方法，以下链接为一个安装教程（记得在安装的时候勾选QtCreator）

[Qt6安装教程(使用国内源)](https://zhuanlan.zhihu.com/p/683591671)

#### 2. 注释HAS_CDN
由于Network库不支持CDN，所以在编译之前请先注释`yunying.pro`文件中的宏`HAS_CDN`

``# DEFINES += HAS_CDN``

否则编译会出错

#### 3. 导入项目
在QtCreator中`文件->打开文件或项目`打开`yunying.pro`即可

#### 3. 构建并运行
点击左下角的绿色三角形即自动构建并运行

另外，[https://www.op9.top](https://www.op9.top)提供了Windows的二进制包下载，无须下载Qt即可运行

## 声明
### 本软件仅用于学习交流，禁止用于任何商业行为，包括但不限于车票代购、倒卖、囤票、加价等行为，违者自行承担相关责任

## 许可证
### [GPL V3](LICENSE)
如果您自行发布了本软件、发布了本软件的修改版本或您的软件中包含了本软件的组件，请确保您的许可证是与`GPL V3`是相兼容的

#### [关于抢票/候补的一点思考](LittleSummary.md)

