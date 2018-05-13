// MEL3.c: Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "mrt-mel3.h"

int main()
{
	printf("Started\n");
	//https://stackoverflow.com/questions/2230758/what-does-lpcwstr-stand-for-and-how-should-it-be-handled-with
	LPCWSTR port = L"COM4";

	BOOL success = FALSE;

	// Opening
	HANDLE hComm;
	hComm = CreateFile(port,
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hComm == INVALID_HANDLE_VALUE) {
		success = FALSE;
		wprintf(L"Error while opening port: %ls, ERRORCODE: %d\n", port, GetLastError());
		return 1;
	}
	wprintf(L"Successfully opened port %ls, Handle is: %d\n", port, hComm);

	// Checking Status
	DCB dcb;
	SecureZeroMemory(&dcb, sizeof(DCB));

	success = GetCommState(hComm, &dcb);

	if (!success)
	{
		printf("GetCommState failed with error %d.\n", GetLastError());
		return (2);
	}
	// TODO do move checking

	//TODO move to const + generic send string or byte
	char rb[1];
	BYTE D = 0x44; // 44 in Hex is the char 'D'

				   // wait for keyboard hit
	while (!_kbhit()) {
		// Send D to Multimeter
		DWORD sendSize = 0;
		DWORD recvSize = 0;
		
		success = WriteFile(hComm, &D, 1, &sendSize, NULL);
		if (!success) {
			printf("Error while writing, ERRORCODE: %d\n", GetLastError());
			break; //TODO should we always break ?
		}
		if (sendSize != 1) {
			printf("Error not same size\n");
			break;
		}

		success = ReadFile(hComm, rb, 15, &recvSize, NULL);
		if (!success) {
			printf("Error while reading, ERRORCODE: %d\n", GetLastError());
			break;
		}
		// TODO move to const
		if (recvSize != 15) {
			printf("Error not same size\n");
			break;
		}

		
		float res = getResistanceFromString((char*)rb);
		float celcius = convertResistanceToCelcius(res);
		printf("received: %f \n", celcius);

		// TODO substract process time
		// TODO move to const
		Sleep(1000);
	}

	success = CloseHandle(hComm);

	// Closing

	printf("Done\n");
	return 0;
}

void PrintCommState(const DCB dcb)
{
	//  Print some of the DCB structure values
	_tprintf(TEXT("\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n"),
		dcb.BaudRate,
		dcb.ByteSize,
		dcb.Parity,
		dcb.StopBits);
}

float getResistanceFromString(const char* string) {
	int i = 0;
	int start = -1;
	while (i < 15) {
		// c is the current char
		char c = *(string + i);
		if (isdigit(c)) {
			if (start == -1) {
				start = i;
			}
		}
		// Check for CR
		if (c == 13) {
			break;
		}
		i++;
	}

	return 1;
}

float convertResistanceToCelcius(const int res) {
	return 4.80472f + 0.361433f * (-20 + res);
}
