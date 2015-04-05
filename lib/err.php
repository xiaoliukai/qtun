<?php
function err_handler($errno, $errstr, $errfile, $errline, $errcontext) {
    $ret = array(
        'errno'   => $errno,
        'errstr'  => $errstr,
        'errfile' => $errfile,
        'errline' => $errline
    );
    echo json_encode(res(500, $ret));
    exit;
}

function exp_handler($e) {
    echo json_encode(res(500, var_export($e, true)));
    exit;
}

function shutdown_handler() {
    $err = error_get_last();
    if ($err['type'] == E_ERROR) exp_handler($err);
}

register_shutdown_function('shutdown_handler');
set_error_handler('err_handler');
set_exception_handler('exp_handler');
ini_set('display_errors', 'off');
error_reporting(E_ALL);