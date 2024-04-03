#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <Windows.h>
#include <string>
#pragma warning(disable: 4996)

using namespace std;

SOCKET Connections[100];
int Counter = 0;

int ClientNumb[100];
int N[50];
int Lobby[2] = { -1, -1 };
bool player1 = true;
bool player2 = true;
bool EntryMsgGl = true;

//Отправка сообщений
void SendMessageString(string message, SOCKET newConnection) {
	int message_size = message.size();
	send(newConnection, (char*)&message_size, sizeof(int), NULL);
	send(newConnection, message.c_str(), message_size, NULL);
}


//Перевод строки в число
int CharToInt(char* arr, int sizearr) {
	int numb = 0;
	for (int i = 0; i < sizearr; i++) {
		if (arr[i] >= '0' && arr[i] <= '9') {
			numb += (arr[i] - '0') * pow(10, sizearr - i - 1);
		}
		else {
			return -1;
		}
	}
	return numb;
}


//Сравнение чисел
int Comparison(int numb1, int numb2) {
	if (numb1 == numb2) {
		return 0;
	}
	else if (numb1 > numb2) {
		return 1;
	}
	else {
		return -1;
	}
}


//Отправка и обработка сообщений от клиента
void ClientHandler(int index) {

	int msg_size;
	int number;
	int pl1 = -1;
	int pl2 = -1;

	bool MakeNumber = true;
	bool EntryLobby = true;
	bool CheckCuple = true;
	bool StartGame = false;
	bool EndGame = false;

	string msgtoclient;

	while (true) {


		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);
		number = CharToInt(msg, msg_size);

		//Загадывание клиентом числа
		if (MakeNumber) {
			if (msg[0] == 0) {
				continue;
			}
			if (number != -1) {
				ClientNumb[index] = number;
				cout << "Загаданное число пользователя (" << index + 1 << "): " << ClientNumb[index] << endl;
				msgtoclient = "Вы загадали число: " + to_string(number) + '\n';
				SendMessageString(msgtoclient, Connections[index]);
				MakeNumber = false;
			}
			else {
				msgtoclient = "Нужно ввести натуральное число!\n";
				SendMessageString(msgtoclient, Connections[index]);
				continue;
			}
		}

		//Вход в лобби
		if (EntryLobby) {
			if (Lobby[0] == -1) {
				Lobby[0] = index;
				EntryLobby = false;
			}
			else {
				Lobby[1] = index;
				EntryLobby = false;
			}
		}


		//Проверка подключения
		while (CheckCuple) {

			if (Lobby[1] == -1) {
				msgtoclient = "Второй игрок не подключился или не загадал число\n";
				SendMessageString(msgtoclient, Connections[index]);
				Sleep(3000);
				continue;
			}
			else if (Lobby[0] == index) {
				msgtoclient = "Второй игрок подключился! Игра начинается\n";
				SendMessageString(msgtoclient, Connections[index]);
				CheckCuple = false;
				continue;
			}
			else {
				msgtoclient = "Вы подключились к игре! Игра начианается\n";
				SendMessageString(msgtoclient, Connections[index]);
				CheckCuple = false;
				continue;
			}
		}

		while (!StartGame)
		{
			if (Lobby[0] == index) {
				pl1 = Lobby[0];
				pl2 = Lobby[1];
				player1 = true;
			}
			else {
				pl1 = Lobby[0];
				pl2 = Lobby[1];
				player2 = true;
			}
			Sleep(5000);
			if (player1 && player2) {
				StartGame = true;
				Lobby[0] = -1;
				Lobby[1] = -1;
				N[pl1] = 0;
			}
		}


		//Игра
		while (StartGame) {
			int result;
			//Case1
			if (N[pl1] == -1) {
				EndGame = true;
				StartGame = false;
			}
			if (N[pl1] % 3 == 0) {
				if (pl1 == index && !EntryMsgGl) {
					msgtoclient = "Угадывай число второго игрока:  ";
					SendMessageString(msgtoclient, Connections[pl1]);
					N[pl1] += 1;
					break;
				}
				else if(pl2 == index && EntryMsgGl){
					msgtoclient = "Ваше число пытаются угадать\n";
					SendMessageString(msgtoclient, Connections[pl2]);
					EntryMsgGl = false;
					continue;
				}
			}

			//Case2
			else if (N[pl1] % 3 == 1) {
				if (pl1 == index && !EntryMsgGl) {
					result = Comparison(number, ClientNumb[pl2]);
					if (result == 0) {
						msgtoclient = "!!!Вы угадали число пользователя!!!\nПоздравляю\n";
						SendMessageString(msgtoclient, Connections[pl1]);
						msgtoclient = "!!!Ваше число угадали!!!\nНе расстраивайтесь\n";
						SendMessageString(msgtoclient, Connections[pl2]);
						N[pl1] = -1;
						continue;
					}
					else if (result == 1) {
						msgtoclient = "МЕНЬШЕ\n";
						SendMessageString(msgtoclient, Connections[pl1]);
						msgtoclient = "Ваше число не угадали!\n";
						SendMessageString(msgtoclient, Connections[pl2]);
					}
					else {
						msgtoclient = "БОЛЬШЕ\n";
						SendMessageString(msgtoclient, Connections[pl1]);
						msgtoclient = "Ваше число не угадали!\n";
						SendMessageString(msgtoclient, Connections[pl2]);
					}
					EntryMsgGl = true;
					continue;
				}
				else if (pl2 == index && EntryMsgGl) {
					msgtoclient = "Угадывай число второго игрока: ";
					SendMessageString(msgtoclient, Connections[pl2]);
					N[pl1] += 1;
					break;
				}
			}

			//Case3
			else if (N[pl1] % 3 == 2) {
				Sleep(10);
				if (pl1 == index && EntryMsgGl) {
					msgtoclient = "Ваше число пытаются угадать\n";
					SendMessageString(msgtoclient, Connections[pl1]);
					EntryMsgGl = false;
					continue;
				}
				else if (pl2 == index && !EntryMsgGl) {
					result = Comparison(number, ClientNumb[pl1]);
					if (result == 0) {
						msgtoclient = "!!!Вы угадали число пользователя!!!\nПоздравляю\n";
						SendMessageString(msgtoclient, Connections[pl2]);
						msgtoclient = "!!!Ваше число угадали!!!\nНе расстраивайтесь\n";
						SendMessageString(msgtoclient, Connections[pl1]);
						N[pl1] = -1;
						continue;
					}
					else if (result == 1) {
						msgtoclient = "МЕНЬШЕ\n";
						SendMessageString(msgtoclient, Connections[pl2]);
						msgtoclient = "Ваше число не угадали!\n";
						SendMessageString(msgtoclient, Connections[pl1]);
					}
					else {
						msgtoclient = "БОЛЬШЕ\n";
						SendMessageString(msgtoclient, Connections[pl2]);
						msgtoclient = "Ваше число не угадали!\n";
						SendMessageString(msgtoclient, Connections[pl1]);
					}
					N[pl1] += 1;
					EntryMsgGl = true;
					continue;
				}	
			}		
		}
		delete[] msg;
	}
}

int main(int argc, char* argv[]) {
	
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	
	//WSAStartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		cout << "Ошибка" << endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);

	cout << "Сервер запущен!\n";

	SOCKET newConnection;
	for (int i = 0; i < 100; i++) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		if (newConnection == 0) {
			cout << "Ошибка #2\n";
		}
		else {
			cout << "Пользователь (" << i+1 << ") подключился!" << endl;
			string msg = "Загадай своё число: ";
			SendMessageString(msg, newConnection);

			Connections[i] = newConnection;
			Counter++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
		}
	}


	system("pause");
	return 0;
}