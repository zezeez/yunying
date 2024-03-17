#include "loginconf.h"

LoginConf::LoginConf()
{

}

LoginConf &LoginConf::instance()
{
    static LoginConf lconf;
    return lconf;
}
