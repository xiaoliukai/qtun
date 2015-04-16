<?php
require dirname(__FILE__).'/../lib/base.php';

$db = conn();
$db->exec("CREATE TABLE IF NOT EXISTS `news` (
            `id` int(11) NOT NULL AUTO_INCREMENT,
            `updated` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'publish time',
            `title` mediumtext NOT NULL COMMENT 'title for news',
            `content` longtext NOT NULL,
            `by` int(11) NOT NULL COMMENT 'commit by',
            PRIMARY KEY (`id`)
          ) ENGINE=MyISAM DEFAULT CHARSET=utf8");
$db->exec("CREATE TABLE IF NOT EXISTS `users` (
            `id` int(11) NOT NULL AUTO_INCREMENT,
            `name` varchar(255) NOT NULL COMMENT 'user name',
            `email` mediumtext COMMENT 'email',
            `recvmsg` bit(1) NOT NULL DEFAULT b'1',
            `level` tinyint(4) NOT NULL DEFAULT '0' COMMENT '0=normal user;1=admin;',
            `password` varchar(40) NOT NULL COMMENT 'password with sha1(md5(pass)+salt)',
            `salt` varchar(8) NOT NULL COMMENT 'salt for sha1',
            `register` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'register time',
            `login` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT 'last login time',
            PRIMARY KEY (`id`)
          ) ENGINE=MyISAM DEFAULT CHARSET=utf8");