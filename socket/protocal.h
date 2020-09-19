#ifndef REAL_FRAMEWORK_PROTOCAL_H_
#define REAL_FRAMEWORK_PROTOCAL_H_

#include "util.h"

const auto protocal_test = "test";
const auto protocal_connect = "connect";
const auto protocal = rea::Json(protocal_test,
                                rea::Json("req", rea::Json(
                                                     "type", protocal_test,
                                                     "key", "hello"
                                                     ),
                                          "res", rea::Json(
                                                     "type", protocal_test,
                                                     "key", "world"
                                                     )),
                                protocal_connect,
                                rea::Json("req", rea::Json(
                                                     "type", protocal_connect,
                                                     "token", rea::GetMachineFingerPrint(),
                                                     "s3_ip_port", "",
                                                     "s3_access_key", "",
                                                     "s3_secret_key", ""
                                                     ),
                                          "res", rea::Json(
                                                     "type", protocal_connect,
                                                     "err_code", 0,
                                                     "msg", "...")));

#endif
