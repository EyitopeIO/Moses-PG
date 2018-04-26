#ifndef SIM800L_H
#define SIM800L_H

#include "Arduino.h"
#include "SoftwareSerial.h"

#define SIM800_TX_PIN           10
#define SIM800_RX_PIN           9

#define TIMEOUT 2000
#define TRIALS 5

class NETWORK
{
	public:
		
		NETWORK(int baudRate = 9600):serialSIM800(SIM800_TX_PIN,SIM800_RX_PIN){
			serialSIM800.begin(baudRate);
		}
		int initModule(int trials);
		int checkResponse(const char *resp, int timeout);
		int sendCmdAndWaitForResponse(const char *cmd, const char *a_part_of_expected_response, int timeout, int trials=TRIALS);
		int initAndSendSMS(const char *message, const char *phone_number);
		//void showIpAdress(void);
		void emptyBuffer(char *buff);
		//int connect1(const char *apn, const char *username, const char *password, int port=80, const char *website);
		int sendRequest(void);
		int setupInternet(const char *apn, const char *request);
		int waitForServerResponse(void);
		char readResponse(void);
		
		
		SoftwareSerial serialSIM800;
		
		private:
	};
#endif
	

		
		

