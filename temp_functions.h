#define NUM_VALUE 45000    // количество минут в одном месяце (количество строк в структуре)

typedef struct sensor{           //структура для правильных данных из файла
		short year[NUM_VALUE];
		short month[NUM_VALUE];
		short day[NUM_VALUE];
		short hour[NUM_VALUE];
		short minute[NUM_VALUE];
		short temperature[NUM_VALUE];
		}sensor;

unsigned short file_to_struct(sensor*, FILE*, FILE*, short);  //функция считывает данные\
														из файла и сохраняет в структуру.\
														строки с ошибками записывает в файл ошибок
short* str_examination(char*, short, short);          //функция проверяет строку из файла на ошибки
int if_m(char, char*);       //функция обрабатывает ключ "-m"
char* if_f(char, char*);     //функция обрабатывает ключ "-f"
short comments(short, short, short, char*, char*, char*);  //функция выводит комментарии\
													перед началом обработки файла
