// "mkrec.c"�� ������ include�� ��� �ѹ��� include�ϱ� ����
#ifndef _MKREC_C
#define _MKREC_C

//*****************************************************************************
//   �������� ���Ǵ� �⺻���� �Լ���
//*****************************************************************************

static int  i, cnt;				// �� �Լ����� ���� ���Ǵ� ������

#include <stdarg.h>

#define PRINT_WAIT(_fmt) \
do { \
    va_list arg; \
    char buf[BUF_LEN]; \
    va_start(arg, _fmt); \
    vsprintf(buf, _fmt, arg); \
    printf("%s", buf); \
    fflush(NULL);	\
    va_end(arg); \
	printf("\n[Presss Enter] ... "); \
	gets(buf); \
} while(0)

static void
err_wait(char *fmt, ...)
{
	PRINT_WAIT(fmt);
}

static void
err_wait_jmp(char *fmt, ...)
{
	PRINT_WAIT(fmt);
	longjmp(jump, -1);
}

// �Է���ġ���� ���� �ϳ��� �о� ����

static int 
get_int(char *quest, char *err_msg, int from, int to)
{
	char line[LINE_LEN];
	printf("%s ", quest);
	gets(line);
	if (line[0] == 0)
		longjmp(jump, -1);
	if ( !isdigit( (int)line[0] ) ||
		 (i = atoi(line)) < from  || i > to )
		err_wait_jmp("%d: %s", i, err_msg? err_msg: "�������� �ʴ� ��ȣ��");
	return(i);
}

//*****************************************************************************
//   ���� �Լ���
//*****************************************************************************

static int
get_key(char *opr, int start_key, int rec_num)
{
	char msg[LINE_LEN], err[LINE_LEN];
	int end_key = start_key + rec_num - 1;

	sprintf(msg, "%s�� ���ڵ��� Ű��?", opr);
	sprintf(err, "key ���� [ %d - %d ]���� ��\n", start_key, end_key);
	return(get_int(msg, err, start_key, end_key));
}

//===========================================================================
// �ڵ����� ���ڵ� �ʵ带 �����ϰų� �����ϴ� �Լ���
//===========================================================================

static char *family[] = {
	"��", "��", "��", "��", "��", "��", "��", "��", "��", "��", 
};
static char *name[] = {
	"��", "ȫ", "��", "��", "��", "��", "��", "��", "��", "��",
	"��", "��", "��", "��", "��", "��", "��", "��", "��", "��",
	"��", "��", "��", "��", "��", "��", "��", "��", "��", "��",
};
static char *dep[] = {
	"��ǻ�Ͱ��а�", "������Ű��а�", "���ڰ��а�", "�����а�", "�ݼӰ��а�", 
};
static char *city[] = {
	"���ֱ�����", "ȭ����", "��õ��", "������", "�强��", 
	"�س���",     "������", "��籺", "�����", "������" 
};
static char *gu[] = {
	"�߱�", "�ϱ�", "����", "����", "����", 
};

#define R(_m)	( rand() % (_m) )

// ���ڵ��� �к�, �г�, �ּҸ� ������ 
void
update_rec(rec_t *r)
{
	sprintf(r->dep, "%s", dep[R(5)]);
	r->grade = R(4) + 1;	// �г�
	sprintf(r->addr, "%s %s %s%s�� %d-%d", city[R(10)], gu[R(5)],
			name[R(30)], name[R(30)], R(200)+100, R(50)+10);
}

// ���ڵ��� �� �ʵ带 �ڵ����� ������  
static void
new_rec(rec_t *r, int key)
{
	r->key = key;	// 132
	sprintf(r->name, "%s%s%s", family[R(10)], name[R(30)], name[R(30)]);
	update_rec(r);
}

rec_t *
alloc_recs(rec_t *recs, int rec_num)
{
	if (recs) free(recs);
	recs = malloc(sizeof(rec_t) * rec_num);
	return(recs);
}

#include <time.h>

rec_t *
create_recs(rec_t *recs, int start_key, int rec_num)
{
	// ���� ���� �߻����� �õ� �� ����: �� ���� ���α׷� ���� ������ 
	srand(time(NULL));		// �޶�� �Ź� �ٸ� ���ڰ� �߻���
	recs = alloc_recs(recs, rec_num);
	for (i = 0; i < rec_num; ++i)
		new_rec(&recs[i], start_key+i);
	return(recs);
}

void
update_recs(rec_t *recs, int rec_num)
{
	// ���� ���� �߻����� �õ� �� ����: �� ���� ���α׷� ���� ������ 
	srand(time(NULL));		// �޶�� �Ź� �ٸ� ���ڰ� �߻���
	for (i = 0; i < rec_num; ++i)
		update_rec(&recs[i]);
}

//===========================================================================
// �޴��� ���÷����ϰ�, ���õ� �Լ��� ȣ���Ѵ�.
//===========================================================================

static void
print_menu(char *title, char *menu[], int num)
{
	int maxlen = 0, len;

	for (i = 1; i < num; ++i)
		if ( (len=strlen(menu[i])) > maxlen ) maxlen = len;

	print_line_seperator();
	printf( "\n%s (����I/O����: %d, �����̸�: %s)\n", 
			title, resource.fio_type, head->name);
	printf("%s\n%-*s", menu[0], maxlen, menu[1]);
	for (i = 2, len = maxlen; i < num; ++i) {
		len += maxlen + 3;
		if (len >= 80) {
			len = maxlen;
			printf("\n%-*s", maxlen, menu[i]);
		}
		else
			printf("   %-*s", maxlen, menu[i]);
	}
	printf("\n\n");
}

void
menu_loop(char *title, char *menu[], funct_t funct[], int fnum)
{
	while (1) {
		setjmp(jump);
		print_menu(title, menu, fnum);
		i = get_int("�׽�Ʈ�� ��ȣ��?", NULL, 0, fnum-1);
		if (i == 0) break;
		print_line_seperator();
		printf("%s\n", menu[i]);
		funct[i]();
	}
}

//===========================================================================
// ���� �̸� �����ϱ�
//===========================================================================

#include <dirent.h>

static void
get_max_name_len(DIR *dp, int *p_max_name_len, int *p_num_per_line)
{
	struct dirent *dirp;
	int max_name_len = 0; // ���� �� �����̸� ����

	while ((dirp = readdir(dp)) != NULL) { 
		int name_len = strlen(dirp->d_name);
		if (name_len > max_name_len)
			max_name_len = name_len;
	}
	rewinddir(dp);
	max_name_len += 4;
	*p_num_per_line = 80 / max_name_len; 
	*p_max_name_len = max_name_len;
}

void 
list_files(char *dir, char *ext)
{
	DIR	 *dp;
	struct dirent *dirp;
	int max_name_len, num_per_line, cnt = 0;

	print_line_seperator();
	if ((dp = opendir(dir)) == NULL) {	// ���丮 ����
		perror("opendir()");
		return;
	}
	get_max_name_len(dp, &max_name_len, &num_per_line);
	while ((dirp = readdir(dp)) != NULL) {
		if (strstr(dirp->d_name, ext) == NULL) // Ȯ���ڰ� ���� ������
			continue;
		printf("%-*s", max_name_len, dirp->d_name);
		if ((++cnt % num_per_line) == 0)
			printf("\n");
	}
	if ((cnt % num_per_line) != 0) 
		printf("\n");
	closedir(dp);						// ���丮 �ݱ�
	print_line_seperator();
}

//===========================================================================
// Ű���忡�� ���� �̸� �Է� �ޱ� 
//===========================================================================

static char *
get_filename(char *mode)
{
	static char path_name[NAME_LEN];
	int fio_type = head->fio_type;
	int is_text = (fio_type < 2);
	char name[NAME_LEN], ext[5];
	
	strcpy(ext, is_text? ".txt": ".bin");
	list_files("./db", ext);
	if (mode[0] == 'r' || mode[1] == 'w') {	// "r" �Ǵ� "rw"�� ��� 
		printf("�׳� ���͸� ġ�� ����Ʈ �̸��� �����\n");
		printf("Ȯ���ڴ� �Է����� �ʾƵ� ��\n");
		printf("%s ���� �̸���(%s)? ", 
				mode[0] == 'r'? "�ҷ���":"������", default_file_name);
	}
	else {									// �����ϱ��� ���
		printf("Ȯ���ڴ� �Է����� �� ��\n");
		printf("������ ���� �̸���? ");
	}
	gets(name);
	if (name[0] == 0)
		return( mode[0] == 'n'? NULL : default_file_name);

	if (mode[0] == 'n') {	// �ٸ� �̸����� �����ϱ��� ���
		char *p;
		// Ȯ���ڸ� �߸� ������ ��� �ڵ����� ������
		if ( (  is_text && (p=strstr(name, ".bin")) ) ||
			 ( !is_text && (p=strstr(name, ".txt")) ) )
			err_wait_jmp("Ȯ���ڸ� �߸� �����Ͽ���");
	}
	// Ȯ���ڸ� ���� ���� ��� �ڵ����� ������
	if (strstr(name, ext) == NULL)
		strcat(name, ext);
	sprintf(path_name, "./db/%s", name);
	return(path_name);
}

#endif  /* _MKREC_C */

