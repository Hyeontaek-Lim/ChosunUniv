#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

#define NAME_LEN	20
#define LINE_LEN	80
#define BUF_LEN		512

static jmp_buf jump;			// setjmp(), longjmp()������ ����

#define print_line_seperator() \
   printf("----------------------------------------------------------------\n")

// File I/O ������ �����ϴ� ���ڿ��� �迭
static char *fio_types[] = {
	"0.Text File IO: fopen() fclose() fscanf() fprintf()",
	"1.Text File IO: fopen()fclose() sscanf()sprintf() fgets()fputs()",
	"2.Binary File IO: fopen() fclose() fread() fwrite()",
	"3.Binary File IO: open() close() read() write()",
};
// File I/O ������ ����
static int fio_num = sizeof(fio_types) / sizeof(fio_types[0]);

//*****************************************************************************
//   ���ҽ� ���Ͽ��� �о� �� ������
//   �� ������ DB ������ ��������ε� ����ȴ�.
//*****************************************************************************

typedef struct resource {
	char  name[NAME_LEN];		// ���ҽ� ���� �̸�
	char  program[NAME_LEN];	// ���α׷� �̸�
	float version;				// ���α׷� ����
	int   start_key;			// �ڵ� ������ ���ڵ��� ���� Ű ��
	int   rec_num;				// �ڵ� ������ ���ڵ� ����
	int   fio_type;				// File I/O ����: fprintf, fputs, fwrite, write
} resource_t;

// ����Ʈ ���ҽ� ��
static resource_t resource = {
	"./fio.rc", "fio.c", 1.2, 0, 10, 0,
};

//*****************************************************************************
//   ���ҽ� ���� �а� ȭ�鿡 ����ϱ�
//*****************************************************************************

// ȭ�鿡 ���ҽ� ������ ������
static void
disp_resource(resource_t *r)
{
	print_line_seperator();
	printf("���ҽ� ���� ����\n");
	printf("FileName %s\n",    r->name);
	printf("Program %s\n",     r->program);
	printf("Version %.1f\n",   r->version);
	printf("START_KEY %d\n",   r->start_key);
	printf("REC_NUM %d\n",     r->rec_num);
	printf("FILE_IO_TYPE %s\n",fio_types[r->fio_type]);
	print_line_seperator();
}

// ���ҽ� ���Ϸκ��� ���ҽ� ���� �о��
void
load_resource(resource_t *r)
{
	char line[BUF_LEN], item[LINE_LEN];
	FILE *fp = fopen(r->name, "r");
	if (fp == NULL) {
		printf("���ҽ� ����(%s)�� �� �� ���� \n", r->name);
		exit(-1);
	}

	for (; fgets(line, sizeof(line), fp); ) {
		if (line[0] == '\n' || line[0] == '#')
			continue;
		else if (strncmp(line, "FileName", 2) == 0)
			sscanf(line, "%s %s", item, r->name);
		else if (strncmp(line, "Program", 2) == 0)
			sscanf(line, "%s %s", item, r->program);
		else if (strncmp(line, "Version", 2) == 0)
			sscanf(line, "%s %f", item, &r->version);
		else if (strncmp(line, "START_KEY", 2) == 0)
			sscanf(line, "%s %d", item, &r->start_key);
		else if (strncmp(line, "REC_NUM", 2) == 0)
			sscanf(line, "%s %d", item, &r->rec_num);
		else if (strncmp(line, "FILE_IO_TYPE", 2) == 0)
			sscanf(line, "%s %d", item, &r->fio_type);
		
		fclose(fp);
		
		if ((r->fio_type < 0 || r->fio_type >= fio_num)) {
			printf("���ҽ� ����(%s)�� FILE_IO_TYPE(%d)�� �߸� �Ǿ��� \n",
				r->name, r->fio_type);
			exit(-1);
		}
	}
	disp_resource(r);
}

//*****************************************************************************
// ���ڵ��� ���� �� ���ڵ� ���� ���������� �Լ���
//*****************************************************************************

typedef struct rec {
	int    key;					// �й�
	char   name[12];			// �̸�
	char   dep[16];				// �к�
	int    grade;				// �г�
	char   addr[40];			// �ּ�
} rec_t;

static rec_t *recs;				// ���ڵ���� �迭(�޸� �Ҵ� ����)
static int    key;				// �� �Լ����� ���� ���Ǵ� ������
static rec_t  rec;				// �� �Լ����� ���� ���Ǵ� ������

// ���ڵ带 �����: 
void
disp_rec(rec_t *r)
{
	printf("[%d %s %-14s %d %s]\n", r->key,r->name,r->dep,r->grade,r->addr);
}

//*****************************************************************************
// ���� ���� ������ ����ü �� ��������
//*****************************************************************************

typedef struct head {
	char  name[NAME_LEN];		// ���� �̸�
	int   head_sz;				// ������ ��� ũ��
	int   rec_sz;				// �� ���ڵ� ũ��
	char  program[NAME_LEN];	// ���α׷� �̸�
	float version;				// ���α׷� ����
	int   start_key;			// ���ڵ��� ���� Ű ��
	int   rec_num;				// ���� ���� ���ڵ� ����
	int   fio_type;				// ���� I/O ����
} head_t;

static char *txt_file_name = "./db/db.txt";
static char *bin_file_name = "./db/db.bin";
static char  default_file_name[NAME_LEN];	// �� �� �� �ϳ��� ������ ����

static head_t load_head;
static head_t *head;	// head = &load_head;

static void
set_head(resource_t *r)
{
	head = &load_head;
	head->fio_type = r->fio_type;
	if (head->fio_type < 2) {
		char buf[BUF_LEN];
		sprintf(buf, "%-19s %-2d %-2d %-19s %-3.1f %-6d %-6d %1d\n",
						"", 0, 0, "", 0.0, 0, 0, 0);
		head->head_sz = strlen(buf);
		sprintf(buf, "%-10d %-11s %-15s %1d %-38s\n", 0, "", "", 0, "");
		head->rec_sz  = strlen(buf);
		strcpy(head->name, txt_file_name);
	}
	else {
		head->head_sz = sizeof(head_t);
		head->rec_sz  = sizeof(rec_t);
		strcpy(head->name, bin_file_name);
	}
	strcpy(head->program, r->program);
	head->version   = r->version;
	head->start_key = r->start_key;
	head->rec_num   = r->rec_num;
	strcpy(default_file_name, head->name);
}

void
disp_head(char *pre_msg)
{
	print_line_seperator();
	printf("%s���� ��� ����\n",   pre_msg);
	printf("����I/O ����: %s\n",   fio_types[head->fio_type]);
	printf("���� �̸�   : %s\n",   head->name);
	printf("��� ũ��   : %d\n",   head->head_sz);
	printf("���ڵ� ũ�� : %d\n",   head->rec_sz);
	printf("���α׷�    : %s\n",   head->program);
	printf("����        : %.1f\n", head->version);
	printf("���� Ű ��  : %d\n",   head->start_key);
	printf("���ڵ� ���� : %d\n",   head->rec_num);
	print_line_seperator();
}

//*****************************************************************************
// FILE IO ������ ���� I/O�� ���õ� �Լ���
// open, close, read, write, setpos
//*****************************************************************************

#include "file_io.c"

//===========================================================================
// FILE I/O ������ �Լ� ���̺�
//===========================================================================

typedef struct fio {

	int (*r_open)  (char *);	// �б������ ���� ����: �ҷ�����: ������ ����
	int (*w_open)  (char *);	// ��������� ���� ����: �����ϱ�: ������ ����
	int (*n_open)  (char *);	// �� �̸� ��������� ���� ����: ������ ����
	int (*rw_open) (char *);	// �б�/��������� ���� ����: ������ ����
	int (*rwc_open)(char *);	// �б�/��������� ���� ����: ������ ����
	int (*close)   (void);		// ���� �ݱ�

	int (*read_rec)  (rec_t *);	// ���ڵ� �ϳ� �б�
	int (*write_rec) (rec_t *);	// ���ڵ� �ϳ� ����
	int (*read_head) (head_t *);// ��� �б�
	int (*write_head)(head_t *);// ��� ����
	int (*setpos)(int);			// ������ �б� �Ǵ� ���� ��ġ ����

} fio_t;

static fio_t fios[] = {
	
{ r_open_fp,   w_open_fp,   n_open_fp, rw_open_fp, rwc_open_fp, close_fp,  
  fscanf_rec,  fprintf_rec, fscanf_hd, fprintf_hd, setpos_fp },

{ r_open_fp,   w_open_fp,   n_open_fp, rw_open_fp, rwc_open_fp, close_fp,
  sscanf_rec,  sprintf_rec, sscanf_hd, sprintf_hd, setpos_fp },

{ r_open_fp,   w_open_fp,   n_open_fp, rw_open_fp, rwc_open_fp, close_fp,
  fread_rec,   fwrite_rec,  fread_hd,  fwrite_hd,  setpos_fp },

{ r_open_fd,   w_open_fd,   n_open_fd, rw_open_fd, rwc_open_fd, close_fd,
  read_rec,    write_rec,   read_hd,   write_hd,   setpos_fd },
  
};

static fio_t *fio;	// ���� ���õ� ���� I/O �Լ� ����ü�� ���� ������

static void 
set_fio(int type)
{
	fio = &fios[type];
}

//*****************************************************************************
// �ڵ����� ���ڵ带 �����ϰų� �����ϴ� �Լ���
// update_rec(), create_recs(), alloc_recs(), list_files()
// get_key(), print_line_seperator(), wait_response(), get_int()
//*****************************************************************************

typedef void (*funct_t)(void);

#include "mkrec.c"

//*****************************************************************************
//   ������ ���ڵ带 �޸𸮷� �о� ���ų� ���Ͽ� �����ϴ� �Լ���
//*****************************************************************************

static void
disp_all_recs(char *msg)
{
	for (i = 0; i < head->rec_num; ++i)
		disp_rec(&recs[i]);
	print_line_seperator();
	printf("�� %d���� ���ڵ带 %s\n", head->rec_num, msg);
}

void
make_recs_disp(void)
{
	recs = create_recs(recs, head->start_key, head->rec_num);
	disp_all_recs("�޸𸮿� ���� �����Ͽ���");
}

static void
load(void)
{
	char *name = get_filename("r");
	if (fio->r_open(name) < 0)
		err_wait_jmp("����(%s)�� �� �� ����", name);
	fio->read_head(head);
	recs = alloc_recs(recs, head->rec_num);
	for (i = 0; fio->read_rec(&recs[i]) > 0; ++i) ;
	fio->close();
	disp_head("");
	disp_all_recs("���Ͽ��� �о� ����");
}

static void
save_head_recs(char *name)
{
	if (name)
		strcpy(head->name, name);
	head->fio_type = resource.fio_type;
	disp_head("������ ");
	fio->write_head(head);
	for (i = 0; i < head->rec_num; ++i)
		fio->write_rec(&recs[i]);
	printf("�� %d���� ���ڵ带 ���Ͽ� �����Ͽ���\n", head->rec_num);
}

// �ٸ� �̸����� �����ϱ�
void
save(void)
{
	if (fio->w_open(head->name) < 0)	// ���� �̸����� ����
		err_wait_jmp("����(%s)�� ������ �� ����", head->name);
	save_head_recs(NULL);
	fio->close();
}

// �ٸ� �̸����� �����ϱ�
void
save_as(void)
{
	int  ret;
	char *name = get_filename("n");
	if (name == NULL) 				// ���� �̸��� ���� ���� ���
		return;
	if ((ret = fio->n_open(name)) == -1)	// �ٸ� �̸����� ����
		err_wait_jmp("����(%s)�� ������ �� ����", name);
	else if (ret == -2)
		err_wait_jmp("����(%s)�� �̹� ������", name);
	save_head_recs(name);
	fio->close();
}

void
list_mem(void)
{
	disp_all_recs("�޸𸮿� ������ ����");
}

void
update_mem(void)
{
	update_recs(recs, head->rec_num);
	disp_all_recs("�����Ͽ���");
}

void
make_new_recs(void)
{
	set_head(&resource);
	make_recs_disp();
}


//-----------------------------------------------------------------------------
//   �޸𸮿� �ִ� ���ڵ� �׽�Ʈ�� ���� �޴��� �Լ� ���̺�
//-----------------------------------------------------------------------------

funct_t mem_funct[] = { 
	NULL, load, list_mem, update_mem, make_new_recs, save, save_as,       
};
int mem_fnum = sizeof(mem_funct) / sizeof(mem_funct[0]); 

char *mem_menu[] = {
	"0.return", 
	"1.���Ͽ��� �ҷ�����",  "2.���ڵ� ��ü ����Ʈ", "3.���ڵ� ��ü ����", 
	"4.���ڵ� ���� �����", "5.���Ͽ� �����ϱ�",    "6.�ٸ� �̸����� ����",      
};

void
mem_rec_mng(void)
{
	menu_loop("�޸� ���ڵ� �����ϱ�", mem_menu, mem_funct, mem_fnum);
}


//*****************************************************************************
//   ���Ͽ� �ִ� ���ڵ���� ���� ã��, ����, ����Ʈ�ϱ�
//*****************************************************************************

#define POS(_key) \
	( ( (_key) - head->start_key ) * head->rec_sz + head->head_sz )	

void
list_file(void)
{
	int beg = head->start_key;
	int end = beg + head->rec_num;

	disp_head("");
	for (key = beg; key < end; ++key) {
		fio->setpos(POS(key));
		fio->read_rec(&rec);
		disp_rec(&rec);
	}
	print_line_seperator();
	printf("�� %d���� ���ڵ尡 ���Ͽ� ����\n", head->rec_num);
}

void
search_file(void)
{
	key = get_key("�˻�", head->start_key, head->rec_num);
	fio->setpos(POS(key));
	fio->read_rec(&rec);
	disp_rec(&rec);
}

void
update_file(void)
{
	key = get_key("����", head->start_key, head->rec_num);
	printf("[���� �� ���ڵ�]\n");
	fio->setpos(POS(key));
	fio->read_rec(&rec);
	disp_rec(&rec);

	update_rec(&rec);

	printf("[���� �� ���ڵ�]\n");
	fio->setpos(POS(key));
	fio->write_rec(&rec);
	disp_rec(&rec);
}

// ������ �������� ���� ��� read/write������ ���� ���� ����
int
open_file_rec(char *name)
{
	char line[NAME_LEN];
	
	// read/write������ ���� ���� ����
	if (fio->rw_open(name) >= 0) {
		fio->read_head(head); // ��带 �о� ��
		return(0);
	}
	printf("����(%s)�� �� �� ����. ���� �����ұ��? enter/[n]o? ", name);
	gets(line);
	if (line[0]) // enter�� ������ ���� ���
		return(-1);
	// read/write������ ���� ���� ����
	if (fio->rwc_open(name) < 0) {
		err_wait("����(%s)�� ������ �� ����", name);
		return(-1);
	}
	save_head_recs(name); 		// ���� ���ڵ带 ������
	printf("���ڵ带 �ڵ� �����Ͽ� ���Ͽ� �����Ͽ���\n");
}

void
change_file()
{
	char *name = get_filename("rw");
	fio->close();
	if (open_file_rec(name) < 0)
		fio->rw_open(head->name);
}

//-----------------------------------------------------------------------------
//   ���Ͽ� �ִ� ���ڵ� ���� ���� �׽�Ʈ�� ���� �޴��� �Լ� ���̺�
//-----------------------------------------------------------------------------

funct_t file_funct[] = { 
	NULL, search_file, update_file, list_file, change_file,
};
int file_fnum = sizeof(file_funct) / sizeof(file_funct[0]); 

char *file_menu[] = {
	"0.return", 
	"1.���ڵ� ã��",  "2.���ڵ� ����",   "3.���ڵ� ����Ʈ", "4.���� ���� ����",     
};

void
file_rec_mng(void)
{
	if (open_file_rec(head->name) < 0)
		return;
	menu_loop("������ ���ڵ� ���� �����ϱ�", file_menu, file_funct, file_fnum);
	fio->close();
}

//*****************************************************************************
//   ���� I/O ���� ����
//*****************************************************************************

void
change_fio_type()
{
	int fio_type;

	printf("���� I/O ����: ���� %d\n", head->fio_type);
	for (i = 0; i < fio_num; ++i)
		printf("%s\n", fio_types[i]);
	print_line_seperator();
	resource.fio_type = get_int("������ ������?", NULL, 0, fio_num-1);
	set_head(&resource);
	set_fio(resource.fio_type);
}

//*****************************************************************************
//   main() �Լ�
//*****************************************************************************

//-----------------------------------------------------------------------------
//   main �׽�Ʈ�� ���� �޴��� �Լ� ���̺�
//-----------------------------------------------------------------------------

funct_t main_funct[] = { 
	NULL, mem_rec_mng, file_rec_mng, change_fio_type,
};
int main_fnum = sizeof(main_funct) / sizeof(main_funct[0]); 

char *main_menu[] = {
	"0.exit", 
	"1.�޸� ���ڵ� ����", "2.���� ���ڵ� ���� ����", "3.���� I/O ���� ����",
};

int 
main(int argc, char **argv)
{
	setbuf(stdout, NULL);	// ��� ���۸� �����Ѵ�.
	setbuf(stderr, NULL);

	load_resource(&resource);
	set_head(&resource);
	set_fio(resource.fio_type);
	make_recs_disp();
	menu_loop("���� I/O �׽�Ʈ", main_menu, main_funct, main_fnum);
}
