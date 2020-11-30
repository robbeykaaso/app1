#ifndef REAL_FRAMEWORK_PROTOCAL_H_
#define REAL_FRAMEWORK_PROTOCAL_H_

#include "util.h"

const auto protocal_test = "test";
const auto protocal_connect = "connect";
const auto protocal_training = "training";
const auto protocal_inference = "inference";
const auto protocal_task_state = "task_state";
const auto protocal_upload = "upload";
const auto protocal_progress_push = "task_progress_push";
const auto protocal_log_push = "task_log";
const auto protocal_stop_job = "stop_job";
const auto protocal_delete_job = "delete";
const auto protocal = rea::Json(protocal_test,
                                rea::Json("req", rea::Json(
                                                     "type", protocal_test,
                                                     "key", "hello"
                                                     ),
                                          "res", rea::Json(
                                                     "type", protocal_test,
                                                     "key", "world"
                                                     )),
                                //for deepsight
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
                                                     "msg", "...")),
                                protocal_training,
                                rea::Json("req", rea::Json(
                                                     "type", protocal_training,
                                                     "token", rea::GetMachineFingerPrint()
                                                         ),
                                          "res", rea::Json(
                                                     "type", protocal_training,
                                                     "err_code", 0,
                                                     "msg", "..."
                                                     )),
                                protocal_inference,
                                rea::Json("req", rea::Json(
                                                     "type", protocal_inference,
                                                     "token", rea::GetMachineFingerPrint()
                                                                                                                  ),
                                          "res", rea::Json(
                                                     "type", protocal_inference,
                                                     "err_code", 0,
                                                     "msg", "..."
                                                                                                              )),
                                protocal_task_state,
                                rea::Json("req", rea::Json(
                                                     "type", protocal_task_state,
                                                     "token", rea::GetMachineFingerPrint()
                                                         ),
                                          "res", rea::Json(
                                                     "type", protocal_task_state,
                                                     "state", "running",
                                                     "err_code", 0)),
                                protocal_upload,
                                rea::Json("req", rea::Json(
                                                     "type", protocal_upload,
                                                     "token", rea::GetMachineFingerPrint(),
                                                     "data_type", "prediction"
                                                     ),
                                          "res", rea::Json(
                                                     "type", protocal_upload,
                                                     "err_code", 0,
                                                     "msg", "..."
                                                     )),
                                protocal_progress_push,
                                rea::Json("res", rea::Json(
                                                     "type", protocal_progress_push
                                                     )),
                                protocal_log_push,
                                rea::Json("res", rea::Json(
                                                     "type", protocal_log_push
                                                     )),
                                protocal_stop_job,
                                rea::Json("req", rea::Json(
                                                     "type", protocal_stop_job,
                                                     "token", rea::GetMachineFingerPrint(),
                                                     "soft_stop", true
                                                     ),
                                          "res", rea::Json(
                                                     "type", protocal_stop_job,
                                                     "err_code", 0,
                                                     "msg", "..."
                                                     )),
                                protocal_delete_job,
                                rea::Json("req", rea::Json(
                                                     "type", protocal_delete_job,
                                                     "token", rea::GetMachineFingerPrint()
                                                         ),
                                          "res", rea::Json(
                                                     "type", protocal_delete_job,
                                                     "err_code", 0,
                                                     "msg", "..."
                                                     ))
                                    );

#endif
