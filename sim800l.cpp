#include "sim800l.h"
#include <stdio.h>

int NETWORK::initModule(int trials) {
	//test module
	sendCmdAndWaitForResponse("AT\r\n", "OK", TIMEOUT, TRIALS);
	
	//enable full functionality of modem
	sendCmdAndWaitForResponse("AT+CFUN=1\r\n", "OK", TIMEOUT, TRIALS),

	//check if simcard ready to send sms or send data.
	//it returns +CPIN:READY
	sendCmdAndWaitForResponse("AT+CPIN?\r\n", "READY", TIMEOUT, TRIALS)
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
	return 1;
}

char NETWORK:readResponse(void)
{
	char c;
	if(serial.available()){
		c = serialSIM800.read();
		return (char)c;
	}
}

int NETWORK::initAndSendSMS(const char *message, const char *phone_number)
{
	//set the stuff to text mode
	sendCmdAndWaitForResponse("AT+CMGF=1\r\n", "OK", TIMEOUT, TRIALS);
	
	//set the phone number
	char cmd[12];
	sprintf(cmd,"AT+CMGS=\"%s\"\r", phone_number);
	sendCmdAndWait(cmd, ">", TIMEOUT, TRIALS);
	
	//it is assumed the user has added the '\r' character.
	serialSIM800.write(message);
	
	//send the CTRL+Z character
	//i couldn't find a way to enter this on the serial monitor.
	//you could write some code to send it when you enter some text.
	serialSIM800.write((char)26); //or 0X1A;
	if (!checkForResponse("OK",TIMEOUT)) {
		return 0;
	}
}

int NETWORK::sendCmdAndWaitForResponse(const char *cmd, const char *a_part_of_expected_response, TIMEOUT, TRIALS){
	int trials = 0;
	do {
		delay(500);
		serialSIM800.write(cmd);
		trials++;
		if(trials == TRIALS) {
			return 0;
		}
	} while (!checkResponse(a_part_of_expected_response,TIMEOUT));
	return 1;
}

int NETWORK::checkResponse(const char *response, int timeout) {
	//Only checks if the given string can be found
	//in the response of SIM800L.
	unsigned long int timerstart, timerend;
	int len = strlen(response);
	timerstart = millis();
	int i = 0;
	while(1){
		if(serialSIM800.available()){
			i = (serialSIM800.read() == response[i]) ? i+1 : 0;
			if (i == len) break;
		}
		timerend = millis();
		if(timerend-timerstart > timeout) break;
	}
	return 1;
}

NETWORK::emptyBuffer(char *buffer){
	unsigned int i;
	for (i=0;i<strlen(buffer);i++){
		buffer[i] = 0;
	}
}
	
NETWORK::setupInternet(const char *apn, const char *request)
{
	char cmd[50];
	
	//set connection type to gprs
	emptyBuffer(cmd);
	sendCmdAndWaitForResponse("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n", "OK", TIMEOUT, TRIALS);

	//set access point
	sprintf(cmd,"AT+SAPBR=3,1,\"APN\",\"%s\"\r\n",apn);
	sendCmdAndWaitForResponse(cmd, "OK", TIMEOUT, TRIALS);
	emptyBuffer(cmd);

	//enable the gprs
	sendCmdAndWaitForResponse("AT+SAPBR=1,1\r\n");

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

int NETWORK::sendRequest(void) {
	//tell the module to send my request
	sendCmdAndWaitForResponse("AT+HTTPACTION=0\r\n", "OK", TIMEOUT, 2);
	serialSIM800.write("AT+HTTPREAD\r\n");
}



/*NETWORK::connect1(const char *apn, const char *username, const char *password, int port=80, const char *website="openweathermap.org") {
	//website is openweathermap.org
	
	//set the access point, username and password for internet connection
	char cmd[50];
	//format string and save in cmd. returns number of bytes written.
	sprintf(cmd, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", apn,username,password);
	sendCmdAndWait(cmd, "OK",TIMEOUT);

	//setup wireless gprs connection
	sendCmdAndWait("AT+CIFSR=?","OK",TIMEOUT);

	how ip on serial monitor. comment out when code works fine
	showIpAddress();

	//starts a TCP connection
	emptyBuffer(cmd);
	sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",80\r\n",website);
	sendCmdAndWait(cmd, "OK", TIMEOUT);

	emptyBuffer(cmd);
	sprintf(cmd, "AT+CIPSEND=%d\r\n",str);
	sendCmdAndWait(cmd
}
*/

