#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

#define NAME_LEN	20
#define LINE_LEN	80
#define BUF_LEN		512

static jmp_buf jump;			// setjmp(), longjmp()용으로 사용됨

#define print_line_seperator() \
   printf("----------------------------------------------------------------\n")

// File I/O 종류를 설명하는 문자열의 배열
static char *fio_types[] = {
	"0.Text File IO: fopen() fclose() fscanf() fprintf()",
	"1.Text File IO: fopen()fclose() sscanf()sprintf() fgets()fputs()",
	"2.Binary File IO: fopen() fclose() fread() fwrite()",
	"3.Binary File IO: open() close() read() write()",
};
// File I/O 종류의 개수
static int fio_num = sizeof(fio_types) / sizeof(fio_types[0]);

//*****************************************************************************
//   리소스 파일에서 읽어 온 정보들
//   이 정보는 DB 파일의 헤드정보로도 저장된다.
//*****************************************************************************

typedef struct resource {
	char  name[NAME_LEN];		// 리소스 파일 이름
	char  program[NAME_LEN];	// 프로그램 이름
	float version;				// 프로그램 버전
	int   start_key;			// 자동 생성할 레코드의 시작 키 값
	int   rec_num;				// 자동 생성할 레코드 개수
	int   fio_type;				// File I/O 종류: fprintf, fputs, fwrite, write
} resource_t;

// 디폴트 리소스 값
static resource_t resource = {
	"./fio.rc", "fio.c", 1.2, 0, 10, 0,
};

//*****************************************************************************
//   리소스 파일 읽고 화면에 출력하기
//*****************************************************************************

// 화면에 리소스 정보를 보여줌
static void
disp_resource(resource_t *r)
{
	print_line_seperator();
	printf("리소스 파일 정보\n");
	printf("FileName %s\n",    r->name);
	printf("Program %s\n",     r->program);
	printf("Version %.1f\n",   r->version);
	printf("START_KEY %d\n",   r->start_key);
	printf("REC_NUM %d\n",     r->rec_num);
	printf("FILE_IO_TYPE %s\n",fio_types[r->fio_type]);
	print_line_seperator();
}

// 리소스 파일로부터 리소스 정보 읽어옴
void
load_resource(resource_t *r)
{
	char line[BUF_LEN], item[LINE_LEN];
	FILE *fp = fopen(r->name, "r");
	if (fp == NULL) {
		printf("리소스 파일(%s)을 열 수 없음 \n", r->name);
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
			printf("리소스 파일(%s)의 FILE_IO_TYPE(%d)이 잘못 되었음 \n",
				r->name, r->fio_type);
			exit(-1);
		}
	}
	disp_resource(r);
}

//*****************************************************************************
// 레코드의 정의 및 레코드 관련 전역변수와 함수들
//*****************************************************************************

typedef struct rec {
	int    key;					// 학번
	char   name[12];			// 이름
	char   dep[16];				// 학부
	int    grade;				// 학년
	char   addr[40];			// 주소
} rec_t;

static rec_t *recs;				// 레코드들의 배열(메모리 할당 받음)
static int    key;				// 각 함수에서 자주 사용되는 변수들
static rec_t  rec;				// 각 함수에서 자주 사용되는 변수들

// 레코드를 출력함: 
void
disp_rec(rec_t *r)
{
	printf("[%d %s %-14s %d %s]\n", r->key,r->name,r->dep,r->grade,r->addr);
}

//*****************************************************************************
// 파일 헤드와 관련한 구조체 및 전역변수
//*****************************************************************************

typedef struct head {
	char  name[NAME_LEN];		// 파일 이름
	int   head_sz;				// 파일의 헤드 크기
	int   rec_sz;				// 한 레코드 크기
	char  program[NAME_LEN];	// 프로그램 이름
	float version;				// 프로그램 버전
	int   start_key;			// 레코드의 시작 키 값
	int   rec_num;				// 파일 내의 레코드 개수
	int   fio_type;				// 파일 I/O 종류
} head_t;

static char *txt_file_name = "./db/db.txt";
static char *bin_file_name = "./db/db.bin";
static char  default_file_name[NAME_LEN];	// 위 둘 중 하나를 가지고 있음

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
	printf("%s파일 헤드 정보\n",   pre_msg);
	printf("파일I/O 종류: %s\n",   fio_types[head->fio_type]);
	printf("파일 이름   : %s\n",   head->name);
	printf("헤드 크기   : %d\n",   head->head_sz);
	printf("레코드 크기 : %d\n",   head->rec_sz);
	printf("프로그램    : %s\n",   head->program);
	printf("버전        : %.1f\n", head->version);
	printf("시작 키 값  : %d\n",   head->start_key);
	printf("레코드 개수 : %d\n",   head->rec_num);
	print_line_seperator();
}

//*****************************************************************************
// FILE IO 종류별 파일 I/O와 관련된 함수들
// open, close, read, write, setpos
//*****************************************************************************

#include "file_io.c"

//===========================================================================
// FILE I/O 종류별 함수 테이블
//===========================================================================

typedef struct fio {

	int (*r_open)  (char *);	// 읽기용으로 파일 열기: 불러오기: 없으면 에러
	int (*w_open)  (char *);	// 쓰기용으로 파일 열기: 저장하기: 없으면 생성
	int (*n_open)  (char *);	// 새 이름 저장용으로 파일 열기: 있으면 에러
	int (*rw_open) (char *);	// 읽기/쓰기용으로 파일 열기: 없으면 에러
	int (*rwc_open)(char *);	// 읽기/쓰기용으로 파일 열기: 없으면 생성
	int (*close)   (void);		// 파일 닫기

	int (*read_rec)  (rec_t *);	// 레코드 하나 읽기
	int (*write_rec) (rec_t *);	// 레코드 하나 쓰기
	int (*read_head) (head_t *);// 헤드 읽기
	int (*write_head)(head_t *);// 헤드 쓰기
	int (*setpos)(int);			// 파일의 읽기 또는 쓰기 위치 설정

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

static fio_t *fio;	// 현재 선택된 파일 I/O 함수 구조체에 대한 포인터

static void 
set_fio(int type)
{
	fio = &fios[type];
}

//*****************************************************************************
// 자동으로 레코드를 생성하거나 수정하는 함수들
// update_rec(), create_recs(), alloc_recs(), list_files()
// get_key(), print_line_seperator(), wait_response(), get_int()
//*****************************************************************************

typedef void (*funct_t)(void);

#include "mkrec.c"

//*****************************************************************************
//   파일의 레코드를 메모리로 읽어 오거나 파일에 저장하는 함수들
//*****************************************************************************

static void
disp_all_recs(char *msg)
{
	for (i = 0; i < head->rec_num; ++i)
		disp_rec(&recs[i]);
	print_line_seperator();
	printf("총 %d개의 레코드를 %s\n", head->rec_num, msg);
}

void
make_recs_disp(void)
{
	recs = create_recs(recs, head->start_key, head->rec_num);
	disp_all_recs("메모리에 새로 생성하였음");
}

static void
load(void)
{
	char *name = get_filename("r");
	if (fio->r_open(name) < 0)
		err_wait_jmp("파일(%s)을 열 수 없음", name);
	fio->read_head(head);
	recs = alloc_recs(recs, head->rec_num);
	for (i = 0; fio->read_rec(&recs[i]) > 0; ++i) ;
	fio->close();
	disp_head("");
	disp_all_recs("파일에서 읽어 왔음");
}

static void
save_head_recs(char *name)
{
	if (name)
		strcpy(head->name, name);
	head->fio_type = resource.fio_type;
	disp_head("저장한 ");
	fio->write_head(head);
	for (i = 0; i < head->rec_num; ++i)
		fio->write_rec(&recs[i]);
	printf("총 %d개의 레코드를 파일에 저장하였음\n", head->rec_num);
}

// 다른 이름으로 저장하기
void
save(void)
{
	if (fio->w_open(head->name) < 0)	// 기존 이름으로 저장
		err_wait_jmp("파일(%s)을 생성할 수 없음", head->name);
	save_head_recs(NULL);
	fio->close();
}

// 다른 이름으로 저장하기
void
save_as(void)
{
	int  ret;
	char *name = get_filename("n");
	if (name == NULL) 				// 파일 이름을 주지 않은 경우
		return;
	if ((ret = fio->n_open(name)) == -1)	// 다른 이름으로 저장
		err_wait_jmp("파일(%s)을 생성할 수 없음", name);
	else if (ret == -2)
		err_wait_jmp("파일(%s)이 이미 존재함", name);
	save_head_recs(name);
	fio->close();
}

void
list_mem(void)
{
	disp_all_recs("메모리에 가지고 있음");
}

void
update_mem(void)
{
	update_recs(recs, head->rec_num);
	disp_all_recs("수정하였음");
}

void
make_new_recs(void)
{
	set_head(&resource);
	make_recs_disp();
}


//-----------------------------------------------------------------------------
//   메모리에 있는 레코드 테스트를 위한 메뉴와 함수 테이블
//-----------------------------------------------------------------------------

funct_t mem_funct[] = { 
	NULL, load, list_mem, update_mem, make_new_recs, save, save_as,       
};
int mem_fnum = sizeof(mem_funct) / sizeof(mem_funct[0]); 

char *mem_menu[] = {
	"0.return", 
	"1.파일에서 불러오기",  "2.레코드 전체 리스트", "3.레코드 전체 수정", 
	"4.레코드 새로 만들기", "5.파일에 저장하기",    "6.다른 이름으로 저장",      
};

void
mem_rec_mng(void)
{
	menu_loop("메모리 레코드 조작하기", mem_menu, mem_funct, mem_fnum);
}


//*****************************************************************************
//   파일에 있는 레코드들을 직접 찾기, 수정, 리스트하기
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
	printf("총 %d개의 레코드가 파일에 있음\n", head->rec_num);
}

void
search_file(void)
{
	key = get_key("검색", head->start_key, head->rec_num);
	fio->setpos(POS(key));
	fio->read_rec(&rec);
	disp_rec(&rec);
}

void
update_file(void)
{
	key = get_key("수정", head->start_key, head->rec_num);
	printf("[수정 전 레코드]\n");
	fio->setpos(POS(key));
	fio->read_rec(&rec);
	disp_rec(&rec);

	update_rec(&rec);

	printf("[수정 후 레코드]\n");
	fio->setpos(POS(key));
	fio->write_rec(&rec);
	disp_rec(&rec);
}

// 파일이 존재하지 않을 경우 read/write용으로 파일 새로 생성
int
open_file_rec(char *name)
{
	char line[NAME_LEN];
	
	// read/write용으로 기존 파일 열기
	if (fio->rw_open(name) >= 0) {
		fio->read_head(head); // 헤드를 읽어 옴
		return(0);
	}
	printf("파일(%s)을 열 수 없음. 새로 생성할까요? enter/[n]o? ", name);
	gets(line);
	if (line[0]) // enter를 누르지 않은 경우
		return(-1);
	// read/write용으로 파일 새로 생성
	if (fio->rwc_open(name) < 0) {
		err_wait("파일(%s)을 생성할 수 없음", name);
		return(-1);
	}
	save_head_recs(name); 		// 헤드와 레코드를 저장함
	printf("레코드를 자동 생성하여 파일에 저장하였음\n");
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
//   파일에 있는 레코드 직접 조작 테스트를 위한 메뉴와 함수 테이블
//-----------------------------------------------------------------------------

funct_t file_funct[] = { 
	NULL, search_file, update_file, list_file, change_file,
};
int file_fnum = sizeof(file_funct) / sizeof(file_funct[0]); 

char *file_menu[] = {
	"0.return", 
	"1.레코드 찾기",  "2.레코드 수정",   "3.레코드 리스트", "4.접근 파일 변경",     
};

void
file_rec_mng(void)
{
	if (open_file_rec(head->name) < 0)
		return;
	menu_loop("파일의 레코드 직접 접근하기", file_menu, file_funct, file_fnum);
	fio->close();
}

//*****************************************************************************
//   파일 I/O 종류 변경
//*****************************************************************************

void
change_fio_type()
{
	int fio_type;

	printf("파일 I/O 종류: 현재 %d\n", head->fio_type);
	for (i = 0; i < fio_num; ++i)
		printf("%s\n", fio_types[i]);
	print_line_seperator();
	resource.fio_type = get_int("변경할 종류는?", NULL, 0, fio_num-1);
	set_head(&resource);
	set_fio(resource.fio_type);
}

//*****************************************************************************
//   main() 함수
//*****************************************************************************

//-----------------------------------------------------------------------------
//   main 테스트를 위한 메뉴와 함수 테이블
//-----------------------------------------------------------------------------

funct_t main_funct[] = { 
	NULL, mem_rec_mng, file_rec_mng, change_fio_type,
};
int main_fnum = sizeof(main_funct) / sizeof(main_funct[0]); 

char *main_menu[] = {
	"0.exit", 
	"1.메모리 레코드 조작", "2.파일 레코드 직접 조작", "3.파일 I/O 종류 변경",
};

int 
main(int argc, char **argv)
{
	setbuf(stdout, NULL);	// 출력 버퍼를 제거한다.
	setbuf(stderr, NULL);

	load_resource(&resource);
	set_head(&resource);
	set_fio(resource.fio_type);
	make_recs_disp();
	menu_loop("파일 I/O 테스트", main_menu, main_funct, main_fnum);
}
