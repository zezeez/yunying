#ifndef H12306_H
#define H12306_H

#define PASSPORT_APPID "otn"
#define PASSPORT_BASEURL "https://kyfw.12306.cn/passport/"
#define BASEURL "https://kyfw.12306.cn"
#define PASSPORT_APPTK_STATIC PASSPORT_BASEURL "web/auth/uamtk-static"
#define PASSPORT_LOGIN PASSPORT_BASEURL "web/login"
#define PASSPORT_UAMTK PASSPORT_BASEURL "web/auth/uamtk"
#define UAMAUTHCLIENT BASEURL PUBLICNAME "/uamauthclient"
#define PUBLICNAME "/otn"
#define SENDMOBILEMSG BASEURL PUBLICNAME"/login/sendMobileMsg"
#define SLIDE_PASSPORT_URL PASSPORT_BASEURL "web/slide-passcode"
#define CHECK_LOGIN_PASSPORT_URL PASSPORT_BASEURL "web/checkLoginVerify"
#define SMS_VERIFICATION PASSPORT_BASEURL "web/getMessageCode"
#define SM4_KEY_SECRET "tiekeyuankp12306"
#define APPKEY "FFFF0N000000000085DE"

#define LOGINCONFURL BASEURL PUBLICNAME "/login/conf"
//  获取JSSIONID、route、BipServerOtn cookie
#define INITLOGINCOOKIE BASEURL PUBLICNAME "/resources/login.html"
//  获取REPEAT_SUBMIT_TOKEN/leftTicket串
#define INITDC BASEURL PUBLICNAME "/confirmPassenger/initDc"
//  获取车站名称/拼音信息
#define STATIONNAMEURL BASEURL PUBLICNAME "/resources/js/framework/station_name.js?station_version="
#define STATIONNAMEVERSION "1.9299"

#define QUERYTICKETBASEURL BASEURL PUBLICNAME "/leftTicket/queryE"
#define USERAGENT "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.107 Safari/537.36"

#define QEURYTRAINSTOPSTATION BASEURL PUBLICNAME "/czxx/queryByTrainNo"

// 本地登录
#define GETPASSCODENEW BASEURL PUBLICNAME "/passcodeNew/getPassCodeNew?module=login&rand=sjrand&"
#define CHECKRANDCODEANSYN BASEURL PUBLICNAME "/passcodeNew/checkRandCodeAnsyn"
#define LOGINAYSNSUGGEST BASEURL PUBLICNAME "/login/loginAysnSuggest"
#define GETBANNERS BASEURL PUBLICNAME "/index12306/getLoginBanner"
#define LOGINSLIDE BASEURL PUBLICNAME "/slide/loginSlide"

#define USERLOGIN  BASEURL PUBLICNAME "/login/userLogin"
#define INITMY12306API  BASEURL PUBLICNAME "/index/initMy12306Api"

// 扫码登录
#define QR BASEURL "/passport/web/create-qr"
#define QR64 BASEURL "/passport/web/create-qr64"
#define CHECKQR BASEURL "/passport/web/checkqr"

#define LOGOUT BASEURL PUBLICNAME "/login/loginOut"

// 订单
// 检查用户是否登陆，下单前需要检查
#define LOGINCHECKUSER  BASEURL PUBLICNAME "/login/checkUser"
// 获取购票人信息 (乘车人信息)
#define GETPASSENGERINFODTOS BASEURL PUBLICNAME "/confirmPassenger/getPassengerDTOs"
// 提交下单请求
#define SUBMITORDERREQUEST BASEURL PUBLICNAME "/leftTicket/submitOrderRequest"
// 检查订单信息 (是否能购票，是否有未支付的订单)
#define CHECKORDERINFO BASEURL PUBLICNAME "/confirmPassenger/checkOrderInfo"
// 获取排队人数、余票信息
#define GETQUEUECOUNT BASEURL PUBLICNAME "/confirmPassenger/getQueueCount"
// sync
#define CONFIRMSINGLE BASEURL PUBLICNAME "/confirmPassenger/confirmSingle"
// async
#define CONFIRMSINGLEFORQUEUE BASEURL PUBLICNAME "/confirmPassenger/confirmSingleForQueue"
#define QUERYORDERWAITTIME BASEURL PUBLICNAME "/confirmPassenger/queryOrderWaitTime"
#define RESULTORDERFORDCQUEUE BASEURL PUBLICNAME "/confirmPassenger/resultOrderForDcQueue"

// 候补
#define CHECHFACE BASEURL PUBLICNAME "/afterNate/chechFace"
#define SUBMITCANDIDATEORDERQUEST BASEURL PUBLICNAME "/afterNate/submitOrderRequest"
#define PASSENGERINITAPI BASEURL PUBLICNAME "/afterNate/passengerInitApi"
#define CANDIDATEGETQUEUECOUNT BASEURL PUBLICNAME "/afterNate/getQueueNum"
#define CONFIRMHB BASEURL PUBLICNAME "/afterNate/confirmHB"
#define CANDIDATEQUERYQUEUE BASEURL PUBLICNAME "/afterNate/queryQueue"
#define LINEUPTOPAYCONFIRM BASEURL PUBLICNAME "/view/lineUp_payConfirm.html"

// 票类别
#define TICKETADULT "1"  // 成人
#define TICKETCHILD "2"  // 儿童
#define TICKETSTUDENT "3"  // 学生
#define TICKETDISABILITY "4"  // 残军

#define TICKETADULTNAME "成人票"
#define TICKETCHILDNAME "孩票"
#define TICKETSTUDENTNAME "学生票"
#define TICKETDISABILITYNAME "伤残军人票"

// 乘客类别
#define PASSENGERADULT "1"  // 成人
#define PASSENGERCHILD "2"  // 儿童
#define PASSENGERSTUDENT "3"  // 学生
#define PASSENGERDISABILITY "4"  // 残军

// 证件类别
#define CARDTWO "1"  // 二代身份证
#define CARDONE "2"  // 一代身份证
#define CARDTMP "3"  // 临时身份证
#define CARDPASSPORD "B"  // 护照
#define CARDWORK "H"  // 工作居留证
#define CARDHONGKONGMACAU "C"  // 港澳身份证
#define CARDTAIWAN "G" // 台湾身份证

#define TICKETQUERYCOMMON "00"
#define TICKETQUERYSTUDENT "0X00"

// 座位类别
#define SEATYIDENG 'M'  // 一等座
#define SEATERDENG 'O'  // 二等座
#define SEATTEDENG 'P'  // 特等座
#define SEATSHANGWU '9'  // 商务座
#define SEATYINGZUO '1'  // 硬座
#define SEATRRUANZUO '2' // 软座
#define SEATYINGWUO '3'  // 硬卧
#define SEATRUANWUO '4'  // 软卧
#define SEATGAOJIDONGWUO 'A'  // 高级动卧
#define SEATDONGWUO 'F'  // 动卧
#define SEATYIDENGWUO 'I'  // 一等卧
#define SEATERDEWUO 'J'  // 二等卧
#define SEATWUZUO SEATERDENG
#define SEATWUZUO1 'W'  // 无座
#define SEATWUZUO2 "WZ"  // 无座
#define SEATQITA 'H' // 其他

#endif // H12306_H
