# 云映12306客户端
由Qt构建的 12306 GUI购票工具  
支持即时下单、候补、定时抢票、捡漏、学生票、车次/席别优先规则、选座、打开支付页面，通知方式支持邮件通知、Server酱通知  
Qt版本要求  
``支持Qt 6.0.0之后的版本``  
操作系统要求  
``支持Windows/MacOS/Linux``  
请安装对应操作系统版本的[Qt](https://www.qt.io/download)/QtCreator，导入yunying.pro文件  
由于Network库不支持CDN，所以在编译之前请先注释`yunying.pro`文件中的宏`HAS_CDN`  
``# DEFINES += HAS_CDN``  
否则编译会出错  
另外，[https://www.op9.top](https://www.op9.top)提供了预编译的二进制包下载、预览及帮助，请前往了解更多信息
## 小提示  
1. 过短的查询间隔导致ip容易被封
2. 抢票的同时可以候补，不冲突
3. 遇到区间限售时建议优先候补
4. 候补订单生效时间是以订单支付时间计算的，不是订单下单时间，第一时间支付很重要
