//
//  raspiBadgerComm.cpp
//
//  Created by Jody J Roth on 6/23/14.
//  Using remote.cpp as starting point to generate program
//    that receives data from arduinos and stores information in
//    a mySQL database.  Also sends out commands tbd.
//
//  6/25/2014 - Added pthread to monitor mysql db for pokes to send out.
//  6/26/2014 - pthread monitor working. TODO: add validation before deleting the poke.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <getopt.h>
#include <iostream>
#include <cstdlib>
#include <mysql.h>	// MySQL libraries
#include "../RF24.h"	// NRF24L01 radio
#include <signal.h>	// Graceful exit, close db...

#include <pthread.h>	// Multi-threading to check db for pokes

using namespace std;

MYSQL *connection, *connection2, mysql, mysql2;
MYSQL_RES *result, *result2;
MYSQL_ROW row, row2;
int query_state, query_state2;

#define HOST "localhost"
#define USER "badger"
#define PASSWD "get3badgers"
#define DB "raspiBadgerData"

RF24 radio("/dev/spidev0.0",8000000 , 25);  //spi device, speed and CSN,only CSN is NEEDED in RPI

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

void setup(void){
	//Prepare the radio module
	printf("\nPreparing interface\n");
	radio.begin();
	// radio.enableDynamicPayloads();
	radio.setAutoAck(1);
	radio.setRetries( 15, 15);
	//radio.setDataRate(RF24_1MBPS);
	radio.setPALevel(RF24_PA_MAX);
	radio.setChannel(0x4c);  //76
	//radio.setCRCLength(RF24_CRC_16);
	
	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1,pipes[1]);
	radio.startListening();
	radio.printDetails();
	usleep(50);
}

bool sendToBadge(long int badgeID, long int action){
	//This function sends a message to the arduino
        //BadgeID and ActionCode
	//Returns true if ACK package is received
	// Will need to change this to only return true if the correct
	// badge received the message.  Right now, all the badges will
	// return true even if the target is not on the network.

	//Stop listening
	radio.stopListening();
	long unsigned int message[] = {badgeID, action};
	printf("Now sending %lu / %lu", message[0], message[1]);
	//Send the message
	bool ok = radio.write( &message, sizeof(message) );
	if (ok) {
		printf("Sent ok...");
		return true;
	}
	else {
		printf("failed.\n\r");
		return false;
	}
	radio.startListening();
	usleep(50);
	////Listen for ACK
	//radio.startListening();
	////Let's take the time while we listen
	//unsigned long started_waiting_at = __millis();
	//bool timeout = false;
	//while ( ! radio.available() && ! timeout ) {
	//	__msleep(10);
	//	if (__millis() - started_waiting_at > 1000 )
	//		timeout = true;
	//}
	//   
	//if( timeout ){
	//	//If we waited too long the transmission failed
	//	printf("Failed, response timed out.\n\r");
	//	return false;
	//}else{
	//	//If we received the message in time, let's read it and print it
	//	unsigned long got_time[2];
	//	radio.read( &got_time, sizeof(got_time) );
	//	printf("Got response Badge#: %lu, message: %lu in %lu milliseconds\n\r",got_time[0],got_time[1],__millis()-started_waiting_at);
	//	return true;
	//}
}

void closeMySQL(){  //unused at this time
    mysql_close(connection);
    cout << "Disconnected from database." << endl;
}

void *pokeChecker(void *threadid)
{	
	// debug
	cout << "pokeChecker thread starting..." << endl;
	//initialize database connection
	mysql_init(&mysql2);

	// check db every 2 second for pending actions and send as needed
	char query2[80] = {0}, query3[80] = {0};

	while (1) {
		connection2 = mysql_real_connect(&mysql2,HOST,USER,PASSWD,DB,0,0,0);
		// Report error if failed to connect to database
		if (connection2 == NULL) {
			cout << "mySQL2 Err: " << mysql_error(&mysql2) << endl;
			//return 1;
		}		
		sprintf(query2, "SELECT badgeID, action FROM badgeAction");
		mysql_query(connection2, query2);  // not sure if I can use the same connection
		result2 = mysql_store_result(connection2);
		while ( ( row2 = mysql_fetch_row(result2)) != NULL ) {
			// send data to badge
			if(sendToBadge(atol(row2[0]), atol(row2[1]))) {
				// sendToBadge returned true so message was sent and received by badge.
				cout << "Message sent to: " << row2[0] << endl;	
				// if ack'd, delete row from db
				sprintf(query3, "DELETE FROM badgeAction WHERE badgeID = %ld AND action = %ld", atol(row2[0]), atol(row2[1]));
				mysql_query(connection2, query3);
				// check to see if it was deleted
				//
				cout << "That last one should have been deleted..." << endl;
			} else {
				// sendToBadge returned 0 and failed to send the message.
				// might want to add column in db for # of sends so can delete
				// poke if not successfull after many tries
				// also, sendToBadge doesn't care right now if the right badge
				// received the message.
				cout << "Message to: " << row2[0] << " failed" << endl;
			}
		}
				
		mysql_close(connection2);
		cout << "pthread pokeChecker going to sleep for 5 seconds" << endl;
		sleep(5);  //wait 5 seconds then check again
	}

	pthread_exit(NULL);
}

void gracefulExit(int sig) {
	// Leaving app, clean up and exit
	pthread_exit(NULL);
	mysql_close(connection);
	cout << "Disconnected from database." << endl;	
	exit(0);
}

int main(int argc, char **argv)
{
	//signal(SIGINT, gracefulExit);  //called on cntl-c exit, not visibly working...
	
	// validate number of arguments (temp for testing, kill args after nRF24 integration since data is radio'd in)
/*	if ((argv[0] == '-h')||(argv[0]=='--help') // possibly add options later
		cout << "Usage: raspiBadgerComm2 [badgeID] [action]" << endl << endl;
		cout << "Option  GNU long option  meaning" << endl;
		cout << "-h      --help           this command line help" << endl;
		return 0;
	} else */
	if (!(argc == 3)) {  // [0] is file info plus two arguments
		cout << "Invalid number of arguments; usage: " << argv[0] << " num1 num2\n";
		//don't need to exit if going into a loop
		//exit(1);
	} else {
		cout << "Valid number of arguments\n\n"; //debugging 
	}

	// initialize radio
	setup();
	
	//initialize database connection
	mysql_init(&mysql);
    
	// the three zeros are: Which port to connect to, which socket to connect to
	// and what client flags to use.  unless you're changing the defaults you only need to put 0 here
	connection = mysql_real_connect(&mysql,HOST,USER,PASSWD,DB,0,0,0);
    
	// Report error if failed to connect to database
	if (connection == NULL) {
		cout << "mySQL Err: " << mysql_error(&mysql) << endl;
		return 1;
	}
	// Send reported badge data to database
	char query[80] = {0};
	sprintf(query, "INSERT INTO badgeTick (badgeID, ticksTotal) VALUES (%d,%d)", atoi(argv[1]), atoi(argv[2]));
	mysql_query(connection, query);
   
	// Send query to database and display all entries (debugging)
	query_state = mysql_query(connection, "select * from badgeTick");
    
	// store result
	result = mysql_store_result(connection);
	while ( ( row = mysql_fetch_row(result)) != NULL ) {
		// Print result, it prints row[column_number])
		cout << row[0] << "\t" << row[1] << endl;
	}

	// create poke thread to relay pokes
	pthread_t pokeThread;
	int pt;
	cout << "Creating pokeChecker thread within Main..." << endl;
	pt = pthread_create(&pokeThread, NULL, pokeChecker, (void *)1); // from NULL to 1
	if (pt){
		cout << "Error:unable to create thread," << pt << endl;
		exit(-1);
	}

	// Loop to listen for new transmissions
	while (1) {
		// listen for transmissions from badges and
		while (radio.available()) {  //TODO: possibly need to send pipe as arg
			//If we received the message in time, let's read it and print it
			unsigned long incomingMsg[2];
			radio.read( &incomingMsg, sizeof(incomingMsg) );
			printf("Received message: badgeID: %lu  Message: %lu. ", incomingMsg[0], incomingMsg[1]);
			// send ack
			//radio.stopListening();
			//radio.write( incomingMsg,sizeof(incomingMsg));
			//printf("Sent response: %lu / %lu \n\r",incomingMsg[0], incomingMsg[1]);
			//radio.startListening();
			// send to DB
			if ((incomingMsg[0] != 0)&&(incomingMsg[1] != 0)) {
				sprintf(query, "INSERT INTO badgeTick (badgeID, ticksTotal) VALUES (%lu,%lu)", incomingMsg[0], incomingMsg[1]);
				mysql_query(connection, query);
				cout<<"Recorded in db"<<endl;
			} else {
				cout << "and ignored zeroes" << endl;
			}
		}
		usleep(20);
	}
}
/*
Notes:
%d    -   int
%u    -   unsigned
%ld   -   long
%lld  -   long long
%lu   -   unsigned long
%llu  -   unsigned long long
*/