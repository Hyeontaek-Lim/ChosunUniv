// "mkrec.c"를 여러번 include한 경우 한번만 include하기 위해
#ifndef _MKREC_C
#define _MKREC_C

//*****************************************************************************
//   공용으로 사용되는 기본적인 함수들
//*****************************************************************************

static int  i, cnt;				// 각 함수에서 자주 사용되는 변수들

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

// 입력장치에서 정수 하나를 읽어 들임

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
		err_wait_jmp("%d: %s", i, err_msg? err_msg: "지원되지 않는 번호임");
	return(i);
}

//*****************************************************************************
//   공용 함수들
//*****************************************************************************

static int
get_key(char *opr, int start_key, int rec_num)
{
	char msg[LINE_LEN], err[LINE_LEN];
	int end_key = start_key + rec_num - 1;

	sprintf(msg, "%s할 레코드의 키는?", opr);
	sprintf(err, "key 값은 [ %d - %d ]여야 함\n", start_key, end_key);
	return(get_int(msg, err, start_key, end_key));
}

//===========================================================================
// 자동으로 레코드 필드를 생성하거나 수정하는 함수들
//===========================================================================

static char *family[] = {
	"김", "이", "박", "최", "강", "유", "오", "권", "심", "조", 
};
static char *name[] = {
	"재", "홍", "지", "영", "민", "섭", "성", "원", "상", "만",
	"범", "준", "윤", "배", "용", "근", "일", "용", "정", "아",
	"판", "구", "현", "숙", "지", "은", "문", "수", "상", "웅",
};
static char *dep[] = {
	"컴퓨터공학과", "정보통신공학과", "전자공학과", "기계공학과", "금속공학과", 
};
static char *city[] = {
	"광주광역시", "화순군", "순천시", "목포시", "장성군", 
	"해남군",     "영광군", "담양군", "광양시", "보성군" 
};
static char *gu[] = {
	"중구", "북구", "동구", "서구", "남구", 
};

#define R(_m)	( rand() % (_m) )

// 레코드의 학부, 학년, 주소를 변경함 
void
update_rec(rec_t *r)
{
	sprintf(r->dep, "%s", dep[R(5)]);
	r->grade = R(4) + 1;	// 학년
	sprintf(r->addr, "%s %s %s%s동 %d-%d", city[R(10)], gu[R(5)],
			name[R(30)], name[R(30)], R(200)+100, R(50)+10);
}

// 레코드의 각 필드를 자동으로 생성함  
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
	// 랜덤 숫자 발생기의 시드 값 설정: 이 값이 프로그램 실행 때마다 
	srand(time(NULL));		// 달라야 매번 다른 숫자가 발생됨
	recs = alloc_recs(recs, rec_num);
	for (i = 0; i < rec_num; ++i)
		new_rec(&recs[i], start_key+i);
	return(recs);
}

void
update_recs(rec_t *recs, int rec_num)
{
	// 랜덤 숫자 발생기의 시드 값 설정: 이 값이 프로그램 실행 때마다 
	srand(time(NULL));		// 달라야 매번 다른 숫자가 발생됨
	for (i = 0; i < rec_num; ++i)
		update_rec(&recs[i]);
}

//===========================================================================
// 메뉴를 디스플레이하고, 선택된 함수를 호출한다.
//===========================================================================

static void
print_menu(char *title, char *menu[], int num)
{
	int maxlen = 0, len;

	for (i = 1; i < num; ++i)
		if ( (len=strlen(menu[i])) > maxlen ) maxlen = len;

	print_line_seperator();
	printf( "\n%s (파일I/O종류: %d, 파일이름: %s)\n", 
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
		i = get_int("테스트할 번호는?", NULL, 0, fnum-1);
		if (i == 0) break;
		print_line_seperator();
		printf("%s\n", menu[i]);
		funct[i]();
	}
}

//===========================================================================
// 파일 이름 나열하기
//===========================================================================

#include <dirent.h>

static void
get_max_name_len(DIR *dp, int *p_max_name_len, int *p_num_per_line)
{
	struct dirent *dirp;
	int max_name_len = 0; // 가장 긴 파일이름 길이

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
	if ((dp = opendir(dir)) == NULL) {	// 디렉토리 열기
		perror("opendir()");
		return;
	}
	get_max_name_len(dp, &max_name_len, &num_per_line);
	while ((dirp = readdir(dp)) != NULL) {
		if (strstr(dirp->d_name, ext) == NULL) // 확장자가 같지 않으면
			continue;
		printf("%-*s", max_name_len, dirp->d_name);
		if ((++cnt % num_per_line) == 0)
			printf("\n");
	}
	if ((cnt % num_per_line) != 0) 
		printf("\n");
	closedir(dp);						// 디렉토리 닫기
	print_line_seperator();
}

//===========================================================================
// 키보드에서 파일 이름 입력 받기 
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
	if (mode[0] == 'r' || mode[1] == 'w') {	// "r" 또는 "rw"인 경우 
		printf("그냥 엔터를 치면 디폴트 이름이 적용됨\n");
		printf("확장자는 입력하지 않아도 됨\n");
		printf("%s 파일 이름은(%s)? ", 
				mode[0] == 'r'? "불러올":"접근할", default_file_name);
	}
	else {									// 저장하기인 경우
		printf("확장자는 입력하지 말 것\n");
		printf("저장할 파일 이름은? ");
	}
	gets(name);
	if (name[0] == 0)
		return( mode[0] == 'n'? NULL : default_file_name);

	if (mode[0] == 'n') {	// 다른 이름으로 저장하기인 경우
		char *p;
		// 확장자를 잘못 지정한 경우 자동으로 삽입함
		if ( (  is_text && (p=strstr(name, ".bin")) ) ||
			 ( !is_text && (p=strstr(name, ".txt")) ) )
			err_wait_jmp("확장자를 잘못 지정하였음");
	}
	// 확장자를 주지 않은 경우 자동으로 삽입함
	if (strstr(name, ext) == NULL)
		strcat(name, ext);
	sprintf(path_name, "./db/%s", name);
	return(path_name);
}

#endif  /* _MKREC_C */

