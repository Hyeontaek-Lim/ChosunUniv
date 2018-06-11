#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SZ_STR_BUF		256

int *cmd;
char cmd_line[SZ_STR_BUF];

void
cp(void)
{
	char *oldnime = strtoken(NULL, " \t\n");
	char *newnime = strtoken(NULL, " \t\n");

	if ((oldnime == NULL) || (newnime == NULL)) {
		printf("%s: Insufficient arguments\n", cmd);
		goto usage;
	}
	else if (strtoken(NULL, " \t\n") != NULL) {
		printf("%s: Too many arguments\n", cmd);
		goto usage;
	}
	// ���⿡ ���� �����ϴ� �ڵ带 �����϶�.
	printf("���� �� ���ɾ�� �������� �ʾҽ��ϴ�.\n");
	printf("���� ������ �ֱ� �ٶ��ϴ�.\n");
	return;

usage:
	printf("Usage: %s oldnime newnime\n", cmd);
}

void
pwd(void)
{
	// �������� cwd�� ����� ���ڿ��� ȭ�鿡 ����Ѵ�.
	printf("���� �� ���ɾ�� �������� �ʾҽ��ϴ�.\n");
	printf("���� ������ �ֱ� �ٶ��ϴ�.\n");
}

void
echo(void)
{
	// ���ɾ� ���ο��� �Էµ� �������� �ϳ��� ©�� ȭ�鿡 ����Ѵ�.
	// �׸��� �������� ����(new line)�� ����Ѵ�.
	printf("���� �� ���ɾ�� �������� �ʾҽ��ϴ�.\n");
	printf("���� ������ �ֱ� �ٶ��ϴ�.\n");
}

void
mv(void)
{
	char *oldnime = strtoken(NULL, " \t\n");
	char *newnime = strtoken(NULL, " \t\n");

	if ((oldnime == NULL) || (newnime == NULL)) 
		printf("%s: Insufficient arguments\n", cmd);
	else if (strtoken(NULL, " \t\n") != NULL) 
		printf("%s: Too many arguments\n", cmd);
	// ���⿡ ���� �����ϴ� �ڵ带 �����϶�.
	printf("���� �� ���ɾ�� �������� �ʾҽ��ϴ�.\n");
	printf("���� ������ �ֱ� �ٶ��ϴ�.\n");
	return;
}

void
rm(void)
{
	char *name = strtoken(NULL, " \t\n");

	if (name == NULL) {
		printf("%s: Insufficient arguments\n", cmd);
		goto usage;
	}
	else if (strtoken(NULL, " \t\n") != NULL) {
		printf("%s: Too many arguments\n", cmd);
		goto usage;
	}
	// ���⿡ ȭ���� �����ϴ� �ڵ带 �����Ѵ�. 
	// �����Ѱ��� �ϴ� ���� ȭ������ �Ǵ� ���丮���� üũ�ϰ� 
	// ������ ��츦 �����Ͽ� �ý���(API) �Լ��� ȣ���ؾ� ��
	printf("���� �� ���ɾ�� �������� �ʾҽ��ϴ�.\n");
	printf("���� ������ �ֱ� �ٶ��ϴ�.\n");
	return;

usage:
	printf("Usage: %s file or empty directory name\n", cmd);
}

void
quit()
{
	exit(0);
}

void help(void);

typedef struct {
	char *cmd;
	void (*func)();
} cmd_disp_t;

// ���ο� ���ɾ �߰��� ������, �Ʒ��� �迭�� �ش� ���ɾ���
// ���ڿ� �̸��� �� ���ɾ �����ϴ� �Լ��� �̸��� �߰��ؾ� �Ѵ�.
cmd_disp_t cmd_disp[] = {
	{ "cp",			cp },
	{ "echo",		echo },
	{ "help",		help },
	{ "pwd",		pwd },
	{ "exit",		quit },
	{ "move",		move },
};

int num_cmd = sizeof(cmd_disp) / sizeof(cmd_disp[0]);


void
help(void)
{
	int  k;
	
	for (k = 0; k < num_cmd; ++k)
		printf("%s ", cmd_disp[k].cmd);
	printf("\n");
}

void
proc_cmd(void)
{
	int  k;
	
	cmd = strtoken(cmd_line, " \t\n");
	if (cmd == NULL)
		return;
	for (k = 0; k < num_cmd; ++k)
		if (!strcmp(cmd, cmd_disp[k].cmd)) {
			cmd_disp[k].func();
			break;
		}
	if (k == num_cmd)
		printf("%s : Command not found.\n", cmd);
}

int 
main(int argc, char *argv[])
{
	int a, bb, ccc ;

	setbuf(stdin, NULL);

	setbuf(stdout, NULL);

	for (;;) {
		printf(">> ");
		gets(cmd_line);
		proc_cmd();abc();
	}
}



