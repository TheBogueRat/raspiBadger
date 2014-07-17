-- phpMyAdmin SQL Dump
-- version 4.2.2
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Jul 17, 2014 at 09:36 AM
-- Server version: 5.5.37-0+wheezy1
-- PHP Version: 5.4.4-14+deb7u11

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `raspiBadgerData`
--
CREATE DATABASE IF NOT EXISTS `raspiBadgerData` DEFAULT CHARACTER SET latin1 COLLATE latin1_swedish_ci;
USE `raspiBadgerData`;

-- --------------------------------------------------------

--
-- Table structure for table `badgeAction`
--

CREATE TABLE IF NOT EXISTS `badgeAction` (
`actionID` int(11) NOT NULL,
  `badgeID` int(11) NOT NULL COMMENT 'Badge ID to contact',
  `action` int(11) NOT NULL COMMENT 'Action for badge to take'
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `badgeTick`
--

CREATE TABLE IF NOT EXISTS `badgeTick` (
  `badgeID` int(11) NOT NULL COMMENT 'Badge Number that submitted a report',
  `ticksTotal` int(11) NOT NULL COMMENT '# of ticks reported by Badge',
  `tickTimeDate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Date/Time of reported Tick',
`badgeTickId` int(11) NOT NULL COMMENT 'Index'
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=319 ;

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE IF NOT EXISTS `users` (
`userID` int(11) NOT NULL,
  `username` varchar(28) COLLATE utf8_unicode_ci NOT NULL,
  `password` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `salt` varchar(8) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=3 ;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `badgeAction`
--
ALTER TABLE `badgeAction`
 ADD PRIMARY KEY (`actionID`);

--
-- Indexes for table `badgeTick`
--
ALTER TABLE `badgeTick`
 ADD PRIMARY KEY (`badgeTickId`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
 ADD PRIMARY KEY (`userID`), ADD UNIQUE KEY `username` (`username`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `badgeAction`
--
ALTER TABLE `badgeAction`
MODIFY `actionID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `badgeTick`
--
ALTER TABLE `badgeTick`
MODIFY `badgeTickId` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Index',AUTO_INCREMENT=319;
--
-- AUTO_INCREMENT for table `users`
--
ALTER TABLE `users`
MODIFY `userID` int(11) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=3;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
