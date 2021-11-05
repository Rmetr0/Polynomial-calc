#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <locale.h>

#define N 200 //Максимальное количество символов в expr

char expr[N]; //Считываемые с клавиатуры символы

typedef struct //Структура скобки
{
	int num; //Номер скобки
	int ord; //Порядок выполнения скобки (чем больше - тем первее)
} bracket;

typedef struct //Струкртура переменной
{
	bracket* br; //Указатель на скобку (если NULL - простая переменная, иначе - коэф-т перед скобкой)
	float cft; //Коэф-т (значение)
	int pwr; //Степень
	char sgn; //Знак (если коэф-т отрицательный, знак +)
} variable;

variable res[N * N / 4][N * N / 4]; //Массив считанного в структуру выражения [по скобкам][по переменным], [0][...] - исходное выражение
variable tmp[N * N / 4], mod_tmp[N / 2]; //Массив для временной записи результата действия; Массив для временной записи остатка от деления
bracket br[N * N / 4]; //Массив скобок (номера смещены на 1 назад относительно массива res)
int br_cnt = 0, cur_br = 0; //Счётчик скобок; Номер текущей скобки (при считывании из строки в структуру)
int res_cnt[N * N / 4], tmp_cnt, mod_cnt; //Массив количеств переменных в каждой скобке; Счётчик переменных в результате действий; Счётчик переменных в остатке от деления
int act_cnt = 0; //Счётчик действий

//Функция, убирающая пробелы из введённого с клав-ры выражения
void remove_spaces()
{
	//От 0 до конца массива
	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//Пока текущий символ - пробел
		while (expr[i] == ' ')
		{
			//От текущего символа до конца
			for (int j = i; expr[j] != '\0'; ++j)
			{
				//Смещение следующего символа в текущий
				expr[j] = expr[j + 1];
			}
		}
	}
}

//Проверка правильности ввода скобок
int check_brackets()
{
	//Счётчик открывающих и закрывающих скобок
	int left_br = 0, right_br = 0;

	//От 0 до конца массива
	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//Если символ ( увел. счётчик
		if (expr[i] == '(')
			left_br++;
		//Если символ ) увел. счётчик
		if (expr[i] == ')')
			right_br++;

		//Если в какой-либо момент закрывающих скобок оказалось больше открывающих - выход и возврат 1 (неправильный ввод скобок)
		if (right_br > left_br)
			return 1;
	}
	//Если количество скобок не равно - выход с 1
	if (right_br != left_br)
		return 1;

	//Иначе выход с 0 (всё введено правильно)
	return 0;
}

//Проверка степеней
int check_powers()
{
	//Буфер для номера символа; Флаг содержания числа в степени
	int n, contains_number;

	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//Если текущий символ - знак степени
		if (expr[i] == '^')
		{
			//Если символ введён в начале строки ИЛИ в конце строки - выход с 1 (неправильный ввод)
			if (i == 0 || expr[i + 1] == '\0')
				return 1;

			//Если символ перед знаком степени - не x - выход с 1
			if (expr[i - 1] != 'x')
				return 1;

			//Запись номера текущего символа в n
			n = i;
			//Переход к следующем символу
			i++;
			//Обнуление флага
			contains_number = 0;
			//Пока символы - числа
			while (expr[i] >= '0' && expr[i] <= '9')
			{
				//Содержится число
				contains_number = 1;
				//Переход к следующему
				i++;
				//Если встречается х или точка - выход с 1
				if (expr[i] == 'x' || expr[i] == '.')
					return 1;
			} 

			//Если по результату проверки не содержит числа - выход с 1
			if (!contains_number)
				return 1;
			//Возврат в i в n для последующей проверки строки
			i = n;
		}
	}
	//Иначе выход с 0
	return 0;
}

//Проверка знаков действий
int check_signs()
{
	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//Если + или -
		if (expr[i] == '+' || expr[i] == '-')
		{
			//Введены в конце строки
			if (expr[i + 1] == '\0')
				return 1;

			//После них введены другие знаки действий
			if (expr[i + 1] == '+' || expr[i + 1] == '-' || expr[i + 1] == '*' || expr[i + 1] == '/')
				return 1;
		}
		//Если * или /
		else if (expr[i] == '*' || expr[i] == '/')
		{
			//Введены в начале или конце строки
			if (i == 0 || expr[i + 1] == '\0')
				return 1;

			//Введены перед другими знаками кроме -
			if (expr[i + 1] == '*' || expr[i + 1] == '/' || expr[i + 1] == '+')
				return 1;
		}
		//Если х
		else if (expr[i] == 'x')
		{
			//И следом за ним х
			if (expr[i + 1] == 'x')
				return 1;
		}
		//Если точка
		else if (expr[i] == '.')
		{
			//В начале или конце строки
			if (i == 0 || expr[i + 1] == '\0')
				return 1;

			//Перед ней не число
			if (expr[i - 1] < '0' || expr[i - 1] > '9')
				return 1;

			//После неё не число
			if (expr[i + 1] < '0' || expr[i + 1] > '9')
				return 1;
		}
		//Если закр. скобка
		else if (expr[i] == ')')
		{
			//После неё число
			if (expr[i + 1] >= '0' && expr[i + 1] <= '9')
				return 1;

			//После неё откр. скобка
			if (expr[i + 1] == '(')
				return 1;
		}
	}
	//Иначе выход с 0
	return 0;
}

//Проверка допустимых символов
int check_characters()
{
	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//Если встретился символ, который не является ни одним из этих
		if (
			expr[i] != '(' &&
			expr[i] != ')' &&
			expr[i] != 'x' &&
			expr[i] != '.' &&
			expr[i] != '^' &&
			expr[i] != '+' &&
			expr[i] != '-' &&
			expr[i] != '*' &&
			expr[i] != '/' &&
			(expr[i] < '0' || expr[i] > '9')
			)
			return 1;
	}

	return 0;
}

//Проверка выражения
int check_expression()
{
	//Удаление пробелов
	remove_spaces();

	//Вышеописанные проверки
	if (check_brackets())
	{
		printf("\nОшибка! Неправильная постановка скобок\n");
		system("pause");
		//Возврат с 0 - в correct в main вернётся 0 - цикл повторится - будет повторный ввод
		return 0;
	}

	if (check_powers())
	{
		printf("\nОшибка! Неправильное значение степени\n");
		system("pause");
		return 0;
	}

	if (check_signs())
	{
		printf("\nОшибка! Неправильная постановка знаков действий\n");
		system("pause");
		return 0;
	}

	if (check_characters())
	{
		printf("\nОшибка! Введены недопустимые символы\n");
		system("pause");
		return 0;
	}

	//Иначе возврат с 1 - в correct в main вернётся 1 - цикл ввода прекратится
	return 1;
}

//Инициализация новой переменной
void init_var(float* intg, float* flt)
{
	//Текущая скобка; Текущая переменная
	int* b = &cur_br, * r = &res_cnt[cur_br];

	//Если знак не определён - знак +
	if (res[*b][*r].sgn == '\0')
		res[*b][*r].sgn = '+';

	//Пока дробная часть >= 1, деление её на 10
	while (*flt >= 1)
		*flt /= 10;
	//Коэф-т - сумма целой и дробной части
	res[*b][*r].cft = *intg + *flt;

	//Обнуление целой и дробной части для дальнейших вычислений
	*intg = *flt = 0;
	//Переход к следующей переменной
	++(*r);
}

//Чтение выражения и запись в массив структур res
void read_expression()
{
	//Текущая скобка; Текущая переменная
	int* b = &cur_br, * r = &res_cnt[cur_br];
	//Счётчик левых скобок; Порядок скобки; Массив-стек открытых скобок; Номер последней открытой скобки
	int left_br = 0, ord = 0, open_br[N / 2], open_i = 0;
	//Флаг записи степени; Флаг записи дробной части; Флаг знака числв
	int pwr_flag = 0, flt_flag = 0, neg_flag = 1;
	//Целая и дробная часть
	float intg = 0, flt = 0;

	//Номер первой открытой скобки равен 0
	open_br[0] = 0;
	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//Если откр. скобка
		if (expr[i] == '(')
		{
			//Если в начале строки или перед скобкой стоит не число
			if (i == 0 || expr[i - 1] < '0' || expr[i - 1] > '9')
			{
				//Целая часть = 1
				intg = 1;
			}

			//Запись порядка и номера скобки
			br[left_br].ord = ord;
			br[left_br].num = left_br;
			//Определение указателя на эту скобку в текущей переменной
			res[*b][*r].br = &br[left_br];

			//Обнуление флагов
			pwr_flag = flt_flag = 0;
			//Домножение на знак
			intg *= neg_flag;
			//Сброс знака в положительный
			neg_flag = 1;
			//Инициализация переменной
			init_var(&intg, &flt);

			//Увеличение счётчика откр. скобки, номера последней открытой
			++left_br;
			++open_i;
			//Запись номера открытой скобки
			open_br[open_i] = left_br;
			//Запись новой текущей скобки и новой текущей переменной
			*b = open_br[open_i];
			r = &res_cnt[*b];
			//Увеличение порядка скобки
			++ord;

			//Запись знака +
			res[*b][*r].sgn = '+';
		}
		//Если закр. скобка
		else if (expr[i] == ')')
		{
			//Если перед этим была закр. скобка
			if (expr[i - 1] != ')')
			{
				//Обнуление флагов, домножение на знак
				pwr_flag = flt_flag = 0;
				intg *= neg_flag;
				neg_flag = 1;
				//Иниц. новой переменной
				init_var(&intg, &flt);
			}

			//Уменьшение номера открытой скобки
			--open_i;
			//Запись новых текущей скобки и текущей переменной
			*b = open_br[open_i];
			r = &res_cnt[*b];
			//Уменьшение порядка скобки
			--ord;
		}
		//Если число
		else if (expr[i] >= '0' && expr[i] <= '9')
		{
			//Если флаг запись степени
			if (pwr_flag)
			{
				//Домножение на 10 и сложение с числом (- '0' - перевод char символа в число int)
				res[*b][*r].pwr *= 10;
				res[*b][*r].pwr += expr[i] - '0';
			}
			//Если флаг записи дробной части
			else if (flt_flag)
			{
				flt *= 10;
				flt += expr[i] - '0';
			}
			//Иначе если никаких флагов нет - запись целой части
			else
			{
				intg *= 10;
				intg += expr[i] - '0';
			}
		}
		//Если х
		else if (expr[i] == 'x')
		{
			//Если в начале строки или перед ним не число
			if (i == 0 || expr[i - 1] < '0' || expr[i - 1] > '9')
			{
				//Целая часть равна 1
				intg = 1;
			}
			//Степень равна 1 (если дальше она не будет обозначена)
			res[*b][*r].pwr = 1;
		}
		//Если знак степени
		else if (expr[i] == '^')
		{
			//Степень равна 0 (если всё-таки обозначена)
			res[*b][*r].pwr = 0;
			//Флаг записи степени
			pwr_flag = 1;
		}
		//Если точка
		else if (expr[i] == '.')
		{
			//Флаг записи дробной части
			flt_flag = 1;
		}
		//Если знак действия
		else if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/')
		{
			//Если не в начале строки И перед знаком стоит * или /
			if (i != 0 && (expr[i - 1] == '*' || expr[i - 1] == '/'))
			{
				//Запись отрицательного коэф-та
				neg_flag = -1;
			}
			//Иначе
			else
			{
				//Если не в начале строки и перед ним стоит закр. скобка
				if (i != 0 && expr[i - 1] != ')')
				{
					//Обнуление, домножение, инициализация новой перем.
					pwr_flag = flt_flag = 0;
					intg *= neg_flag;
					neg_flag = 1;
					init_var(&intg, &flt);
				}
				
				//Если знак - минус
				if (expr[i] == '-')
				{
					//Запись отриц. коэф-та, знак коэф-та - плюс
					neg_flag = -1;
					res[*b][*r].sgn = '+';
				}
				//Иначе
				else
					//Запись считанного знака
					res[*b][*r].sgn = expr[i];
			}
		}
	}
	//Если целия и/или дробная части остались ненулевые
	if (intg != 0 || flt != 0)
	{
		//Домножение на знак, дозапись частей в последнюю переменную
		intg *= neg_flag;
		while (flt >= 1)
			flt /= 10;
		res[*b][*r].cft = intg + flt;
		++(*r);
	}

	//Кол-во скобок равно кол-ву открывающих скобок
	br_cnt = left_br;
}

//Вывод выражения на экран
void output_expression(variable* var, int cnt)
{
	//От 0 до переданного количества
	for (int i = 0; i < cnt; ++i)
	{
		//Если коэф-т равен 0 - вывод нуля
		if (var[i].cft == 0)
		{
			printf("0");
		}
		//Иначе
		else
		{
			//Если первый коэф-т
			if (i == 0)
			{
				//Если отрицательный - вывод минуса без пробелов
				if (var[i].cft < 0)
					printf("-");
			}
			//Иначе
			else
			{
				//Если отрицательный
				if (var[i].cft < 0)
				{
					//Знак - плюс - вывод минуса с пробелами
					if (var[i].sgn == '+')
						printf(" %c ", '-');
					//Иначе вывод знака с пробелами и минуса без пробела
					else
						printf(" %c -", var[i].sgn);
				}
				//Иначе вывод знака с пробелами без минуса
				else
					printf(" %c ", var[i].sgn);
			}

			//Если коэф-т отрицательный
			if (var[i].cft < 0)
				//Вывод -коэф-та
				printf("%.2f", -var[i].cft);
			else
				//Иначе вывод коэф-та
				printf("%.2f", var[i].cft);

			//Если указатель скобки ненулевой
			if (var[i].br != NULL)
			{
				//Вывод скобок и выражения по номеру скобки + 1 между ними
				printf(" (");
				output_expression(res[var[i].br->num + 1], res_cnt[var[i].br->num + 1]);
				printf(")");
			}
			//Иначе (скобок нет)
			else
			{
				//Степень = 1 - вывод просто х
				if (var[i].pwr == 1)
					printf(" x");
				//Иначе если степень не 0 - вывод x со степенью
				else if (var[i].pwr != 0)
					printf(" x^%d", var[i].pwr);
			}
		}
	}
}

//Установка значений переменной
void set_var(variable* var1, variable* var2)
{
	//Из var2 записываются в var1
	var1->br = var2->br;
	var1->cft = var2->cft;
	var1->pwr = var2->pwr;
	var1->sgn = var2->sgn;
}

//Сортировка выражения в скобке по убыванию степеней
void sort_bracket(variable* var, int cnt)
{
	//Временная переменная
	variable tmp;
	//Флаг наличия перестановок
	int flag = 1;

	//Пока есть перестановки - проверять выражение
	while (flag)
	{
		//Обнул. флага
		flag = 0;
		//От 0 до переданного кол-ва переменных в скобке - 1
		for (int i = 0; i < cnt - 1; ++i)
		{
			//Если степень текущей перем. меньше степени следующей перем.
			if (var[i].pwr < var[i + 1].pwr)
			{
				//Меняем местами текущую и следующую через tmp
				set_var(&tmp, &var[i]);
				set_var(&var[i], &var[i + 1]);
				set_var(&var[i + 1], &tmp);

				//Перестановка была
				flag = 1;
			}
		}
	}
}

//Сложение переменных с одинаковыми степенями в отсортированном выражении
void quick_add(variable* var, int* cnt)
{
	//От 0 до кол-ва переменных - 1
	for (int i = 0; i < *cnt - 1; ++i)
	{
		//Если коэф-т равен 0
		if (var[i].cft == 0)
		{
			//От текущего до конца - 1
			for (int j = i; j < *cnt - 1; ++j)
			{
				//Запись из следующей переменной в текущую (смещение на 1 в начало)
				set_var(&var[j], &var[j + 1]);
			}
			//Уменьшение кол-ва переменных
			--(*cnt);
			//Смещение на 1 переменную назад (Необходимо проверить ещё раз)
			--i;
			//Следующий цикл проверки (в начало for)
			continue;
		}
		//Если степени текущей и следующей переменных равны
		if (var[i].pwr == var[i + 1].pwr)
		{
			//Сложение коэф-тов
			var[i].cft += var[i + 1].cft;
			//От следующей переменной до конца - 1
			for (int j = i + 1; j < *cnt - 1; ++j)
			{
				//Смещение на 1 в начало
				set_var(&var[j], &var[j + 1]);
			}
			//Уменьшение кол-ва, смещение на 1 назад
			--(*cnt);
			--i;
		}
	}
}

//Деление
void divide(variable* var1, variable* var2, int* cnt1, int* cnt2)
{
	//Массив для разности в промежуточных действиях
	variable dif_tmp[N * N / 4];
	//Счётчик кол-ва переменных в разности
	int dif_cnt;
	//Обнуления счётчика переменных в результате действия
	tmp_cnt = 0;

	//Если старшая степень делимого не меньше старшей степени
	if (var1[0].pwr >= var2[0].pwr)
	{
		do
		{
			//Степень очередной переменной результата деления - вычитание степени делимого из степени делителя
			tmp[tmp_cnt].pwr = var1[0].pwr - var2[0].pwr;
			//Коэф-т - деление
			tmp[tmp_cnt].cft = var1[0].cft / var2[0].cft;
			tmp[tmp_cnt].sgn = '+';

			//Обнуление счётчика остатка и запись в его степень - сумм степени переменной результата и степени текущей переменной делителя
			//А в коэф-т произведения коэф-та переменной результата и коэф-та текущей переменной делителя
			mod_cnt = 0;
			for (int i = 0; i < *cnt2; i++)
			{
				mod_tmp[i].pwr = tmp[tmp_cnt].pwr + var2[i].pwr;
				mod_tmp[i].cft = tmp[tmp_cnt].cft * var2[i].cft;
				mod_tmp[i].sgn = '+';
				++mod_cnt;
			}
			//В дальнейшем - запись следующей переменной результата деления
			++tmp_cnt;

			//Запись в массив разности промежуточного действия
			//Сперва коэф-ты и степени из делимого
			//Потом коэф-ты со знаком минус и степени из остатка
			dif_cnt = 0;
			for (int i = 1; i < *cnt1; i++)
			{
				dif_tmp[dif_cnt].cft = var1[i].cft;
				dif_tmp[dif_cnt].pwr = var1[i].pwr;
				++dif_cnt;
			}
			for (int i = 1; i < mod_cnt; i++)
			{
				dif_tmp[dif_cnt].cft = -mod_tmp[i].cft;
				dif_tmp[dif_cnt].pwr = mod_tmp[i].pwr;
				++dif_cnt;
			}

			//Сортировка и сложение одинаковых степеней разности
			sort_bracket(dif_tmp, dif_cnt);
			quick_add(dif_tmp, &dif_cnt);

			//Запись в делимое полученной разности для дальнейших действий
			for (int i = 0; i < dif_cnt; ++i)
			{
				var1[i].cft = dif_tmp[i].cft;
				var1[i].pwr = dif_tmp[i].pwr;
			}
			*cnt1 = dif_cnt;

			//Запись в остаток полученной разности
			for (int i = 0; i < *cnt1; ++i)
			{
				mod_tmp[i].cft = var1[i].cft;
				mod_tmp[i].pwr = var1[i].pwr;
			}
			mod_cnt = *cnt1;

		//Пока старшая степень остатка не меньше старшей степени делителя
		} while (mod_tmp[0].pwr >= var2[0].pwr);
	}
	//Иначе если старшая степень делимого меньше, чем у делителя
	else
	{
		//Будет одна переменная с коэф-том 0
		++tmp_cnt;
		tmp[0].cft = 0;
		for (int i = 0; i < *cnt1; i++)
		{
			//Запись в остаток делимого
			set_var(&mod_tmp[i], &var1[i]);
			//Затирание делимого нулями
			var1[i].cft = 0;
		}
		//Счётчик отстатка равен счётчику делимого
		mod_cnt = *cnt1;
		//Счётчик делимого равен нулю
		*cnt1 = 0;
	}
}

//Умножение
void multiply(variable* var1, variable* var2, int* cnt1, int* cnt2)
{
	//Обнуления счётчика переменных в результате действия
	tmp_cnt = 0;

	//От 0 до кол-ва перем. в первом выражении
	for (int i = 0; i < *cnt1; ++i)
	{
		//От 0 до кол-ва перем. во втором выражении
		for (int j = 0; j < *cnt2; ++j)
		{
			//Указатель скобки и знак - значения по умолчанию
			//Коэф-т - произведение, степень - сумма 
			tmp[tmp_cnt].br = NULL;
			tmp[tmp_cnt].cft = var1[i].cft * var2[j].cft;
			tmp[tmp_cnt].pwr = var1[i].pwr + var2[j].pwr;
			tmp[tmp_cnt].sgn = '+';

			//Увел. счётчика перем. в произведении
			++tmp_cnt;
		}
	}
}

//Сложение/вычитание
void add_sub(variable* var1, variable* var2, int* cnt1, int* cnt2)
{
	//Текущая степень
	int cur_pwr;
	//Обнуления счётчика переменных в результате действия
	tmp_cnt = 0;

	//От 0 до размера наибольнего выражения из слагаемых
	for (int i = 0; i < max(*cnt1, *cnt2); ++i)
	{
		//Если коэф-т текущей переменной первого слагаемого не ноль И номер элемента не вышел за пределы первого слагаемого 
		if (var1[i].cft != 0 && i < *cnt1)
		{
			//Текущая степень равна степени текущей переменной первого слагаемого
			cur_pwr = var1[i].pwr;

			//Инициализация переменной в массиве результата сложения
			tmp[tmp_cnt].br = NULL;
			tmp[tmp_cnt].cft = 0;
			tmp[tmp_cnt].pwr = cur_pwr;
			tmp[tmp_cnt].sgn = '+';

			//От текущего элемента до максимального размера выражения
			for (int j = i; j < max(*cnt1, *cnt2); ++j)
			{
				//Если номер не вышел за пределы первого слагаемого И степень текущей переменной первого слагаемого равна текущей степени
				if (j < *cnt1 && var1[j].pwr == cur_pwr)
				{
					//Складывание степеней и обнуление во избежание повторного сложения
					tmp[tmp_cnt].cft += var1[j].cft;
					var1[j].cft = 0;
				}
				//Аналогично для второго слагаемого
				if (j < *cnt2 && var2[j].pwr == cur_pwr)
				{
					tmp[tmp_cnt].cft += var2[j].cft;
					var2[j].cft = 0;
				}
			}
			
			//Если итоговый коэф-т не стал равным нулю - запись следующей результирующей переменной
			if (tmp[tmp_cnt].cft != 0)
				++tmp_cnt;
		}
		//Аналогично для второго слагаемого
		if (var2[i].cft != 0 && i < *cnt2)
		{
			cur_pwr = var2[i].pwr;

			tmp[tmp_cnt].br = NULL;
			tmp[tmp_cnt].cft = 0;
			tmp[tmp_cnt].pwr = cur_pwr;
			tmp[tmp_cnt].sgn = '+';

			for (int j = i; j < max(*cnt1, *cnt2); ++j)
			{
				if (j < *cnt1 && var1[j].pwr == cur_pwr)
				{
					tmp[tmp_cnt].cft += var1[j].cft;
					var1[j].cft = 0;
				}
				if (j < *cnt2 && var2[j].pwr == cur_pwr)
				{
					tmp[tmp_cnt].cft += var2[j].cft;
					var2[j].cft = 0;
				}
			}

			if (tmp[tmp_cnt].cft != 0)
				++tmp_cnt;
		}
	}
}

//Поиск делений, аналогично поиску умножений и сложений/вычитаний (см. ниже)
void if_div(int n)
{
	variable* var1, * var2;
	int one = 1;

	for (int i = 1; i < res_cnt[n]; ++i)
	{
		if (res[n][i].sgn == '/')
		{
			if (res[n][i].br == NULL)
			{
				if (res[n][i - 1].br == NULL)
				{
					var1 = &res[n][i - 1];
					var2 = &res[n][i];
					++act_cnt;

					printf("\n Действие %d:\n\n", act_cnt);
					printf("  %.2f x^%d / %.2f x^%d = ", var1->cft, var1->pwr, var2->cft, var2->pwr);

					divide(var1, var2, &one, &one);
				}
				else
				{
					var1 = res[res[n][i - 1].br->num + 1];
					var2 = &res[n][i];
					++act_cnt;

					printf("\n Действие %d:\n\n", act_cnt);
					printf("  (");
					output_expression(var1, res_cnt[res[n][i - 1].br->num + 1]);
					printf(") / %.2f x^%d = ", var2->cft, var2->pwr);

					divide(var1, var2, &res_cnt[res[n][i - 1].br->num + 1], &one);
				}
			}
			else
			{
				if (res[n][i - 1].br == NULL)
				{
					var1 = &res[n][i - 1];
					var2 = res[res[n][i].br->num + 1];
					++act_cnt;

					printf("\n Действие %d:\n\n", act_cnt);
					printf("  %.2f x^%d / (", var1->cft, var1->pwr);
					output_expression(var2, res_cnt[res[n][i].br->num + 1]);
					printf(") = ");

					divide(var1, var2, &one, &res_cnt[res[n][i].br->num + 1]);
				}
				else
				{
					var1 = res[res[n][i - 1].br->num + 1];
					var2 = res[res[n][i].br->num + 1];
					++act_cnt;

					printf("\n Действие %d:\n\n", act_cnt);
					printf("  (");
					output_expression(var1, res_cnt[res[n][i - 1].br->num + 1]);
					printf(") / (");
					output_expression(var2, res_cnt[res[n][i].br->num + 1]);
					printf(") = ");

					divide(var1, var2, &res_cnt[res[n][i - 1].br->num + 1], &res_cnt[res[n][i].br->num + 1]);
				}
			}

			if (tmp_cnt >= 2)
			{
				br[br_cnt].num = br_cnt;
				br[br_cnt].ord = br[n].ord;
				res[n][i - 1].br = &br[br_cnt];
				res[n][i - 1].cft = 1;
				res[n][i - 1].pwr = 0;
				res_cnt[br_cnt + 1] = tmp_cnt;

				//Нет необходимости сортировки и сложения, т.к. результат деления и так отсортирован

				for (int j = 0; j < tmp_cnt; ++j)
				{
					set_var(&res[br_cnt + 1][j], &tmp[j]);
				}
				++br_cnt;
			}
			else
			{
				set_var(&res[n][i - 1], &tmp[0]);
			}
			for (int j = i + 1; j < res_cnt[n]; ++j)
			{
				set_var(&res[n][j - 1], &res[n][j]);
			}
			--res_cnt[n];
			--i;

			output_expression(tmp, tmp_cnt);
			printf(" (Остаток: ");
			output_expression(mod_tmp, mod_cnt);
			printf(")\n\n");
			system("pause");
		}
	}
}

//Поиск умножений, работает аналогично поиску сложений/вычитаний (см. ниже)
//Доп. условия подписаны
void if_mul(int n)
{
	variable* var1, * var2;
	int one = 1;

	for (int i = 0; i < res_cnt[n]; ++i)
	{
		//Если коэф-т перед скобкой
		if (res[n][i].br != NULL)
		{
			//Не равен 1
			if (res[n][i].cft != 1)
			{
				//Выполнение умножения коэф-та на скобку
				var1 = &res[n][i];
				var2 = res[res[n][i].br->num + 1];
				++act_cnt;

				printf("\n Действие %d:\n\n", act_cnt);
				printf("  %.2f * (", var1->cft);
				output_expression(var2, res_cnt[res[n][i].br->num + 1]);
				printf(") = ");

				multiply(var1, var2, &one, &res_cnt[res[n][i].br->num + 1]);

				//Теперь коэф-т равен 1
				res[n][i].cft = 1;
				for (int j = 0; j < tmp_cnt; ++j)
				{
					//Запись из tmp в res
					set_var(&var2[j], &tmp[j]);
				}

				output_expression(tmp, tmp_cnt);
				printf("\n\n");
				system("pause");
			}
		}

		if (res[n][i].sgn == '*')
		{
			if (res[n][i].br == NULL)
			{
				if (res[n][i - 1].br == NULL)
				{
					var1 = &res[n][i - 1];
					var2 = &res[n][i];
					++act_cnt;

					printf("\n Действие %d:\n\n", act_cnt);
					printf("  %.2f x^%d * %.2f x^%d = ", var1->cft, var1->pwr, var2->cft, var2->pwr);

					multiply(var1, var2, &one, &one);
				}
				else
				{
					var1 = res[res[n][i - 1].br->num + 1];
					var2 = &res[n][i];
					++act_cnt;

					printf("\n Действие %d:\n\n", act_cnt);
					printf("  (");
					output_expression(var1, res_cnt[res[n][i - 1].br->num + 1]);
					printf(") * %.2f x^%d = ", var2->cft, var2->pwr);

					multiply(var1, var2, &res_cnt[res[n][i - 1].br->num + 1], &one);
				}
			}
			else
			{
				if (res[n][i - 1].br == NULL)
				{
					var1 = &res[n][i - 1];
					var2 = res[res[n][i].br->num + 1];
					++act_cnt;

					printf("\n Действие %d:\n\n", act_cnt);
					printf("  %.2f x^%d * (", var1->cft, var1->pwr);
					output_expression(var2, res_cnt[res[n][i].br->num + 1]);
					printf(") = ");

					multiply(var1, var2, &one, &res_cnt[res[n][i].br->num + 1]);
				}
				else
				{
					var1 = res[res[n][i - 1].br->num + 1];
					var2 = res[res[n][i].br->num + 1];
					++act_cnt;

					printf("\n Действие %d:\n\n", act_cnt);
					printf("  (");
					output_expression(var1, res_cnt[res[n][i - 1].br->num + 1]);
					printf(") * (");
					output_expression(var2, res_cnt[res[n][i].br->num + 1]);
					printf(") = ");

					multiply(var1, var2, &res_cnt[res[n][i - 1].br->num + 1], &res_cnt[res[n][i].br->num + 1]);
				}
			}

			//Если в результате выполнения действий от двух переменных
			if (tmp_cnt >= 2)
			{
				//Запись в новую скобку номера и порядка
				br[br_cnt].num = br_cnt;
				br[br_cnt].ord = br[n].ord;
				//Присвоение указателя на новую скобку
				res[n][i - 1].br = &br[br_cnt];
				res[n][i - 1].cft = 1;
				res[n][i - 1].pwr = 0;

				//Сортировка и сложение одинаковых степеней
				sort_bracket(tmp, tmp_cnt);
				quick_add(tmp, &tmp_cnt);
				//Кол-во переменных в новой скобке равно кол-ву переменных в результате действий (отсортированном и сложенном)
				res_cnt[br_cnt + 1] = tmp_cnt;

				for (int j = 0; j < tmp_cnt; ++j)
				{
					//Запись из tmp в res
					set_var(&res[br_cnt + 1][j], &tmp[j]);
				}
				//Увел. кол-ва скобок
				++br_cnt;
			}
			else
			{
				set_var(&res[n][i - 1], &tmp[0]);
			}
			for (int j = i + 1; j < res_cnt[n]; ++j)
			{
				set_var(&res[n][j - 1], &res[n][j]);
			}
			--res_cnt[n];
			--i;

			output_expression(tmp, tmp_cnt);
			printf("\n\n");
			system("pause");
		}
	}
}

//Поиск сложений/вычитаний
void if_add_sub(int n)
{
	//Два слагаемых
	variable* var1, * var2;
	//Единица для передачи по ссылке в функцию
	int one = 1;
	//Увелничение счётчика действий
	++act_cnt;

	//Вывод выполняемого действия
	printf("\n Действие %d:\n\n  ", act_cnt);
	output_expression(res[n], res_cnt[n]);
	printf(" = ");

	//От 1 до кол-ва переменных в выражении
	for (int i = 1; i < res_cnt[n]; ++i)
	{
		//Если знак текущей переменной - плюс
		if (res[n][i].sgn == '+')
		{
			//Если эта переменная - не коэф-т скобки
			if (res[n][i].br == NULL)
			{
				//Если предыдущая переменная - не коэф-т скобки
				if (res[n][i - 1].br == NULL)
				{
					//Обе переменные - одиночные
					var1 = &res[n][i - 1];
					var2 = &res[n][i];

					add_sub(var1, var2, &one, &one);
				}
				//Иначе если коэф-т скобки
				else
				{
					//Сложение скобки и одиночной переменной
					var1 = res[res[n][i - 1].br->num + 1];
					var2 = &res[n][i];

					add_sub(var1, var2, &res_cnt[res[n][i - 1].br->num + 1], &one);
				}
			}
			//Иначе если коэф-т скобки
			else
			{
				//Если предыдущая переменная - не коэф-т скобки
				if (res[n][i - 1].br == NULL)
				{
					//Сложение одиночной переменной и скобки
					var1 = &res[n][i - 1];
					var2 = res[res[n][i].br->num + 1];

					add_sub(var1, var2, &one, &res_cnt[res[n][i].br->num + 1]);
				}
				//Иначе если коэф-т скобки
				else
				{
					//Сложение двух скобок
					var1 = res[res[n][i - 1].br->num + 1];
					var2 = res[res[n][i].br->num + 1];

					add_sub(var1, var2, &res_cnt[res[n][i - 1].br->num + 1], &res_cnt[res[n][i].br->num + 1]);
				}
			}

			//Запись результата сложения в массив res
			//Если результат сложения содержит от двух переменных
			if (tmp_cnt >= 2)
			{
				//От 0 до кол-ва переменных в результате действий -2 (т.к. складывались две переменные)
				for (int j = 0; j < tmp_cnt - 2; ++j)
				{
					//Смещение переменных с конца на кол-во переменных в результате действий -2
					set_var(&res[n][res_cnt[n] - 1 - j + tmp_cnt], &res[n][res_cnt[n] - 1 - j]);
				}
				for (int j = 0; j < tmp_cnt; ++j)
				{
					//Запись из массива результата действий tmp в res
					set_var(&res[n][i - 1 + j], &tmp[j]);
				}
				//Увел. кол-ва переменных на кол-во в результате действий -2
				res_cnt[n] += tmp_cnt - 2;
			}
			//Иначе если содержит одну переменную
			else
			{
				//Запись из tmp в res
				set_var(&res[n][i - 1], &tmp[0]);

				for (int j = i + 1; j < res_cnt[n]; ++j)
				{
					//Смещение на 1 элемент к началу
					set_var(&res[n][j - 1], &res[n][j]);
				}
				//Уменьшение кол-ва на 1 и смещение на 1 элемент назад
				--res_cnt[n];
				--i;
			}
		}
	}
	//Сортировка и сложение одинаковых степеней в итоговом выражении
	sort_bracket(res[n], res_cnt[n]);
	quick_add(res[n], &res_cnt[n]);

	//Вывод итогового выражения на экран
	output_expression(res[n], res_cnt[n]);
	printf("\n\n");
	system("pause");
}

//Обработка выражения в скобке
void proc_bracket(int n)
{
	//Сперва выполнение всех делений, потом умножений, потом сложений/вычитаний
	if_div(n);
	if_mul(n);
	if_add_sub(n);
}

//Обработка целого выражения
void proc_expression()
{
	//Вычисление максимального порядка скобки, чтобы с него начать
	int max_ord = 0;
	for (int i = 0; i < br_cnt; ++i)
	{
		if (br[i].ord > max_ord)
			++max_ord;
	}

	//От максимального порядка до 0
	for (int i = max_ord; i >= 0; --i)
	{
		//От 0 до кол-ва скобок
		for (int j = 0; j < br_cnt; ++j)
		{
			//Если найден нужный порядок - обработка выражения
			if (br[j].ord == i)
				proc_bracket(j + 1);
		}
	}
	//В конце - обработка целого исходного выражения ("нулевое" выражение)
	proc_bracket(0);
}

void main(void)
{
	setlocale(LC_ALL, "Russian");
	int correct = 0;
	
	//Пока correct равно 0
	while (!correct)
	{
		//Очистка массива expr для записи выражения
		memset(expr, 0, N);

		system("cls");
		printf("\n  Введите выражение с полиномами\n\n");
		printf("  - Для обозначение порядка действий допускается использование скобок: ()\n");
		printf("  - Допустимое обозначение переменной: x\n");
		printf("  - Допустимые действия: + - * /\n");
		printf("  - Допустимное обозначение степени переменной: ^\n");
		printf("  - Максимальное количество считываемых символов: %d\n\n  ", N);

		//Считывание выражения
		gets_s(expr, N);
		//Функция check_expression возвращает 0, если есть ошибка во введённом выражении, и 1, если ошибок нет
		correct = check_expression();
	}
	//Чтение выражения и запись в массив структур переменных res
	read_expression();

	//Вывод введённого выражения
	system("cls");
	printf(" Введённое выражение:\n\n  ");
	output_expression(res[0], res_cnt[0]);
	printf("\n\n");
	system("pause");

	//Выполнение действий в выражении
	printf("\n\n");
	proc_expression();
}