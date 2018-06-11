#include <stdio.h>

int add(int a, int b) { return (a+b);}
int sub(int a, int b) { return (a-b);}
int mul(int a, int b) { return (a*b);}
double div(int a, int b) { return ((double)a/b);}
int mod(int a, int b) {return (a%b);}

int main(int argc, char **argv)
{
	int oprd1, oprd2, res;
	char line[128], exitstr[20], opr[20];

	setbuf(stdout, NULL);
	printf("�����Ǵ� ������ +, -, *, /, % \n");
	printf("��뿹: 2+3[enter], 2 * 3 [enter]\n");
	printf("�����Ϸ��� exit[enter]\n\n");

	for(;;) {

		printf("Expression: ");
		gets(line);
		sscanf(line, "%s", exitstr);
		if (!strcmp(exitstr, "exit"))
			exit(0);
		sscanf(line, "%d %s %d", &oprd1, opr, &oprd2);
		if (strlen(opr) > 1) {
			sscanf(opr+1, "%d", &oprd2);
			opr[1] = 0;
		}
		if(opr[0] == '+')
			res = add(oprd1, oprd2);
		else if (opr[0] == '-')
			res = sub(oprd1, oprd2);
		else if (opr[0] == '*')
			res = mul(oprd1, oprd2);
		else if (opr[0] == '/') {
			double res2 = div(oprd1, oprd2);
			printf("Result: %d %s %d = %f\n", oprd1, opr, oprd2, res2);
			continue;
			}
		else if (opr[0] == '%')
			res = mod(oprd1, oprd2);
		else{
        	printf("%s: ���ǵ��� ���� ��������!!\n", opr);
			continue;
		
		}
		printf("Result: %d %s %d = %d\n", oprd1, opr, oprd2, res);
	}
}
