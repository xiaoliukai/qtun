<?php
require dirname(__FILE__).'/../framework/base.php';
require ROOT.'/lib/base.php';

$db = conn();
$db->exec("CREATE TABLE IF NOT EXISTS `news` (
          `id` int(11) NOT NULL AUTO_INCREMENT,
          `updated` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'publish time',
          `title` mediumtext NOT NULL COMMENT 'title for news',
          `content` longtext NOT NULL,
          PRIMARY KEY (`id`)
         ) ENGINE=MyISAM DEFAULT CHARSET=utf8");
$db->exec("CREATE TABLE IF NOT EXISTS `users` (
          `id` int(11) NOT NULL AUTO_INCREMENT,
          `name` varchar(255) NOT NULL COMMENT 'user name',
          `email` mediumtext COMMENT 'email',
          `level` tinyint(4) NOT NULL DEFAULT '0' COMMENT '0=normal user;1=admin;',
          `password` varchar(40) NOT NULL COMMENT 'password with sha1(md5(pass + salt1)+salt2)',
          `salt1` varchar(8) NOT NULL COMMENT 'salt for md5',
          `salt2` varchar(8) NOT NULL COMMENT 'salt for sha1',
          `register` int(11) NOT NULL COMMENT 'register time',
          `login` int(11) NOT NULL COMMENT 'last login time',
          PRIMARY KEY (`id`)
         ) ENGINE=MyISAM DEFAULT CHARSET=utf8");