#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <Windows.h>
#include <string>
#pragma warning(disable: 4996)

using namespace std;


const int CountUser = 10;


SOCKET Connections[CountUser];
int Counter = 0;

int ClientNumb[CountUser];
int N[CountUser / 2];
int Lobby[2] = { -1, -1 };
bool EntryMsgGl[CountUser / 2];

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
	int pl1;
	int pl2;
	int step;

	bool EntryMsg = true;
	bool MakeNumber = true;
	bool EntryLobby = true;
	bool CheckCuple = true;
	bool Game = false;

	string msgtoclient;
	string WinMsg = "!!!Вы угадали число игрока!!!\nПоздравляем\n";
	string LosserMsg = "!!!Ваше число угадали!!!\nНе расстраивайтесь\n";
	string GreaterMsg = "БОЛЬШЕ\n";
	string LessMsg = "МЕНЬШЕ\n";
	string MissMsg = "Ваше число не угадали! (Игрок ввёл: ";
	string AttemptMsg = "\nУгадывай число другого игрока:  ";
	string AlertMsg = "Ваше число пытаются угадать...\n";

	while (true) {

		//Непосредственное получение сообщения
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);
		
		//Проверка сообщения 
		if (msg[0] == 0) {
			continue;
		}
		number = CharToInt(msg, msg_size);
		if (number <= 0) {
			msgtoclient = "Нужно ввести НАТУРАЛЬНОЕ число. Повторите попытку: ";
			SendMessageString(msgtoclient, Connections[index]);
			continue;
		}

		//Загадывание клиентом числа
		if (MakeNumber) {
			ClientNumb[index] = number;
			cout << "Загаданное число пользователя (" << index + 1 << "): " << ClientNumb[index] << endl;
			msgtoclient = "Вы загадали число: " + to_string(number) + '\n';
			SendMessageString(msgtoclient, Connections[index]);
			MakeNumber = false;
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

		//Проверка подключения второго игрока
		while (CheckCuple) {
			if (Lobby[1] == -1 && EntryMsg) {
				msgtoclient = "Второй игрок не подключился или не загадал число\n";
				SendMessageString(msgtoclient, Connections[index]);
				EntryMsg = false;
				continue;
			}
			else if (Lobby[0] == index && Lobby[1] != -1) {
				msgtoclient = "Второй игрок подключился! Игра начинается...\n";
				SendMessageString(msgtoclient, Connections[index]);
				CheckCuple = false;
				EntryMsg = true;
				continue;
			}
			else if (Lobby[1] == index) {
				msgtoclient = "Вы подключились к игре! Игра начинается... \n";
				SendMessageString(msgtoclient, Connections[index]);
				CheckCuple = false;
				continue;
			}
		}

		//Предподготовка к игре: очищение лобби для новых игроков, закрепление N шага за парой игроков
		while (!Game){
			
			Game = true;
			pl1 = Lobby[0];
			pl2 = Lobby[1];
			for (int t = 0; t < (CountUser / 2); t++) {
				if (N[t] == -2) {
					step = t;
					break;
				}
			}
			if (index == pl2) {
				cout << "Game: " << pl1+1 << " & "<< pl2+1 << endl;
				Lobby[0] = -1;
				Lobby[1] = -1;
				N[step] = 0;
				EntryMsgGl[step] = true;
			}
			break;
		}

		//Игра
		while (Game) {
			int result;

			if (N[step] == -1) {
				Game = false;
				if (pl2 == index) cout << "End game: " << pl1 + 1 << " & " << pl2 + 1 <<endl;
				closesocket(Connections[pl1]);
				closesocket(Connections[pl2]);
				N[step] = -2;
				break;
			}

			//Шаг 1
			if (N[step] % 3 == 0) {
				if (pl1 == index && !EntryMsgGl[step]) {
					SendMessageString(AttemptMsg, Connections[pl1]);
					N[step] += 1;
					break;
				}
				else if (pl2 == index && EntryMsgGl[step]) {
					SendMessageString(AlertMsg, Connections[pl2]);
					EntryMsgGl[step] = false;
					continue;
				}
			}

			//Шаг 2
			else if (N[step] % 3 == 1) {
				if (pl1 == index && !EntryMsgGl[step]) {
					result = Comparison(number, ClientNumb[pl2]);
					if (result == 0) {
						SendMessageString(WinMsg, Connections[pl1]);
						SendMessageString(LosserMsg, Connections[pl2]);
						N[step] = -1;
						continue;
					}
					else if (result == 1) {
						SendMessageString(LessMsg, Connections[pl1]);
						SendMessageString(MissMsg + to_string(number) + ")\n", Connections[pl2]);
					}
					else {
						SendMessageString(GreaterMsg, Connections[pl1]);
						SendMessageString(MissMsg + to_string(number) + ")\n", Connections[pl2]);
					}
					EntryMsgGl[step] = true;
					continue;
				}
				else if (pl2 == index && EntryMsgGl[step]) {
					SendMessageString(AttemptMsg, Connections[pl2]);
					N[step] += 1;
					break;
				}
			}

			//Шаг 3
			else if (N[step] % 3 == 2) {
				Sleep(100);
				if (pl1 == index && EntryMsgGl[step]) {
					SendMessageString(AlertMsg, Connections[pl1]);
					EntryMsgGl[step] = false;
					continue;
				}
				else if (pl2 == index && !EntryMsgGl[step]) {
					result = Comparison(number, ClientNumb[pl1]);
					if (result == 0) {
						SendMessageString(WinMsg, Connections[pl2]);
						SendMessageString(LosserMsg, Connections[pl1]);
						N[step] = -1;
						continue;
					}
					else if (result == 1) {
						SendMessageString(LessMsg, Connections[pl2]);
						SendMessageString(MissMsg + to_string(number) + ")\n", Connections[pl1]);
					}
					else {
						SendMessageString(GreaterMsg, Connections[pl2]);
						SendMessageString(MissMsg + to_string(number) + ")\n", Connections[pl1]);
					}
					N[step] += 1;
					EntryMsgGl[step] = true;
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

	//Инициализзация глобальных переменных, отвечающих за ШАГ каждой комнаты
	for (int j = 0; j < (CountUser / 2); j++) {
		N[j] = -2;
	}

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

	//Непосредственный запуск сервера (ожидание подключений клиентов)
	cout << "Сервер запущен!\n";

	SOCKET newConnection;
	for (int i = 0; i < CountUser; i++) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		if (newConnection == 0) {
			cout << "Ошибка #2\n";
		}
		else {
			cout << "Пользователь (" << i + 1 << ") подключился!" << endl;
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