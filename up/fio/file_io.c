// "fio_bin_fwrite.c"를 여러번 include한 경우 한번만 include하기 위해
#pragma warning(disable:4996)

#ifndef _FILE_IO_C
#define _FILE_IO_C

//*****************************************************************************
//   FILE 열고 닫기 함수들:  C 표준 fopen() fclose() fsetpos()
//*****************************************************************************

static FILE *fp;		//이미 선언되어 있음

int
r_open_fp(char *name) // 파일 불러오기: 읽기용으로 열기
{
	return((fp = fopen(name, "r")) ? 0 : -1);
	// 위 문장은 return( (fp = fopen(name, _____)) != NULL ? 0 : -1 );와 동일함
	// NULL은 (void *)0, 즉 포인터 값(주소 값)이 0임
	// FILE *p = NULL; 이것은 p = (void *)0과 같고, p에는 사실 0이 저장됨
	// if (p)하면, 이 조건문은 p가 0이므로 false임
	// 만약 p가 NULL이 아니면 true임
}


int 
w_open_fp(char *name) // 저장하기: 파일을 쓰기용으로 열기
{ // 파일이 있으면 열고 크기 0로 만듦, 없으면 새로 생성
	return ((fp = fopen(name, "w")) ? 0 : -1);
}

int n_open_fp(char *name) // 다른(새) 이름으로 저장하기: 쓰기용으로 열기
{ // 먼저 읽기용으로 파일 열기
	if ((fp = fopen(name, "r"))) {// 기존에 파일이 존재할 경우 (fp != NULL)
		fclose(fp); // 열린 fp 파일을 닫고
		return(-2); // 동일한 이름의 파일이 이미 존재하므로 에러
	}// 파일이 존재하지 않을 경우(fp == NULL): 쓰기용으로 파일 열기
	return((fp = fopen(name, "w")) ? 0 : -1);
}

int
rw_open_fp(char *name) // 파일을 읽기/쓰기용으로 열기 (파일 없으면 에러)
{ // 파일 레코드 직접 조작 메뉴 선택 시 호출됨
	return((fp = fopen(name, "r+")) ? 0 : -1);
} // 위 함수를 호출하여 파일이 존재하지 않을 경우 아래 함수가 호출됨

int
rwc_open_fp(char *name)		//파일을 읽기/쓰기용으로 열기 (파일 없으면 생성)
{
	return ((fp = fopen(name, "w+")) ? 0 : -1);
}

//	DB 파일을 닫는다.
int
close_fp(void)
{
	return(fclose(fp));
}

int 
setpos_fp(int pos)		// fp 파일의 읽고 쓰는 위치(file position)를 pos로 옮김
{
	return(fseek(fp, pos, SEEK_SET));
}

//===========================================================================
// TEXT FILE I/O 관련 함수들 : C 표준 fscanf() fprintf()
//===========================================================================

static void
trunc_space(char *p)
{
	while (*p) ++p;
	for (--p; isspace((int)*p); --p) ;
	*(++p) = 0;
}

//	DB 파일에서 레코드 하나를 읽어 메모리 r에 저장한다.
int
fscanf_rec(rec_t *r)
{
	if(fscanf(fp, "%d %s %s %d", &r->key, r->name, r->dep, &r->grade, r->addr) <= 0)
		return (0);
	fgetc(fp);

	if(fgets(r->addr, 40, fp) == NULL)
		return (0);
	trunc_space(r->addr);
	return(sizeof(rec_t));
}

//	DB 파일에 레코드 r을 저장한다.
int
fprintf_rec(rec_t *r)
{
	return(fprintf(fp, "%-10d %-11s %-15s %1d %-38s\n",
		r->key, r->name, r->dep, r->grade, r->addr));
}

//	DB 파일에서 각각의 헤드정보를 읽어 해당 변수에 저장한다.
int
fscanf_hd(head_t *h)
{
	return(fscanf(fp, "%s %d %d %s %f %d %d %d\n",
		h->name, &h->head_sz, &h->rec_sz, h->program,
		&h->version, &h->start_key, &h->rec_num, &h->fio_type));
}

//	DB 파일에 각각의 헤드정보 변수 값을 저장한다.
int
fprintf_hd(head_t *h)
{
	return(fprintf(fp, "%-19s %-2d %-2d %-19s %-3.1f %-6d %-6d %1d\n",
		h->name, h->head_sz, h->rec_sz, h->program,
		h->version, h->start_key, h->rec_num, h->fio_type));
}



//===========================================================================
// TEXT FILE I/O 관련 함수들 : C 표준 sscanf() sprintf() fgets() fputs()
//===========================================================================

static char sbuf[BUF_LEN];

//	DB 파일에서 레코드 하나를 읽어 메모리 r에 저장한다.
int
sscanf_rec(rec_t *r)
{
	if(fgets(sbuf, BUF_LEN , fp) == NULL)
		return (0);
	
	sscanf(sbuf, "%d %s %s %d", &r->key, r->name, r->dep, &r->grade);
	strcpy(r->addr, sbuf+41);
	trunc_space(r->addr);
	return(sizeof(rec_t));
}

//	DB 파일에 레코드 r을 저장한다.
int
sprintf_rec(rec_t *r)
{
	sprintf(sbuf, "%-10d %-11s %-15s %1d %-38s\n",
		r->key, r->name, r->dep, r->grade, r->addr);
	return(fputs(sbuf, fp));
}

//	DB 파일에서 각각의 헤드정보를 읽어 해당 변수에 저장한다.
int
sscanf_hd(head_t *h)
{
	if (fgets(sbuf, BUF_LEN, fp) == NULL)
		return (0);

	return(sscanf(sbuf, "%s %d %d %s %f %d %d %d\n",
		h->name, &h->head_sz, &h->rec_sz, h->program,
		&h->version, &h->start_key, &h->rec_num, &h->fio_type));
}

//	DB 파일에 각각의 헤드정보 변수 값을 저장한다.
int
sprintf_hd(head_t *h)
{
	sprintf(sbuf, "%-19s %-2d %-2d %-19s %-3.1f %-6d %-6d %1d\n",
		h->name, &h->head_sz, &h->rec_sz, h->program,
		&h->version, &h->start_key, &h->rec_num, &h->fio_type);
	return(fputs(sbuf, fp));
}

//===========================================================================
// Binary FILE I/O 관련 함수들 : C 표준 fread() fwrite()
//===========================================================================

//	DB 파일에서 레코드 하나를 읽어 메모리 r에 저장한다.
int
fread_rec(rec_t *r)
{
	return(fread(r, sizeof(rec_t), 1, fp));
}

//	DB 파일에 레코드 r을 저장한다.
int
fwrite_rec(rec_t *r)
{
	return(fwrite(r, sizeof(rec_t), 1, fp));
}

//	DB 파일에서 각각의 헤드정보를 읽어 해당 변수에 저장한다.
int
fread_hd(head_t *h)
{
	fread(h->name, sizeof(char), NAME_LEN, fp);
	fread(&h->head_sz, sizeof(int), 1, fp);
	fread(&h->rec_sz, sizeof(int), 1, fp);
	fread(h->program, sizeof(char), NAME_LEN, fp);
	fread(&h->version, sizeof(float), 1, fp);
	fread(&h->start_key, sizeof(int), 1, fp);
	fread(&h->rec_num, sizeof(int), 1, fp);
	fread(&h->fio_type, sizeof(int), 1, fp);

	return (sizeof(head_t));
}

//	DB 파일에 각각의 헤드정보 변수 값을 저장한다.
int
fwrite_hd(head_t *h)
{
	fwrite(h->name, sizeof(char), NAME_LEN, fp);
	fwrite(&h->head_sz, sizeof(int), 1, fp);
	fwrite(&h->rec_sz, sizeof(int), 1, fp);
	fwrite(h->program, sizeof(char), NAME_LEN, fp);
	fwrite(&h->version, sizeof(float), 1, fp);
	fwrite(&h->start_key, sizeof(int), 1, fp);
	fwrite(&h->rec_num, sizeof(int), 1, fp);
	fwrite(&h->fio_type, sizeof(int), 1, fp);
	return (sizeof(head_t));
}


//===========================================================================
// Binary FILE I/O 관련 함수들 : UNIX API open() closde() read() write()
//===========================================================================

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int fd;

int
r_open_fd(char *name)
{
	return ( (fd = open(name, O_RDONLY, 0644)) );
}

int
w_open_fd(char *name)
{
	return( (fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) );
}

int
n_open_fd(char *name)
{
	return( (fd = open(name, O_WRONLY | O_CREAT | O_EXCL, 0644)) ); 
}

int
rw_open_fd(char *name)
{
	return( (fd = open(name, O_RDWR)) );
}

int
rwc_open_fd(char *name)
{
	return ( (fd = open(name, O_CREAT | O_RDWR, 0644)) );
}

//	DB 파일을 닫는다.
int
close_fd(void)
{
	return(close(fd));
}

//	DB 파일에서 레코드 하나를 읽어 메모리 r에 저장한다.
int
read_rec(rec_t *r)
{
	//read(파일기술자, 파일에서 읽어 온 데이터 저장할 메모리 주소, 메모리 크기);
	return (read(fd, r, sizeof(rec_t)) );
}

//	DB 파일에 레코드 r을 저장한다.
int
write_rec(rec_t *r)
{
	//write(파일기술자, 파일에 쓸 메모리 주소, 메모리 크기(파일 쓸 데이터 길이));
	return (write(fd, r, sizeof(rec_t)) );
}

//	DB 파일에서 각각의 헤드정보를 읽어 해당 변수에 저장한다.
// 파일에서 헤드를 읽어와 메모리 h에 저장한다.
int 
read_hd(head_t *h)
{ // write()한 순서대로 멤버들을 읽어 들어야 함
  //read(fd, 파일에서 읽어 온 데이터 저장할 메모리 주소, 메모리 크기);
	read(fd, h->name, NAME_LEN);
	read(fd, &h->head_sz, sizeof(int));
	read(fd, &h->rec_sz, sizeof(int));
	read(fd, h->program, NAME_LEN);
	read(fd, &h->version, sizeof(float));
	read(fd, &h->start_key, sizeof(int));
	read(fd, &h->rec_num, sizeof(int));
	read(fd, &h->fio_type, sizeof(int));
	
	return(sizeof(head_t)); // 에러가 발생하지 않았다고 가정하고 헤드 크기 리턴
}

//	DB 파일에 각각의 헤드정보 변수 값을 저장한다.
int 
write_hd(head_t *h) // 파일에 헤드 h를 저장한다.
{
	// char 배열의 이름은 주소임, int와 float 멤버는 주소를 직접 지정해 주어야 함
	//write(fd, 파일에 쓸 메모리 주소, 메모리 크기(파일에 쓸 데이터 길이));
	write(fd, h->name, NAME_LEN);
	write(fd, &h->head_sz, sizeof(int));
	write(fd, &h->rec_sz, sizeof(int));
	write(fd, h->program, NAME_LEN);
	write(fd, &h->version, sizeof(float));
	write(fd, &h->start_key, sizeof(int));
	write(fd, &h->rec_num, sizeof(int));
	write(fd, &h->fio_type, sizeof(int));
		
	return(sizeof(head_t)); // 에러가 발생하지 않았다고 가정하고 헤드 크기 리턴
}

int 
setpos_fd(int pos)
{
	return(lseek(fd, pos, SEEK_SET));
}

#endif  /* _FILE_IO_C */

