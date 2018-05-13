#ifndef SIM800L_H
#define SIM800L_H

#include "Arduino.h"
#include "SoftwareSerial.h"

#define SIM800_TX_PIN 10
#define SIM800_RX_PIN 9

#define TIMEOUT 90000
#define TRIALS 5

#define SEE(X) Serial.println(X)

#define USE_RESPONSE true
#define IGNORE_RESPONSE false
#define RESTART true
#define CLOSE_ONLY false

class NETWORK
{
	public:

		NETWORK(int baudRate = 9600):serialSIM800(SIM800_RX_PIN,SIM800_TX_PIN){
			serialSIM800.begin(baudRate);
		}
		int initModule(unsigned char trials);
		int checkResponse(const char *resp, unsigned long timeout, bool flag);
		int sendCmdAndWaitForResponse(const char *cmd, const char *a_part_of_expected_response, unsigned long timeout, unsigned char trials, bool flag);
		int initAndSendSMS(const char *message, const char *phone_number);
		void emptyBuffer(char *buff);
		//int connect1(const char *apn, const char *username, const char *password, int port=80, const char *website);
		//int sendRequest(void);
		int sendRequest2(const char *request);
		int setupInternet2(const char *apn, const char *username, const char *password, int port, const char *website);
		//int setupInternet(const char *apn, const char *request);
		int waitForServerResponse(void);
		char readResponse(void);
		//void closeNetwork(void);
		void closeNetwork2(bool flag);
		
		SoftwareSerial serialSIM800;
		
		private:
	};
#endif
	

		
		
