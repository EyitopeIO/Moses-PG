#include "sim800l.h"
#include <stdio.h>
#include<string.h>



int NETWORK::initModule(unsigned char trials) {
	//test modulE
	sendCmdAndWaitForResponse("AT\r\n", "OK", TIMEOUT, trials, USE_RESPONSE);
	
	//enable full functionality of modem
	sendCmdAndWaitForResponse("AT+CFUN=1\r\n", "OK", TIMEOUT, trials, USE_RESPONSE);

	//check if simcard ready to send sms or send data.
	//it returns +CPIN:READY
	sendCmdAndWaitForResponse("AT+CPIN?\r\n", "READY", TIMEOUT, trials, USE_RESPONSE);
}

int NETWORK::waitForServerResponse(void)
{
	unsigned int jsonend = 0;
	unsigned long int timeout = millis();
	while(serialSIM800.available() == 0){
		if(millis() - timeout > 30000){
			return 0;
		}
	}
}


char NETWORK::readResponse(void)
{
	char c;
	if(Serial.available()){
		c = serialSIM800.read();
		return (char)c;
	}
}

int NETWORK::initAndSendSMS(const char *message, const char *phone_number)
{
	//set the stuff to text mode
	sendCmdAndWaitForResponse("AT+CMGF=1\r\n", "OK", TIMEOUT, TRIALS,USE_RESPONSE);
	
	//set the phone number
	char cmd[30];
	sprintf(cmd,"AT+CMGS=\"%s\"\r", phone_number);
	sendCmdAndWaitForResponse(cmd, ">", TIMEOUT, TRIALS,USE_RESPONSE);
	
	//it is assumed the user has added the '\r' character.
	serialSIM800.write(message);
	
	//send the CTRL+Z character
	//i couldn't find a way to enter this on the serial monitor.
	//you could write some code to send it when you enter some text.
	serialSIM800.write((char)26); //or 0X1A;
	if (!checkResponse("OK",TIMEOUT,USE_RESPONSE)) {
		return 0;
	}
}

int NETWORK::sendCmdAndWaitForResponse(const char *cmd, const char *a_part_of_expected_response, unsigned long timeout, unsigned char trials, bool flag){
	int tries = 0;
	while(1) {
		delay(200);
		SEE(cmd);
		serialSIM800.write(cmd);
		if(1 == checkResponse(a_part_of_expected_response, (unsigned long)timeout, flag)){
			return 1;
		}
		tries++;
		if(tries == trials) {
			return 0;
		}
		
	}
}

int NETWORK::checkResponse(const char *response, unsigned long timeout, bool flag) {
	//Only checks if the given string can be found
	//in the response of SIM800L
	unsigned long timerstart, timerend;
	int len = strlen(response);
	timerstart = millis();
	int i = 0;
	Serial.println(F("Loop starts."));
	while(1){
		if(serialSIM800.available()){
			Serial.println(F("GSM response"));
			char c = serialSIM800.read();
			delay(150);
			SEE(c);
			i = (c == response[i]) ? i+1 : 0;
			if (i == len) {
				delay(200);
				break;
			}
		}
		timerend = millis();
		if (timerend-timerstart > timeout){
			if(flag == USE_RESPONSE){
				Serial.println(F("returning zero"));
				return 0;
			}
			if (flag == IGNORE_RESPONSE){
				break;
			}
		}
	}
	//empty input buffer
	while(serialSIM800.available()){
		serialSIM800.read();
	}
	return 1;
}

void NETWORK::emptyBuffer(char *buffer){
	unsigned int i;
	for (i=0;i<strlen(buffer);i++){
		buffer[i] = 0;
	}
}

/*
int NETWORK::setupInternet(const char *apn, const char *request)
{
	int lenght_of_request = strlen(request);
	char cmd[lenght_of_request+15];
	//set connection type to gprs
	emptyBuffer(cmd);
	sendCmdAndWaitForResponse("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n", "OK", TIMEOUT, TRIALS);

	//set access point
	sprintf(cmd,"AT+SAPBR=3,1,\"APN\",\"%s\"\r\n",apn);
	sendCmdAndWaitForResponse(cmd, "OK", TIMEOUT, TRIALS);
	emptyBuffer(cmd);

	//enable the gprs
	sendCmdAndWaitForResponse("AT+SAPBR=1,1\r\n", "OK", TIMEOUT, TRIALS);

	//was connection setup properly?
	sendCmdAndWaitForResponse("AT+SAPBR=2,1\r\n", "OK", TIMEOUT, TRIALS);

	//enable HTTP mode
	sendCmdAndWaitForResponse("AT+HTTPINIT\r\n", "OK", TIMEOUT, TRIALS);

	//set http bearer profile identifier
	sendCmdAndWaitForResponse("AT+HTTPPARA=\"CID\",1\r\n", "OK", TIMEOUT, TRIALS);

	//set url of page to connect
	emptyBuffer(cmd);
	sprintf(cmd, "AT+HTTPPARA=\"URL\",\"%s\"\r\n",request);
	sendCmdAndWaitForResponse(cmd, "OK", TIMEOUT, TRIALS);
	emptyBuffer(cmd);
	return 1;
}
*/

/*
int NETWORK::sendRequest(void) {
	//tell the module to send my request
	sendCmdAndWaitForResponse("AT+HTTPACTION=0\r\n", "OK", TIMEOUT, 2);
	serialSIM800.write("AT+HTTPREAD\r\n");
}
*/

int NETWORK::sendRequest2(const char *request)
{
	SEE("In send request");
	delay(500);
	char cmd[25];
	emptyBuffer(cmd);
	int str = strlen(request);
	sprintf(cmd, "AT+CIPSEND=%d\r\n",str);
	sendCmdAndWaitForResponse(cmd, ">", TIMEOUT, 5, USE_RESPONSE);
	serialSIM800.print(request);
	SEE("printed request");
	serialSIM800.write(0x1A);
	SEE("dONE.");
	delay(500);
}

/*
void NETWORK::closeNetwork(void) {
	sendCmdAndWaitForResponse("AT+HTTPTERM\r\n", "OK", TIMEOUT, 2);
	sendCmdAndWaitForResponse("AT+SAPBR=0,1\r\n", "OK", TIMEOUT, 2);
}
*/

void NETWORK::closeNetwork2(bool flag)
{
	//sendCmdAndWaitForResponse("AT+CIPCLOSE=1\r\n", "OK", TIMEOUT, 2);
	sendCmdAndWaitForResponse("AT+CIPSHUT\r\n", "OK", TIMEOUT, 2, USE_RESPONSE);
	if (flag){
		sendCmdAndWaitForResponse("AT+CFUN=0\r\n", "OK", TIMEOUT, 2, USE_RESPONSE);
		initModule(2);
	}
}


int NETWORK::setupInternet2(const char *apn, const char *username, const char *password, int port, const char *website)
{
	//website is api.openweathermap.org
	
	//set the access point, username and password for internet connection
	char cmd[strlen(website)+20];
	//format string and save in cmd. returns number of bytes written.
	sprintf(cmd, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", apn,username,password);
	sendCmdAndWaitForResponse(cmd, "OK",TIMEOUT,2, USE_RESPONSE);
	
	//setup wireless gprs connection
	sendCmdAndWaitForResponse("AT+CIICR\r\n","OK",20000,2, USE_RESPONSE);

	//get ip address
	sendCmdAndWaitForResponse("AT+CIFSR\r\n","OK",15000,2, IGNORE_RESPONSE);

	//starts a TCP connection
	emptyBuffer(cmd);
	sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",80\r\n",website);
	
	while(!sendCmdAndWaitForResponse(cmd, "CONNECT OK", 20000, 2, USE_RESPONSE)) {
		//if internet doesn't connect first time, restart it. Then try again.
		//if successful, go ahead and send data
		closeNetwork2(RESTART);
		return 0;
	}
	return 1;
}

