SET NAMES utf8;
SET time_zone = '+07:00';

CREATE DATABASE `iot` /*!40100 DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci */;
USE `iot`;

CREATE TABLE `device` (
  `device_id` varchar(18) COLLATE utf8_unicode_ci NOT NULL,
  `username` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `pass` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


CREATE TABLE `status` (
  `device_id` varchar(18) COLLATE utf8_unicode_ci NOT NULL,
  `label` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `update_time` datetime NOT NULL ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

