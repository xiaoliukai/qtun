<?php
try {
    $b = defined(__DIR__);
    define('ROOT', __DIR__.'/../');
} catch (Exception $e) {
    define('ROOT', dirname(__FILE__).'/../');
}