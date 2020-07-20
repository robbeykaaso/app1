#ifndef REAL_FRAMEWORK_PROTOCAL_H_
#define REAL_FRAMEWORK_PROTOCAL_H_

#include "util.h"

const auto protocal_test = "test";
const auto protocal = rea::Json(protocal_test,
                                rea::Json("req", rea::Json(
                                                     "key", "hello"
                                                     ),
                                          "res", rea::Json(
                                                     "key", "world"
                                                     )));

#endif
