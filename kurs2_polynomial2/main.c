#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <locale.h>

#define N 200 //������������ ���������� �������� � expr

char expr[N]; //����������� � ���������� �������

typedef struct //��������� ������
{
	int num; //����� ������
	int ord; //������� ���������� ������ (��� ������ - ��� ������)
} bracket;

typedef struct //���������� ����������
{
	bracket* br; //��������� �� ������ (���� NULL - ������� ����������, ����� - ����-� ����� �������)
	float cft; //����-� (��������)
	int pwr; //�������
	char sgn; //���� (���� ����-� �������������, ���� +)
} variable;

variable res[N * N / 4][N * N / 4]; //������ ���������� � ��������� ��������� [�� �������][�� ����������], [0][...] - �������� ���������
variable tmp[N * N / 4], mod_tmp[N / 2]; //������ ��� ��������� ������ ���������� ��������; ������ ��� ��������� ������ ������� �� �������
bracket br[N * N / 4]; //������ ������ (������ ������� �� 1 ����� ������������ ������� res)
int br_cnt = 0, cur_br = 0; //������� ������; ����� ������� ������ (��� ���������� �� ������ � ���������)
int res_cnt[N * N / 4], tmp_cnt, mod_cnt; //������ ��������� ���������� � ������ ������; ������� ���������� � ���������� ��������; ������� ���������� � ������� �� �������
int act_cnt = 0; //������� ��������

//�������, ��������� ������� �� ��������� � ����-�� ���������
void remove_spaces()
{
	//�� 0 �� ����� �������
	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//���� ������� ������ - ������
		while (expr[i] == ' ')
		{
			//�� �������� ������� �� �����
			for (int j = i; expr[j] != '\0'; ++j)
			{
				//�������� ���������� ������� � �������
				expr[j] = expr[j + 1];
			}
		}
	}
}

//�������� ������������ ����� ������
int check_brackets()
{
	//������� ����������� � ����������� ������
	int left_br = 0, right_br = 0;

	//�� 0 �� ����� �������
	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//���� ������ ( ����. �������
		if (expr[i] == '(')
			left_br++;
		//���� ������ ) ����. �������
		if (expr[i] == ')')
			right_br++;

		//���� � �����-���� ������ ����������� ������ ��������� ������ ����������� - ����� � ������� 1 (������������ ���� ������)
		if (right_br > left_br)
			return 1;
	}
	//���� ���������� ������ �� ����� - ����� � 1
	if (right_br != left_br)
		return 1;

	//����� ����� � 0 (�� ������� ���������)
	return 0;
}

//�������� ��������
int check_powers()
{
	//����� ��� ������ �������; ���� ���������� ����� � �������
	int n, contains_number;

	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//���� ������� ������ - ���� �������
		if (expr[i] == '^')
		{
			//���� ������ ����� � ������ ������ ��� � ����� ������ - ����� � 1 (������������ ����)
			if (i == 0 || expr[i + 1] == '\0')
				return 1;

			//���� ������ ����� ������ ������� - �� x - ����� � 1
			if (expr[i - 1] != 'x')
				return 1;

			//������ ������ �������� ������� � n
			n = i;
			//������� � ��������� �������
			i++;
			//��������� �����
			contains_number = 0;
			//���� ������� - �����
			while (expr[i] >= '0' && expr[i] <= '9')
			{
				//���������� �����
				contains_number = 1;
				//������� � ����������
				i++;
				//���� ����������� � ��� ����� - ����� � 1
				if (expr[i] == 'x' || expr[i] == '.')
					return 1;
			} 

			//���� �� ���������� �������� �� �������� ����� - ����� � 1
			if (!contains_number)
				return 1;
			//������� � i � n ��� ����������� �������� ������
			i = n;
		}
	}
	//����� ����� � 0
	return 0;
}

//�������� ������ ��������
int check_signs()
{
	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//���� + ��� -
		if (expr[i] == '+' || expr[i] == '-')
		{
			//������� � ����� ������
			if (expr[i + 1] == '\0')
				return 1;

			//����� ��� ������� ������ ����� ��������
			if (expr[i + 1] == '+' || expr[i + 1] == '-' || expr[i + 1] == '*' || expr[i + 1] == '/')
				return 1;
		}
		//���� * ��� /
		else if (expr[i] == '*' || expr[i] == '/')
		{
			//������� � ������ ��� ����� ������
			if (i == 0 || expr[i + 1] == '\0')
				return 1;

			//������� ����� ������� ������� ����� -
			if (expr[i + 1] == '*' || expr[i + 1] == '/' || expr[i + 1] == '+')
				return 1;
		}
		//���� �
		else if (expr[i] == 'x')
		{
			//� ������ �� ��� �
			if (expr[i + 1] == 'x')
				return 1;
		}
		//���� �����
		else if (expr[i] == '.')
		{
			//� ������ ��� ����� ������
			if (i == 0 || expr[i + 1] == '\0')
				return 1;

			//����� ��� �� �����
			if (expr[i - 1] < '0' || expr[i - 1] > '9')
				return 1;

			//����� �� �� �����
			if (expr[i + 1] < '0' || expr[i + 1] > '9')
				return 1;
		}
		//���� ����. ������
		else if (expr[i] == ')')
		{
			//����� �� �����
			if (expr[i + 1] >= '0' && expr[i + 1] <= '9')
				return 1;

			//����� �� ����. ������
			if (expr[i + 1] == '(')
				return 1;
		}
	}
	//����� ����� � 0
	return 0;
}

//�������� ���������� ��������
int check_characters()
{
	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//���� ���������� ������, ������� �� �������� �� ����� �� ����
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

//�������� ���������
int check_expression()
{
	//�������� ��������
	remove_spaces();

	//������������� ��������
	if (check_brackets())
	{
		printf("\n������! ������������ ���������� ������\n");
		system("pause");
		//������� � 0 - � correct � main ������� 0 - ���� ���������� - ����� ��������� ����
		return 0;
	}

	if (check_powers())
	{
		printf("\n������! ������������ �������� �������\n");
		system("pause");
		return 0;
	}

	if (check_signs())
	{
		printf("\n������! ������������ ���������� ������ ��������\n");
		system("pause");
		return 0;
	}

	if (check_characters())
	{
		printf("\n������! ������� ������������ �������\n");
		system("pause");
		return 0;
	}

	//����� ������� � 1 - � correct � main ������� 1 - ���� ����� �����������
	return 1;
}

//������������� ����� ����������
void init_var(float* intg, float* flt)
{
	//������� ������; ������� ����������
	int* b = &cur_br, * r = &res_cnt[cur_br];

	//���� ���� �� �������� - ���� +
	if (res[*b][*r].sgn == '\0')
		res[*b][*r].sgn = '+';

	//���� ������� ����� >= 1, ������� � �� 10
	while (*flt >= 1)
		*flt /= 10;
	//����-� - ����� ����� � ������� �����
	res[*b][*r].cft = *intg + *flt;

	//��������� ����� � ������� ����� ��� ���������� ����������
	*intg = *flt = 0;
	//������� � ��������� ����������
	++(*r);
}

//������ ��������� � ������ � ������ �������� res
void read_expression()
{
	//������� ������; ������� ����������
	int* b = &cur_br, * r = &res_cnt[cur_br];
	//������� ����� ������; ������� ������; ������-���� �������� ������; ����� ��������� �������� ������
	int left_br = 0, ord = 0, open_br[N / 2], open_i = 0;
	//���� ������ �������; ���� ������ ������� �����; ���� ����� �����
	int pwr_flag = 0, flt_flag = 0, neg_flag = 1;
	//����� � ������� �����
	float intg = 0, flt = 0;

	//����� ������ �������� ������ ����� 0
	open_br[0] = 0;
	for (int i = 0; expr[i] != '\0'; ++i)
	{
		//���� ����. ������
		if (expr[i] == '(')
		{
			//���� � ������ ������ ��� ����� ������� ����� �� �����
			if (i == 0 || expr[i - 1] < '0' || expr[i - 1] > '9')
			{
				//����� ����� = 1
				intg = 1;
			}

			//������ ������� � ������ ������
			br[left_br].ord = ord;
			br[left_br].num = left_br;
			//����������� ��������� �� ��� ������ � ������� ����������
			res[*b][*r].br = &br[left_br];

			//��������� ������
			pwr_flag = flt_flag = 0;
			//���������� �� ����
			intg *= neg_flag;
			//����� ����� � �������������
			neg_flag = 1;
			//������������� ����������
			init_var(&intg, &flt);

			//���������� �������� ����. ������, ������ ��������� ��������
			++left_br;
			++open_i;
			//������ ������ �������� ������
			open_br[open_i] = left_br;
			//������ ����� ������� ������ � ����� ������� ����������
			*b = open_br[open_i];
			r = &res_cnt[*b];
			//���������� ������� ������
			++ord;

			//������ ����� +
			res[*b][*r].sgn = '+';
		}
		//���� ����. ������
		else if (expr[i] == ')')
		{
			//���� ����� ���� ���� ����. ������
			if (expr[i - 1] != ')')
			{
				//��������� ������, ���������� �� ����
				pwr_flag = flt_flag = 0;
				intg *= neg_flag;
				neg_flag = 1;
				//����. ����� ����������
				init_var(&intg, &flt);
			}

			//���������� ������ �������� ������
			--open_i;
			//������ ����� ������� ������ � ������� ����������
			*b = open_br[open_i];
			r = &res_cnt[*b];
			//���������� ������� ������
			--ord;
		}
		//���� �����
		else if (expr[i] >= '0' && expr[i] <= '9')
		{
			//���� ���� ������ �������
			if (pwr_flag)
			{
				//���������� �� 10 � �������� � ������ (- '0' - ������� char ������� � ����� int)
				res[*b][*r].pwr *= 10;
				res[*b][*r].pwr += expr[i] - '0';
			}
			//���� ���� ������ ������� �����
			else if (flt_flag)
			{
				flt *= 10;
				flt += expr[i] - '0';
			}
			//����� ���� ������� ������ ��� - ������ ����� �����
			else
			{
				intg *= 10;
				intg += expr[i] - '0';
			}
		}
		//���� �
		else if (expr[i] == 'x')
		{
			//���� � ������ ������ ��� ����� ��� �� �����
			if (i == 0 || expr[i - 1] < '0' || expr[i - 1] > '9')
			{
				//����� ����� ����� 1
				intg = 1;
			}
			//������� ����� 1 (���� ������ ��� �� ����� ����������)
			res[*b][*r].pwr = 1;
		}
		//���� ���� �������
		else if (expr[i] == '^')
		{
			//������� ����� 0 (���� ��-���� ����������)
			res[*b][*r].pwr = 0;
			//���� ������ �������
			pwr_flag = 1;
		}
		//���� �����
		else if (expr[i] == '.')
		{
			//���� ������ ������� �����
			flt_flag = 1;
		}
		//���� ���� ��������
		else if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/')
		{
			//���� �� � ������ ������ � ����� ������ ����� * ��� /
			if (i != 0 && (expr[i - 1] == '*' || expr[i - 1] == '/'))
			{
				//������ �������������� ����-��
				neg_flag = -1;
			}
			//�����
			else
			{
				//���� �� � ������ ������ � ����� ��� ����� ����. ������
				if (i != 0 && expr[i - 1] != ')')
				{
					//���������, ����������, ������������� ����� �����.
					pwr_flag = flt_flag = 0;
					intg *= neg_flag;
					neg_flag = 1;
					init_var(&intg, &flt);
				}
				
				//���� ���� - �����
				if (expr[i] == '-')
				{
					//������ �����. ����-��, ���� ����-�� - ����
					neg_flag = -1;
					res[*b][*r].sgn = '+';
				}
				//�����
				else
					//������ ���������� �����
					res[*b][*r].sgn = expr[i];
			}
		}
	}
	//���� ����� �/��� ������� ����� �������� ���������
	if (intg != 0 || flt != 0)
	{
		//���������� �� ����, �������� ������ � ��������� ����������
		intg *= neg_flag;
		while (flt >= 1)
			flt /= 10;
		res[*b][*r].cft = intg + flt;
		++(*r);
	}

	//���-�� ������ ����� ���-�� ����������� ������
	br_cnt = left_br;
}

//����� ��������� �� �����
void output_expression(variable* var, int cnt)
{
	//�� 0 �� ����������� ����������
	for (int i = 0; i < cnt; ++i)
	{
		//���� ����-� ����� 0 - ����� ����
		if (var[i].cft == 0)
		{
			printf("0");
		}
		//�����
		else
		{
			//���� ������ ����-�
			if (i == 0)
			{
				//���� ������������� - ����� ������ ��� ��������
				if (var[i].cft < 0)
					printf("-");
			}
			//�����
			else
			{
				//���� �������������
				if (var[i].cft < 0)
				{
					//���� - ���� - ����� ������ � ���������
					if (var[i].sgn == '+')
						printf(" %c ", '-');
					//����� ����� ����� � ��������� � ������ ��� �������
					else
						printf(" %c -", var[i].sgn);
				}
				//����� ����� ����� � ��������� ��� ������
				else
					printf(" %c ", var[i].sgn);
			}

			//���� ����-� �������������
			if (var[i].cft < 0)
				//����� -����-��
				printf("%.2f", -var[i].cft);
			else
				//����� ����� ����-��
				printf("%.2f", var[i].cft);

			//���� ��������� ������ ���������
			if (var[i].br != NULL)
			{
				//����� ������ � ��������� �� ������ ������ + 1 ����� ����
				printf(" (");
				output_expression(res[var[i].br->num + 1], res_cnt[var[i].br->num + 1]);
				printf(")");
			}
			//����� (������ ���)
			else
			{
				//������� = 1 - ����� ������ �
				if (var[i].pwr == 1)
					printf(" x");
				//����� ���� ������� �� 0 - ����� x �� ��������
				else if (var[i].pwr != 0)
					printf(" x^%d", var[i].pwr);
			}
		}
	}
}

//��������� �������� ����������
void set_var(variable* var1, variable* var2)
{
	//�� var2 ������������ � var1
	var1->br = var2->br;
	var1->cft = var2->cft;
	var1->pwr = var2->pwr;
	var1->sgn = var2->sgn;
}

//���������� ��������� � ������ �� �������� ��������
void sort_bracket(variable* var, int cnt)
{
	//��������� ����������
	variable tmp;
	//���� ������� ������������
	int flag = 1;

	//���� ���� ������������ - ��������� ���������
	while (flag)
	{
		//�����. �����
		flag = 0;
		//�� 0 �� ����������� ���-�� ���������� � ������ - 1
		for (int i = 0; i < cnt - 1; ++i)
		{
			//���� ������� ������� �����. ������ ������� ��������� �����.
			if (var[i].pwr < var[i + 1].pwr)
			{
				//������ ������� ������� � ��������� ����� tmp
				set_var(&tmp, &var[i]);
				set_var(&var[i], &var[i + 1]);
				set_var(&var[i + 1], &tmp);

				//������������ ����
				flag = 1;
			}
		}
	}
}

//�������� ���������� � ����������� ��������� � ��������������� ���������
void quick_add(variable* var, int* cnt)
{
	//�� 0 �� ���-�� ���������� - 1
	for (int i = 0; i < *cnt - 1; ++i)
	{
		//���� ����-� ����� 0
		if (var[i].cft == 0)
		{
			//�� �������� �� ����� - 1
			for (int j = i; j < *cnt - 1; ++j)
			{
				//������ �� ��������� ���������� � ������� (�������� �� 1 � ������)
				set_var(&var[j], &var[j + 1]);
			}
			//���������� ���-�� ����������
			--(*cnt);
			//�������� �� 1 ���������� ����� (���������� ��������� ��� ���)
			--i;
			//��������� ���� �������� (� ������ for)
			continue;
		}
		//���� ������� ������� � ��������� ���������� �����
		if (var[i].pwr == var[i + 1].pwr)
		{
			//�������� ����-���
			var[i].cft += var[i + 1].cft;
			//�� ��������� ���������� �� ����� - 1
			for (int j = i + 1; j < *cnt - 1; ++j)
			{
				//�������� �� 1 � ������
				set_var(&var[j], &var[j + 1]);
			}
			//���������� ���-��, �������� �� 1 �����
			--(*cnt);
			--i;
		}
	}
}

//�������
void divide(variable* var1, variable* var2, int* cnt1, int* cnt2)
{
	//������ ��� �������� � ������������� ���������
	variable dif_tmp[N * N / 4];
	//������� ���-�� ���������� � ��������
	int dif_cnt;
	//��������� �������� ���������� � ���������� ��������
	tmp_cnt = 0;

	//���� ������� ������� �������� �� ������ ������� �������
	if (var1[0].pwr >= var2[0].pwr)
	{
		do
		{
			//������� ��������� ���������� ���������� ������� - ��������� ������� �������� �� ������� ��������
			tmp[tmp_cnt].pwr = var1[0].pwr - var2[0].pwr;
			//����-� - �������
			tmp[tmp_cnt].cft = var1[0].cft / var2[0].cft;
			tmp[tmp_cnt].sgn = '+';

			//��������� �������� ������� � ������ � ��� ������� - ���� ������� ���������� ���������� � ������� ������� ���������� ��������
			//� � ����-� ������������ ����-�� ���������� ���������� � ����-�� ������� ���������� ��������
			mod_cnt = 0;
			for (int i = 0; i < *cnt2; i++)
			{
				mod_tmp[i].pwr = tmp[tmp_cnt].pwr + var2[i].pwr;
				mod_tmp[i].cft = tmp[tmp_cnt].cft * var2[i].cft;
				mod_tmp[i].sgn = '+';
				++mod_cnt;
			}
			//� ���������� - ������ ��������� ���������� ���������� �������
			++tmp_cnt;

			//������ � ������ �������� �������������� ��������
			//������ ����-�� � ������� �� ��������
			//����� ����-�� �� ������ ����� � ������� �� �������
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

			//���������� � �������� ���������� �������� ��������
			sort_bracket(dif_tmp, dif_cnt);
			quick_add(dif_tmp, &dif_cnt);

			//������ � ������� ���������� �������� ��� ���������� ��������
			for (int i = 0; i < dif_cnt; ++i)
			{
				var1[i].cft = dif_tmp[i].cft;
				var1[i].pwr = dif_tmp[i].pwr;
			}
			*cnt1 = dif_cnt;

			//������ � ������� ���������� ��������
			for (int i = 0; i < *cnt1; ++i)
			{
				mod_tmp[i].cft = var1[i].cft;
				mod_tmp[i].pwr = var1[i].pwr;
			}
			mod_cnt = *cnt1;

		//���� ������� ������� ������� �� ������ ������� ������� ��������
		} while (mod_tmp[0].pwr >= var2[0].pwr);
	}
	//����� ���� ������� ������� �������� ������, ��� � ��������
	else
	{
		//����� ���� ���������� � ����-��� 0
		++tmp_cnt;
		tmp[0].cft = 0;
		for (int i = 0; i < *cnt1; i++)
		{
			//������ � ������� ��������
			set_var(&mod_tmp[i], &var1[i]);
			//��������� �������� ������
			var1[i].cft = 0;
		}
		//������� �������� ����� �������� ��������
		mod_cnt = *cnt1;
		//������� �������� ����� ����
		*cnt1 = 0;
	}
}

//���������
void multiply(variable* var1, variable* var2, int* cnt1, int* cnt2)
{
	//��������� �������� ���������� � ���������� ��������
	tmp_cnt = 0;

	//�� 0 �� ���-�� �����. � ������ ���������
	for (int i = 0; i < *cnt1; ++i)
	{
		//�� 0 �� ���-�� �����. �� ������ ���������
		for (int j = 0; j < *cnt2; ++j)
		{
			//��������� ������ � ���� - �������� �� ���������
			//����-� - ������������, ������� - ����� 
			tmp[tmp_cnt].br = NULL;
			tmp[tmp_cnt].cft = var1[i].cft * var2[j].cft;
			tmp[tmp_cnt].pwr = var1[i].pwr + var2[j].pwr;
			tmp[tmp_cnt].sgn = '+';

			//����. �������� �����. � ������������
			++tmp_cnt;
		}
	}
}

//��������/���������
void add_sub(variable* var1, variable* var2, int* cnt1, int* cnt2)
{
	//������� �������
	int cur_pwr;
	//��������� �������� ���������� � ���������� ��������
	tmp_cnt = 0;

	//�� 0 �� ������� ����������� ��������� �� ���������
	for (int i = 0; i < max(*cnt1, *cnt2); ++i)
	{
		//���� ����-� ������� ���������� ������� ���������� �� ���� � ����� �������� �� ����� �� ������� ������� ���������� 
		if (var1[i].cft != 0 && i < *cnt1)
		{
			//������� ������� ����� ������� ������� ���������� ������� ����������
			cur_pwr = var1[i].pwr;

			//������������� ���������� � ������� ���������� ��������
			tmp[tmp_cnt].br = NULL;
			tmp[tmp_cnt].cft = 0;
			tmp[tmp_cnt].pwr = cur_pwr;
			tmp[tmp_cnt].sgn = '+';

			//�� �������� �������� �� ������������� ������� ���������
			for (int j = i; j < max(*cnt1, *cnt2); ++j)
			{
				//���� ����� �� ����� �� ������� ������� ���������� � ������� ������� ���������� ������� ���������� ����� ������� �������
				if (j < *cnt1 && var1[j].pwr == cur_pwr)
				{
					//����������� �������� � ��������� �� ��������� ���������� ��������
					tmp[tmp_cnt].cft += var1[j].cft;
					var1[j].cft = 0;
				}
				//���������� ��� ������� ����������
				if (j < *cnt2 && var2[j].pwr == cur_pwr)
				{
					tmp[tmp_cnt].cft += var2[j].cft;
					var2[j].cft = 0;
				}
			}
			
			//���� �������� ����-� �� ���� ������ ���� - ������ ��������� �������������� ����������
			if (tmp[tmp_cnt].cft != 0)
				++tmp_cnt;
		}
		//���������� ��� ������� ����������
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

//����� �������, ���������� ������ ��������� � ��������/��������� (��. ����)
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

					printf("\n �������� %d:\n\n", act_cnt);
					printf("  %.2f x^%d / %.2f x^%d = ", var1->cft, var1->pwr, var2->cft, var2->pwr);

					divide(var1, var2, &one, &one);
				}
				else
				{
					var1 = res[res[n][i - 1].br->num + 1];
					var2 = &res[n][i];
					++act_cnt;

					printf("\n �������� %d:\n\n", act_cnt);
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

					printf("\n �������� %d:\n\n", act_cnt);
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

					printf("\n �������� %d:\n\n", act_cnt);
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

				//��� ������������� ���������� � ��������, �.�. ��������� ������� � ��� ������������

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
			printf(" (�������: ");
			output_expression(mod_tmp, mod_cnt);
			printf(")\n\n");
			system("pause");
		}
	}
}

//����� ���������, �������� ���������� ������ ��������/��������� (��. ����)
//���. ������� ���������
void if_mul(int n)
{
	variable* var1, * var2;
	int one = 1;

	for (int i = 0; i < res_cnt[n]; ++i)
	{
		//���� ����-� ����� �������
		if (res[n][i].br != NULL)
		{
			//�� ����� 1
			if (res[n][i].cft != 1)
			{
				//���������� ��������� ����-�� �� ������
				var1 = &res[n][i];
				var2 = res[res[n][i].br->num + 1];
				++act_cnt;

				printf("\n �������� %d:\n\n", act_cnt);
				printf("  %.2f * (", var1->cft);
				output_expression(var2, res_cnt[res[n][i].br->num + 1]);
				printf(") = ");

				multiply(var1, var2, &one, &res_cnt[res[n][i].br->num + 1]);

				//������ ����-� ����� 1
				res[n][i].cft = 1;
				for (int j = 0; j < tmp_cnt; ++j)
				{
					//������ �� tmp � res
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

					printf("\n �������� %d:\n\n", act_cnt);
					printf("  %.2f x^%d * %.2f x^%d = ", var1->cft, var1->pwr, var2->cft, var2->pwr);

					multiply(var1, var2, &one, &one);
				}
				else
				{
					var1 = res[res[n][i - 1].br->num + 1];
					var2 = &res[n][i];
					++act_cnt;

					printf("\n �������� %d:\n\n", act_cnt);
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

					printf("\n �������� %d:\n\n", act_cnt);
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

					printf("\n �������� %d:\n\n", act_cnt);
					printf("  (");
					output_expression(var1, res_cnt[res[n][i - 1].br->num + 1]);
					printf(") * (");
					output_expression(var2, res_cnt[res[n][i].br->num + 1]);
					printf(") = ");

					multiply(var1, var2, &res_cnt[res[n][i - 1].br->num + 1], &res_cnt[res[n][i].br->num + 1]);
				}
			}

			//���� � ���������� ���������� �������� �� ���� ����������
			if (tmp_cnt >= 2)
			{
				//������ � ����� ������ ������ � �������
				br[br_cnt].num = br_cnt;
				br[br_cnt].ord = br[n].ord;
				//���������� ��������� �� ����� ������
				res[n][i - 1].br = &br[br_cnt];
				res[n][i - 1].cft = 1;
				res[n][i - 1].pwr = 0;

				//���������� � �������� ���������� ��������
				sort_bracket(tmp, tmp_cnt);
				quick_add(tmp, &tmp_cnt);
				//���-�� ���������� � ����� ������ ����� ���-�� ���������� � ���������� �������� (��������������� � ���������)
				res_cnt[br_cnt + 1] = tmp_cnt;

				for (int j = 0; j < tmp_cnt; ++j)
				{
					//������ �� tmp � res
					set_var(&res[br_cnt + 1][j], &tmp[j]);
				}
				//����. ���-�� ������
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

//����� ��������/���������
void if_add_sub(int n)
{
	//��� ���������
	variable* var1, * var2;
	//������� ��� �������� �� ������ � �������
	int one = 1;
	//����������� �������� ��������
	++act_cnt;

	//����� ������������ ��������
	printf("\n �������� %d:\n\n  ", act_cnt);
	output_expression(res[n], res_cnt[n]);
	printf(" = ");

	//�� 1 �� ���-�� ���������� � ���������
	for (int i = 1; i < res_cnt[n]; ++i)
	{
		//���� ���� ������� ���������� - ����
		if (res[n][i].sgn == '+')
		{
			//���� ��� ���������� - �� ����-� ������
			if (res[n][i].br == NULL)
			{
				//���� ���������� ���������� - �� ����-� ������
				if (res[n][i - 1].br == NULL)
				{
					//��� ���������� - ���������
					var1 = &res[n][i - 1];
					var2 = &res[n][i];

					add_sub(var1, var2, &one, &one);
				}
				//����� ���� ����-� ������
				else
				{
					//�������� ������ � ��������� ����������
					var1 = res[res[n][i - 1].br->num + 1];
					var2 = &res[n][i];

					add_sub(var1, var2, &res_cnt[res[n][i - 1].br->num + 1], &one);
				}
			}
			//����� ���� ����-� ������
			else
			{
				//���� ���������� ���������� - �� ����-� ������
				if (res[n][i - 1].br == NULL)
				{
					//�������� ��������� ���������� � ������
					var1 = &res[n][i - 1];
					var2 = res[res[n][i].br->num + 1];

					add_sub(var1, var2, &one, &res_cnt[res[n][i].br->num + 1]);
				}
				//����� ���� ����-� ������
				else
				{
					//�������� ���� ������
					var1 = res[res[n][i - 1].br->num + 1];
					var2 = res[res[n][i].br->num + 1];

					add_sub(var1, var2, &res_cnt[res[n][i - 1].br->num + 1], &res_cnt[res[n][i].br->num + 1]);
				}
			}

			//������ ���������� �������� � ������ res
			//���� ��������� �������� �������� �� ���� ����������
			if (tmp_cnt >= 2)
			{
				//�� 0 �� ���-�� ���������� � ���������� �������� -2 (�.�. ������������ ��� ����������)
				for (int j = 0; j < tmp_cnt - 2; ++j)
				{
					//�������� ���������� � ����� �� ���-�� ���������� � ���������� �������� -2
					set_var(&res[n][res_cnt[n] - 1 - j + tmp_cnt], &res[n][res_cnt[n] - 1 - j]);
				}
				for (int j = 0; j < tmp_cnt; ++j)
				{
					//������ �� ������� ���������� �������� tmp � res
					set_var(&res[n][i - 1 + j], &tmp[j]);
				}
				//����. ���-�� ���������� �� ���-�� � ���������� �������� -2
				res_cnt[n] += tmp_cnt - 2;
			}
			//����� ���� �������� ���� ����������
			else
			{
				//������ �� tmp � res
				set_var(&res[n][i - 1], &tmp[0]);

				for (int j = i + 1; j < res_cnt[n]; ++j)
				{
					//�������� �� 1 ������� � ������
					set_var(&res[n][j - 1], &res[n][j]);
				}
				//���������� ���-�� �� 1 � �������� �� 1 ������� �����
				--res_cnt[n];
				--i;
			}
		}
	}
	//���������� � �������� ���������� �������� � �������� ���������
	sort_bracket(res[n], res_cnt[n]);
	quick_add(res[n], &res_cnt[n]);

	//����� ��������� ��������� �� �����
	output_expression(res[n], res_cnt[n]);
	printf("\n\n");
	system("pause");
}

//��������� ��������� � ������
void proc_bracket(int n)
{
	//������ ���������� ���� �������, ����� ���������, ����� ��������/���������
	if_div(n);
	if_mul(n);
	if_add_sub(n);
}

//��������� ������ ���������
void proc_expression()
{
	//���������� ������������� ������� ������, ����� � ���� ������
	int max_ord = 0;
	for (int i = 0; i < br_cnt; ++i)
	{
		if (br[i].ord > max_ord)
			++max_ord;
	}

	//�� ������������� ������� �� 0
	for (int i = max_ord; i >= 0; --i)
	{
		//�� 0 �� ���-�� ������
		for (int j = 0; j < br_cnt; ++j)
		{
			//���� ������ ������ ������� - ��������� ���������
			if (br[j].ord == i)
				proc_bracket(j + 1);
		}
	}
	//� ����� - ��������� ������ ��������� ��������� ("�������" ���������)
	proc_bracket(0);
}

void main(void)
{
	setlocale(LC_ALL, "Russian");
	int correct = 0;
	
	//���� correct ����� 0
	while (!correct)
	{
		//������� ������� expr ��� ������ ���������
		memset(expr, 0, N);

		system("cls");
		printf("\n  ������� ��������� � ����������\n\n");
		printf("  - ��� ����������� ������� �������� ����������� ������������� ������: ()\n");
		printf("  - ���������� ����������� ����������: x\n");
		printf("  - ���������� ��������: + - * /\n");
		printf("  - ����������� ����������� ������� ����������: ^\n");
		printf("  - ������������ ���������� ����������� ��������: %d\n\n  ", N);

		//���������� ���������
		gets_s(expr, N);
		//������� check_expression ���������� 0, ���� ���� ������ �� �������� ���������, � 1, ���� ������ ���
		correct = check_expression();
	}
	//������ ��������� � ������ � ������ �������� ���������� res
	read_expression();

	//����� ��������� ���������
	system("cls");
	printf(" �������� ���������:\n\n  ");
	output_expression(res[0], res_cnt[0]);
	printf("\n\n");
	system("pause");

	//���������� �������� � ���������
	printf("\n\n");
	proc_expression();
}