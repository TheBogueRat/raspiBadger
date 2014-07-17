raspiBadger 
===========

raspiBadger is firstly, an inexpensive sensor network in the range of $10 per badge.  The goal is a complete IoT solution for deploying remote sensors (badges) that report sensor data to a central database and can be poked, to act or report, via a web interface.  There are several great tutorials already published but they are light on detail and function.  My goal is to provide a one stop starting point for people like me that want two-way communication, IoT, storage, and low cost sensors.

The PHP folder is the web interface to the MySQL database and remote badges.  Simply place the contents into your /etc/www directory and edit the db credentials.

The MySQL folder is the current database structure.  Import using PHPMyAdmin.

raspiBadgerLogger is the program that communicates between the Raspberry Pi and Arduino RF24L01+ modules.

Detailed installation instructions are provided at http://hobbies.boguerat.com
