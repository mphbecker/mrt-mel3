// MEL3.c: Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "mrt-mel3.h"
#include <time.h>

int main()
{
	printf("Started\n");
	//https://stackoverflow.com/questions/2230758/what-does-lpcwstr-stand-for-and-how-should-it-be-handled-with
	LPCWSTR port = L"COM4";

	BOOL success = FALSE;

	// Opening
	HANDLE halbesHandle;
	halbesHandle = CreateFile(port,
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (halbesHandle == INVALID_HANDLE_VALUE) {
		success = FALSE;
		wprintf(L"Error while opening port: %ls, ERRORCODE: %d\n", port, GetLastError());
		return 1;
	}
	wprintf(L"Successfully opened port %ls, Handle is: %d\n", port, halbesHandle);

	// Checking Status
	DCB dcb;
	SecureZeroMemory(&dcb, sizeof(DCB));

	success = GetCommState(halbesHandle, &dcb);

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
		clock_t start_clk = clock();
		
		success = WriteFile(halbesHandle, &D, 1, &sendSize, NULL);
		if (!success) {
			printf("Error while writing, ERRORCODE: %d\n", GetLastError());
			break; //TODO should we always break ?
		}
		if (sendSize != 1) {
			printf("Error not same size after send\n");
			break;
		}

		success = ReadFile(halbesHandle, rb, 15, &recvSize, NULL);
		if (!success) {
			printf("Error while reading, ERRORCODE: %d\n", GetLastError());
			break;
		}
		// TODO move to const

		if (recvSize != 15) {
			printf("Error not same size received\n");
			break;
		}

		
		float res = getResistanceFromString((char*)rb);
		float celcius = convertResistanceToCelcius(res);
		printf("received: %f \n", celcius);

		//substract process time
		long double time = ((clock() - start_clk) / (long double)CLOCKS_PER_SEC);
		//move to const
		const int well = 1000-time*1000;
		Sleep(well);
	}

	success = CloseHandle(halbesHandle);
	if (!success) {
		printf("Error while trying to close handle");
		return 0;		//return error?
	}
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
	char* substr;
	float f;
	while (i < 15) {
		// c is the current char
		char c = *(string + i);
		if (isdigit(c)) {
			if (start == -1) {
				start = i;
				strncpy(substr, string + start, 5);
				sscanf(substr, "%f", &f);				//f = resistance in k/M/ohm
			}
			strncpy(substr, string + start + 5, 2);
			// conv MOhm to Ohm
			if (strstr(substr, "M") != NULL) {
				f = f * 1000000;
			}
			if (strstr(substr, "k") != NULL) {
				f = f * 1000;
			}
			if (strstr(substr, "m") != NULL) {
				f = f / 1000;
			}
			return f;					//return resistance value
		}

		// Check for CR
		if (c == 13) {
			break;
		}
		i++;
	}

	return f;
}

float convertResistanceToCelcius(const float res) {
	return 4.80472f + 0.361433f * (-20 + res);
}
