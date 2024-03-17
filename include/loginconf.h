#ifndef LOGINCONF_H
#define LOGINCONF_H


class LoginConf
{
public:
    LoginConf();
    static LoginConf &instance();

    bool isUamLogin;  // 是否统一认证登录
    bool isLoginPasscode;  // 是否启用验证码校验登录（仅本地登录）
    bool isSweepLogin;  // 统一认证登录情况下是否开启扫码登录
    bool isLogin;  // 是否已登录
    bool isMessagePasscode;  //是否显示短信验证

};

#endif // LOGINCONF_H
