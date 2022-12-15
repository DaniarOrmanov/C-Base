#include <stdio.h>
#include "temp_functions.h" //файл с функциями

#define STR_LEN 20    //длина строки в файле

//разбиение месяцев на группы по количеству дней
#define DAYS_31 ((a >= 1 && a <= 31) && (data[k - 1] == 1 ||\
data[k - 1] == 3 || data[k - 1] == 5 || data[k - 1] == 7 ||\
data[k - 1] == 8 || data[k - 1] == 10 || data[k - 1] == 12))
#define DAYS_30 ((a >= 1 && a <= 30) && (data[k - 1] == 4 ||\
data[k - 1] == 6 || data[k - 1] == 9 || data[k - 1] == 11))
#define DAYS_29 (a >= 1 && a <= 29 && data[k - 1] == 2 && year % 4 == 0)
#define DAYS_28 (a >= 1 && a <= 28 && data[k - 1] == 2)

extern sensor tmp;      //структура для сохранения правильных данных из файла
static short year = 0;  //в эту переменную в первой итерации запишется год.
						//если в дальнейшем встретится другой год, то это будет ошибка

//----------------------------------------------------------------------
//Функция формирует структуру с корректными данными
//также функция сохраняет в файл строки с ошибками
unsigned short file_to_struct(sensor *tmp, FILE *f, FILE *f_error, short num_month)
{
	short *str_data;                    //указатель на массив чисел,\
	                                           сформированный из строки
	unsigned long int row = 0;          //количество строк в структуре без ошибок
	unsigned long int row_full = 0;     //общее количество строк во входном файле
	char str[STR_LEN];                  //строка, считанная из файла
	char simbol;                        //символ, считанный из файла
	int i;                              //счетчик символов в строке
	
	fseek(f, 0, SEEK_SET);           //чтение файла начинаем с начала\
	                                            при смене номера месяца
	while (1) {
		row_full++;                  //счетчик общего количества строк\
		                                                 в файле данных
		if (feof(f) != 0) break;     //проверка на конец файла
		
		//считывем строку и записываем в массив str
		i = 0;
		while (1) {
			fscanf(f, "%c", &simbol);
			if (simbol == '\n' || feof(f) != 0) break;
			str[i] = simbol;
			i++;
			}
			
		//если номер месяца в текущей строке тот, что нам нужен, то
		if ((str[5] == num_month / 10 + 48 || str[5] == ' ') &&\
		                         str[6] == num_month % 10 + 48) {
			// функция проверки символов строки, формирует массив чисел
			str_data = str_examination(str, i, num_month);
			}
		else {             //если номер месяца не тот, то
			continue;	   // переходим к следующей строке файла
			}
		   
		if (*(str_data + 6) == 0) {      //если строчка без ошибок, то\
			                                        заполняем структуру
			tmp->year[row] = str_data[0];
			tmp->month[row] = str_data[1];
			tmp->day[row] = str_data[2];
			tmp->hour[row] = str_data[3];
			tmp->minute[row] = str_data[4];
			tmp->temperature[row] = str_data[5];
			row++;                       //счетчик правильных строк
			}
		else if (*(str_data + 6) == 1) {   //если в строке ошибка, то\
			                        записываем эту строку в файл ошибок
			fprintf(f_error, "Row #%lu - ", row_full);
			for (int j = 0; j < i; j++) {
				fprintf(f_error, "%c", str[j]);
				}
			fprintf(f_error, "\n");
			}
		}
	
	return row;   //возвращаем количество строк в структуре
}
//----------------------------------------------------------------------
//функция принимает на вход строку и ее длину.
//На выходе формирует массив из шести чисел и седьмой флаг ошибки
short* str_examination(char *str, short number, short num_month) {
	short sign = 1;  //отвечает за знак + или - у значения температуры
	short k = 0;  //счетчик в массиве "data"
	short a = 0;  //числовое значение из строки данных
	short count_delim = 0;     //счетчик разделителей в строке
	static short data[7] = {0};   //массив числовых значений в строке
	
	data[6] = 0;   //последний символ в массиве отвечает за ошибку в строке
	for (int i = 0; i < number; i++) {  //пробегаем по строке
		
		switch (i) {   //проверяем каждый символ строки данных
			case 0: case 1: case 2: case 3: case 5: case 6: case 8:\
			case 9: case 11: case 12: case 14: case 15: case 18:\
			case 19: case 20:
				
				if (str[i] >= '0' && str[i] <= '9') {     //если цифра, то
					a *= 10;                              
					a += str[i] - 48;
					}
				else if (str[i] == ' ' && (i == 5 || i == 8 ||\
				i == 11 || i == 14)) {   //если пробелы на определенных\
					                              местах, то игнорируем
					continue;
					}
				else {
					data[6] = 1;           //флаг ошибки строки равен 1
					break;
					}
				
				if (i == number - 1 && (a >= 0 && a <= 99)) { //если\
					                         символ последний в строке,\
					                         и модуль значения температуры\
					                         в диапазоне, и количество\
					                         разделителей равно 5, то
					data[k] = sign * a;     //вносим в массив значение
					a = 0;
					k++;
					sign = 1;
					}
				break;
			case 4: case 7: case 10: case 13: case 16:
				if (str[i] == ';') {                //если разделитель
					count_delim++;
					if (i == 4 && (a >= 1900 && a <= 2100) && year == 0) {  //если\
						                   первый разделитель, год в\
						                   диапазоне и это первая строчка, то
						data[k] = a;      //вносим в массив значение
						year = data[k];   //запоминаем год из первой строки
						a = 0;
						k++;
						}
					else if (i == 4 && a == year && year != 0) { //если\
						                   первый разделитель, год равен\
						                   году из первой строки, то
						data[k] = a;       //вносим в массив значение
						a = 0;
						k++;	
						}
					else if (i == 7 && (a == num_month)) { //если второй\
						                   разделитель, месяц равен текущему, то
						data[k] = a;       //вносим в массив значение
						a = 0;
						k++;	
						}
					else if (i == 7 && (a != num_month) &&\
					                 (a >= 1 && a <= 12)) {  //если второй\
										   разделитель, месяц не равен\
										   текущему, но месяц в диапазоне, то
						data[6] = 2;       //флаг ошибки строки равен 2\
						                   (значит. что строчку мы уже\
						                   обрабатывали). В функции выше\
						                   мы ее просто пропустим
						goto A;            //идем в самый конец этой функции
						}
					else if (i == 10 && DAYS_31) {  //если третий разделитель,\
						                            день в диапазоне до 31, то
						data[k] = a;                //вносим в массив значение
						a = 0;
						k++;	
						}
					else if (i == 10 && DAYS_30) {  //если третий разделитель,\
						                            день в диапазоне до 30, то
						data[k] = a;                //вносим в массив значение
						a = 0;
						k++;	
						}
					else if (i == 10 && DAYS_29) {  //если третий разделитель,\
						                            день в диапазоне до 29, то
						data[k] = a;                //вносим в массив значение
						a = 0;
						k++;	
						}
					else if (i == 10 && DAYS_28) {  //если третий разделитель,\
						                            день в диапазоне до 28, то
						data[k] = a;                //вносим в массив значение
						a = 0;
						k++;	
						}
					else if (i == 13 && (a >= 0 && a <= 23)) { //если четвертый\
						                            разделитель, час в диапазоне , то
						data[k] = a;                //вносим в массив значение
						a = 0;
						k++;	
						}
					else if (i == 16 && (a >= 0 && a <= 59)) { //если пятый\
						                            разделитель, минута в диапазоне , то
						data[k] = a;                //вносим в массив значение
						a = 0;
						k++;	
						}
					else {
						data[6] = 1;                //флаг ошибки строки равен 1
						break;
						}
					if (i == number - 1) {          //если символ последний в строке, то
						data[6] = 1;                //флаг ошибки строки равен 1
						break;
						}
					}
				else {
					data[6] = 1;                    //флаг ошибки строки равен 1
					break;
					}
				break;
			case 17:
				if (str[i] == '-' && number > 17) { //если минус, но есть еще\
					                                сивмолы, то число отрицательное
					sign = -1;                          
					}  
				else if (str[i] >= '0' && str[i] <= '9') { //если цифра, то
					a = str[i] - 48;
					}
				else if (str[i] == ' ' && number > 17) {
					continue;
					}
				else {
					data[6] = 1;                     //флаг ошибки строки равен 1
					break;
					}
				
				if (i == number - 1) {        //если символ последний в строке,\
					                          и модуль значения температуры в\
					                          диапазоне, и количество\
					                          разделителей равно 5, то
					data[k] = sign * a;       //вносим в массив значение
					a = 0;
					k++;
					sign = 1;
					}
				break;
			default:
				data[6] = 1;                //флаг ошибки строки равен 1
			}
		}
	
	if (count_delim != 5) {          //если разделителей не 5 штук, то
		data[6] = 1;                 //флаг ошибки строки равен 1
		}
		
A:	return data;	//возвращаем адрес массива числовых данных
	}
//----------------------------------------------------------------------	
//Функция обрабатывает ключ "-m"	
int if_m(char key, char* optarg) {	
	
	int opt_len = 0;  //длина строки со значением после ключа "-m"
	int a = 0;        //числовое значение после ключа "-m"
	int month_number = 0;   //номер месяца
		
	opt_len = sizeof(optarg) / sizeof(optarg[0]); //находим длину строки после "m"
	for (int i = 0; i < opt_len; i++) {     //цикл по символам строки "optarg"
		if (optarg[i] >= '0' && optarg[i] <= '9') {  //если символ - это цифра,\
			                           то переводим посимвольно в число
			a *= 10;
			a += optarg[i] - 48;
			}
		else break;
		}
	if (a >= 1 && a <= 12) {   //если получившееся число в диапазоне, то
		month_number = a;      //записываем это число как номер месяца
		}
	else {                               //если не в диапазоне, то
		month_number = -1;               //-1 - значит ошибка введенного\
		                                  значения после ключа -m                
		}
	return month_number;         //возвращаем номер месяца
	}
//----------------------------------------------------------------------	
//Функция обрабатывает ключ "-f"	
char* if_f(char key, char* optarg) {
	
	FILE *f;             //указатель на переменную типа "файл"
	int opt_len = 0;     //длина строки после ключа "\f"  
	short file_found;    //Флаг того. что файл найден и его расширение верно
	short count;         //счетчик совпавших символов расширения файла
	static char file_name[20] = {0};        //строка для названия файла
	char ref_ext[] = ".csv";          //образец расширения файла
	short len_ext = sizeof(ref_ext) / sizeof(ref_ext[0]); //длина расширения файла
	
	opt_len = sizeof(optarg) / sizeof(optarg[0]);  //находим длину\
	                                                  строки после "m"
	//Пробегаем по 3 длинам массива optarg(optarg вмещает только 8\
	символов. Умножаем на 3, чтобы название файла, которое мы проверяем,\
	                                       могло состоять из 24 символа)
	int i;
	for (i = 0; i < opt_len * 3; i++) {    //записываем в file_name считанное\
		                                                        название файла
		if (optarg[i] != '\0') {
			file_name[i] = optarg[i];
			}
		else break;
		}
	if (i < 5) {           //если название состоит из 4 символов, то ошибка,\
		                   т.к. расширение с точной - это уже 4 символа
		file_name[0] = '|';
		goto A;
		}
		
	int k = len_ext - 2;  //счетчик символов в образце расширения

	for (int j = i - 1; j > i - len_ext; j--) {   //сравниваем расширеней\
		                                    введенного файла с образцом
		if (file_name[j] == ref_ext[k]) {   //если текущий символ расширения\
			                                         равен образцу, то
			count++;
			k--;
			}
		else {
			count = 0;
			break;
			}
		}
		
	if (count == len_ext - 1) {      //если совпали все символы расширения, то
		if ((f = fopen(file_name, "r")) == 0) {    //если файл не найден
			file_name[0] = ':';         //первый символ массива = ":"
			}
		else {
			file_found = 1;           // файл открылся нормально
			fclose(f);                //закрываем файл
			}
		}
	else {
		file_name[0] = '|';           //первый символ массива = "|"
		}
	
A:	return file_name;                 //возвращаем название файла
	}
//----------------------------------------------------------------------
//функция выводит комментарии перед началом обработки файла	
short comments(short flag_enter, short flag_f, short month_number,\
char* ref_ext, char* p_name, char* month_name) {
	
	if (flag_enter == 0) {       //если был введен неизвестный ключ, или\
		                              ключи вообще не были введены, то
		printf("\nUnknown keys have been entered.\
\n-h - description of the application functionality\
\n-f <filename.%s> - input file for processing\
\n-m <month number> - if you want to see the data of only one month\n", ref_ext);
		goto A;
		}
	else {                      //если все введенные ключи верны, но есть\
		                                      ошибочные значения ключей
		if (flag_f == 0) {      //если имя файла до сих пор не вводилось
			printf("\nThe file was not entered.");
			goto A;
			}
		else if (flag_f == 1) {
			printf("\nThe file format is incorrect. The format must be\
			                                          '%s'.", ref_ext);
			goto A;
			}
		else if (flag_f == 2) {
			printf("\nThe entered file was not found.");
			goto A;
			}
		else if (flag_f == 3) {
			printf("\nOpen a file named '%s'.", p_name);
			}
			
		if (month_number == -2){
			month_number = 0;
			printf("\nThe month number was not entered.");
			}	
		else if (month_number == -1) {              //если номер месяца\
			                                        введен с ошибкой
			month_number = 0;
			printf("\nMonth is entered incorrectly!");
			}
		else if (month_number == 0) {               //если номер месяца\
			                                        не был введен
			printf("\nThe month number was not entered.");
			}	
			
		if (flag_f == 3 && month_number == 0) {     //если корректное имя,\
			                                        и расчет ведем за весь год
			printf("\nThe program will calculate for the whole year.\n");
			}
		else if (flag_f == 3 && month_number >= 1 &&\
		                        month_number <= 12) { //если корректное имя,\
									   и расчет ведем за выбранный месяц
			printf("\nThe program will calculate for %s.\n", month_name);
			}
		}
	
A:	return month_number;                      //возвращаем номер месяца
	}
