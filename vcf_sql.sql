/*
 Navicat Premium Data Transfer

 Source Server         : Nuo
 Source Server Type    : MySQL
 Source Server Version : 50718
 Source Host           : cq-cdb-itpzoyvn.sql.tencentcdb.com:63864
 Source Schema         : hvh1

 Target Server Type    : MySQL
 Target Server Version : 50718
 File Encoding         : 65001

 Date: 30/08/2021 20:05:35
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for vcf_adminmenu
-- ----------------------------
DROP TABLE IF EXISTS `vcf_adminmenu`;
CREATE TABLE `vcf_adminmenu`  (
  `title` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `command` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `modid` tinyint(4) NOT NULL
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_admins
-- ----------------------------
DROP TABLE IF EXISTS `vcf_admins`;
CREATE TABLE `vcf_admins`  (
  `pid` int(11) NOT NULL,
  `modid` tinyint(4) NOT NULL,
  `nick` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `access` tinyint(4) NOT NULL,
  `flag` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  INDEX `FK_MODID_MOD`(`modid`) USING BTREE,
  INDEX `FK_PID_ADMIN`(`pid`) USING BTREE,
  CONSTRAINT `FK_MODID_MOD` FOREIGN KEY (`modid`) REFERENCES `vcf_mod` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT `FK_PID_ADMIN` FOREIGN KEY (`pid`) REFERENCES `vcf_users` (`pid`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_bans
-- ----------------------------
DROP TABLE IF EXISTS `vcf_bans`;
CREATE TABLE `vcf_bans`  (
  `id` int(32) NOT NULL AUTO_INCREMENT,
  `auth` char(17) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '0',
  `name` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `modid` int(11) NOT NULL,
  `serverid` int(11) NOT NULL,
  `adminauth` char(17) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '',
  `adminname` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `bantime` int(11) NOT NULL,
  `expiretime` int(11) NOT NULL,
  `bantype` tinyint(2) NOT NULL,
  `reason` tinytext CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 5 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_errorlog
-- ----------------------------
DROP TABLE IF EXISTS `vcf_errorlog`;
CREATE TABLE `vcf_errorlog`  (
  `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `sid` int(4) NOT NULL,
  `level` smallint(2) UNSIGNED NOT NULL DEFAULT 0,
  `module` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT 'Unknown',
  `message` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT 'Unknown Error',
  `timestamp` int(11) UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `log_server_id`(`sid`) USING BTREE,
  CONSTRAINT `log_server_id` FOREIGN KEY (`sid`) REFERENCES `vcf_servers` (`id`) ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_honor
-- ----------------------------
DROP TABLE IF EXISTS `vcf_honor`;
CREATE TABLE `vcf_honor`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `steamid` char(17) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '0',
  `type` enum('rulehonor','otherhonor','steamhonor') CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT 'rulehonor',
  `value` tinyint(4) NOT NULL DEFAULT 0,
  `time` int(11) NOT NULL DEFAULT 0,
  `reason` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_invite
-- ----------------------------
DROP TABLE IF EXISTS `vcf_invite`;
CREATE TABLE `vcf_invite`  (
  `pid` int(11) NOT NULL,
  `code` varchar(32) CHARACTER SET utf8 COLLATE utf8_general_ci NULL DEFAULT NULL,
  `entered` tinyint(4) NOT NULL DEFAULT 0,
  `invcount` smallint(6) NOT NULL DEFAULT 0,
  `total` smallint(6) NOT NULL DEFAULT 0,
  `streamer` tinyint(4) NOT NULL DEFAULT 0,
  `comment` varchar(32) CHARACTER SET utf8 COLLATE utf8_general_ci NULL DEFAULT NULL,
  PRIMARY KEY (`pid`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_iteminfo
-- ----------------------------
DROP TABLE IF EXISTS `vcf_iteminfo`;
CREATE TABLE `vcf_iteminfo`  (
  `code` varchar(16) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `name` varchar(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `desc` varchar(64) CHARACTER SET utf8 COLLATE utf8_general_ci NULL DEFAULT NULL,
  PRIMARY KEY (`code`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_itemown
-- ----------------------------
DROP TABLE IF EXISTS `vcf_itemown`;
CREATE TABLE `vcf_itemown`  (
  `pid` int(11) NOT NULL,
  `lastupdate` timestamp(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0) ON UPDATE CURRENT_TIMESTAMP(0),
  `code` varchar(16) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `amount` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`code`) USING BTREE,
  INDEX `FK_itemown_iteminfo`(`code`) USING BTREE,
  INDEX `FK_itemown_pid`(`pid`) USING BTREE,
  CONSTRAINT `FK_itemown_iteminfo` FOREIGN KEY (`code`) REFERENCES `vcf_iteminfo` (`code`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `FK_itemown_pid` FOREIGN KEY (`pid`) REFERENCES `vcf_users` (`pid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_logs
-- ----------------------------
DROP TABLE IF EXISTS `vcf_logs`;
CREATE TABLE `vcf_logs`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `admin` char(17) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '',
  `adminnick` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `serverid` tinyint(4) NOT NULL DEFAULT 0,
  `operate` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `target` char(17) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '',
  `targetname` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `operationtime` int(11) NOT NULL,
  `note` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 7 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_mod
-- ----------------------------
DROP TABLE IF EXISTS `vcf_mod`;
CREATE TABLE `vcf_mod`  (
  `id` tinyint(4) NOT NULL AUTO_INCREMENT,
  `name` varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 6 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_servers
-- ----------------------------
DROP TABLE IF EXISTS `vcf_servers`;
CREATE TABLE `vcf_servers`  (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `modid` tinyint(4) NOT NULL,
  `name` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '',
  `address` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `public` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `shortname` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `invisible` int(11) NOT NULL DEFAULT 0 COMMENT '为1则不显示',
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `FK_modid_Servers`(`modid`) USING BTREE,
  CONSTRAINT `FK_modid_Servers` FOREIGN KEY (`modid`) REFERENCES `vcf_mod` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE = InnoDB AUTO_INCREMENT = 4 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_sign
-- ----------------------------
DROP TABLE IF EXISTS `vcf_sign`;
CREATE TABLE `vcf_sign`  (
  `pid` int(11) NOT NULL,
  `lastsign` int(11) NOT NULL DEFAULT 0,
  `continoussign` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`pid`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_stats
-- ----------------------------
DROP TABLE IF EXISTS `vcf_stats`;
CREATE TABLE `vcf_stats`  (
  `pid` int(11) NOT NULL,
  `modid` int(3) NOT NULL DEFAULT 0,
  `slot1` int(12) NOT NULL DEFAULT 0,
  `slot2` int(12) NOT NULL DEFAULT 0,
  `slot3` int(12) NOT NULL DEFAULT 0,
  `slot4` int(12) NOT NULL DEFAULT 0,
  `slot5` int(12) NOT NULL DEFAULT 0,
  `slot6` int(12) NOT NULL DEFAULT 0,
  `slot7` int(12) NOT NULL DEFAULT 0,
  `slot8` int(12) NOT NULL DEFAULT 0,
  PRIMARY KEY (`pid`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_upload
-- ----------------------------
DROP TABLE IF EXISTS `vcf_upload`;
CREATE TABLE `vcf_upload`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `steamid` char(17) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '0',
  `filename` varchar(100) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '0',
  `size` int(11) NOT NULL DEFAULT 0,
  `time` int(11) NOT NULL DEFAULT 0,
  `status` varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 3 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_users
-- ----------------------------
DROP TABLE IF EXISTS `vcf_users`;
CREATE TABLE `vcf_users`  (
  `pid` int(11) NOT NULL AUTO_INCREMENT,
  `auth` char(17) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '0',
  `rulehonor` tinyint(4) NOT NULL DEFAULT 50,
  `otherhonor` tinyint(4) NOT NULL DEFAULT 30,
  `steamhonor` tinyint(4) NOT NULL DEFAULT 20,
  `warningcredits` tinyint(4) NOT NULL DEFAULT 0,
  `cleardate` char(11) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '0',
  `cleartimes` int(11) NOT NULL DEFAULT 0,
  `totalpurchase` decimal(6, 2) NOT NULL,
  PRIMARY KEY (`pid`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 5 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_vip
-- ----------------------------
DROP TABLE IF EXISTS `vcf_vip`;
CREATE TABLE `vcf_vip`  (
  `pid` int(11) NOT NULL,
  `modid` tinyint(4) NOT NULL,
  `viplevel` tinyint(4) NOT NULL,
  INDEX `FK_ModID_VIP`(`modid`) USING BTREE,
  CONSTRAINT `FK_ModID_VIP` FOREIGN KEY (`modid`) REFERENCES `vcf_mod` (`id`) ON DELETE NO ACTION ON UPDATE CASCADE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_viplevel
-- ----------------------------
DROP TABLE IF EXISTS `vcf_viplevel`;
CREATE TABLE `vcf_viplevel`  (
  `level` tinyint(4) NOT NULL DEFAULT 0,
  `name` varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '0',
  `needpurchase` float(11, 2) NOT NULL DEFAULT 0.00,
  PRIMARY KEY (`level`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for vcf_warninglog
-- ----------------------------
DROP TABLE IF EXISTS `vcf_warninglog`;
CREATE TABLE `vcf_warninglog`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `steamid` char(17) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '0',
  `type` char(1) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '0',
  `value` tinyint(4) NOT NULL DEFAULT 0,
  `time` char(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '0',
  `reason` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '0',
  `note` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 4 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Dynamic;

DROP TABLE IF EXISTS `vcf_couples`;
CREATE TABLE `vcf_couples` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `source_id` int(11) unsigned NOT NULL DEFAULT '0',
  `target_id` int(11) unsigned NOT NULL DEFAULT '0',
  `date` int(11) unsigned NOT NULL DEFAULT '0',
  `exp` int(11) unsigned NOT NULL DEFAULT '0',
  `together` int(11) unsigned NOT NULL DEFAULT '0',
  `luv` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Id`),
  UNIQUE KEY `P` (`source_id`,`target_id`)
) ENGINE=InnoDB AUTO_INCREMENT=0 DEFAULT CHARSET=utf8mb4;

SET FOREIGN_KEY_CHECKS = 1;
