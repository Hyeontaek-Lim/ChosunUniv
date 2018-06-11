#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <utime.h>
#include <sys/wait.h>
#include <setjmp.h> 	//for setjmp()

#define SZ_STR_BUF		256		// �Ϲ� ���ڿ� �迭 ����
#define SZ_FILE_BUF		1024 	// For file I/O need Array_size

// get_argv_optv() �Լ����� ���� ������
char *cmd;
char *argv[100];
char *optv[10];
int  argc, optc;
char cur_work_dir[SZ_STR_BUF]; //present word directory name saved buffer.  
jmp_buf jump;
int cmd_idx;


//***************************************************************************
// 	��ũ�� �Լ�
//***************************************************************************
// ���� ���ڵ��� �ϳ��� ����ó�� ����ϱ� ���� ���ǹ�
// #define���� �Լ�ó�� �ۼ��� ���� ����
// ��ũ�� �Լ��� ��¥ �Լ��� �ƴ϶�, �� ��ũ�� �Լ��� ȣ��(���)�� ���� 
//	 ��ũ�� ����(do { ... } while(0))�� ������ �� ���Ե�.
// �Ϲ� ����� ��ü�Ǵ� �Ͱ� ������.

// #define ������ �ݵ�� �� �ٿ� �� �ۼ��ؾ� ��
//   ���� �Ʒ� �� ���� ���� \�� ���� ���� �� �ٷ� ����� �ִ� ������ ��
//   �Ʒ��� \ �ڿ��� �ݵ�� �ٷ� ���͸� �ľ� �ϰ�, 
//   �����̽��� ������ ���� �߻���
//
// �Ʒ� ��ũ�� �Լ����� return�� ��ũ�� �Լ� ��ü���� ������ �ƴ϶�, 
//	 �� ��ũ�θ� ȣ���� ���� �ش� �Լ����� �����Ѵٴ� �ǹ���.
//	 ��, cp() �Լ����� PRINT_ERR_RET()�� ȣ���ߴٸ� �Ʒ� return��
//	 cp() �Լ����� �����Ѵٴ� �ǹ���.
//--------------------------------------------------------------------------

// ���� ������ ȭ�鿡 ����ϰ� �� ��ũ�θ� ����ϴ� �ش� �Լ����� ������
/* cmd�� "ls"��� "ls: ��������" ���·� ��µ� */  


#define EQUAL(_s1, _s2) 	(strcmp(_s1, _s2) == 0) // �ΰ� ���ڿ� ������ true
#define NOT_EQUAL(_s1, _s2)	(strcmp(_s1, _s2) != 0)	// �ΰ� ���ڿ� �ٸ��� true

// ��ɾ� ������ �����
// help()�� proc_cmd()���� ȣ���
static void
print_usage(char *msg, char *cmd, char *opt, char *arg)
{
	// proc_cmd()���� msg�� "����: "��, 
	// help()    ���� msg�� "    "  �� �Ѱ� ��
	printf("%s%s", msg, cmd);	// "����: ls"
	if (NOT_EQUAL(opt, "")) 	// �ɼ��� ������
		printf("  %s", opt);	// " -l"
	printf("  %s\n", arg);		// " [���丮 �̸�]"
	// ���� ��� ��) "����: ls  -l  [���丮 �̸�]\n"
}

//***************************************************************************
//  ��ɾ� �ɼ� �� ���� ������ ��Ȯ���� üũ�ϴ� �۾��� ������
//***************************************************************************

// [��ɾ� ������ ����]�� ��Ȯ���� üũ��
// argc: ����ڰ� �Է��� ��ɾ� ������ ��
// count: �� ��ɾ �ʿ�� �ϴ� ������ ��
// ���ϰ�: ���ڰ����� ��Ȯ�ϸ� 0, Ʋ������ -1
void
check_arg(int count)
{
	if(count < 0){
		count = -count;
		if (argc <= count)
			return;
	}
	if(argc == count)
		return;
	if(argc > count)
		 printf("Too many arguments. \n");
	else
		printf("Insufficient arguments. \n");
	longjmp(jump, -2);	

}

// ��ɾ�� ��Ȯ�� [�ɼ�]�� �־������� üũ��
// optc: ����ڰ� �Է��� �ɼ� ����
// optv[i]: ����ڰ� �Է��� �ɼ� ���ڿ�
// opt: �� ��ɾ �ʿ�� �ϴ� �ɼ� ���ڿ�
// ���ϰ�: �ɼ��� ������ 0, ������ 1, �ɼ��� Ʋ������ -1
void
check_opt(char *opt)
{
	int i, err = 0;

	for(i = 0; i < optc; ++i) {
		if (NOT_EQUAL(opt, optv[i])) {
			printf("Not supported option(%s). \n", optv[i]);
			err = -1;
		}
	}
	if (err) longjmp(jump, -2);
}

//***************************************************************************
// 	get_argv_optv(): ��ɾ� ������ ��ɾ�,�ɼ�,������ڸ� �� �ܾ�� �и�
//***************************************************************************
//
// ��������
// char *cmd; 		// ��ɾ� ���ڿ�
// char *argv[100];	// ��ɾ� ���� ���ڿ��� �����ּ� ����
// char *optv[10];	// ��ɾ� �ɼ� ���ڿ��� �����ּ� ����
// int optc; // �ɼ� ��ū ��, ��, optv[]�� ����� ������ ����
// int argc; // ��ɾ�� �ɼ��� ������ ��ɾ� ������ ��
//			 // ��, argv[]�� ����� ������ ����

// �Էµ� ��ɾ� �� ��ü�� �����ϰ� �ִ� cmd_line[]����
// �� ��ū(�ܾ�)�� ������ ���ڿ��� �ڸ� ��, �� ��ū�� ���� �ּҸ� 
// ��ɾ��� ��� cmd��, �ɼ��� ��� optv[]��, 
// ��ɾ� ������ ��� argv[]�� ���� ���������� �����Ѵ�.

// ����: get_argv_optv(cmd_line)�� ȣ���Ͽ� ������ ���� �� ������ ��
//
// cmd_line[]�� "ls -l pr4" �� ����Ǿ� ���� ���
// cmd -> "ls"
// optc = 1, optv[0] -> "-l";
// argc = 1, argv[0] -> "pr4";

// cmd_line[]�� "ln -s f1 f2" �� ����Ǿ� ���� ���
// cmd -> "ln"
// optc = 1, optv[0] -> "-s";
// argc = 2, argv[0] -> "f1", argv[1] -> "f1";

// cmd_line[]�� "ln f1 f2" �� ����Ǿ� ���� ���
// cmd -> "ln"
// optc = 0;
// argc = 2, argv[0] -> "f1", argv[1] -> "f2";

// cmd_line[]�� "pwd" �� ����Ǿ� ���� ���
// cmd -> "pwd"
// optc = 0;
// argc = 0;

// cmd, optv[], argv[]�� ����Ǵ� ���� �� ���ڿ��� �����ּ�, 
// ��, �� ���ڿ��� ù ������ �ּҰ� �����
// ���� �� ���ڿ��� ������ cmd_line[]�� ����Ǿ� ����

static char *
get_argv_optv(char *cmd_line)
{
	char *tok;

	argc = optc = 0;

	if ((cmd = strtok(cmd_line, " \t\n")) == NULL)
		return (NULL);

	for( ; (tok = strtok(NULL, " \t\n")) != NULL; ) {

		if (tok[0] == '-')
			optv[optc++] = tok;
		else
			argv[argc++] = tok;
	}
	return (cmd);
}

static void
print_attr(char *path, char *fn)
{
	struct passwd *pwp;
	struct group *grp;
	struct stat st_buf;
	char full_path[SZ_STR_BUF], buf[SZ_STR_BUF], c;
	char time_buf[13];
	struct tm *tmp;

	sprintf(full_path, "%s/%s", path, fn);
	if(lstat(full_path, &st_buf) < 0)
		longjmp(jump, -1);
	
	if(S_ISREG(st_buf.st_mode))
		c = '-';
	else if(S_ISDIR(st_buf.st_mode))
		c = 'd';
	else if(S_ISCHR(st_buf.st_mode))
		c = 'c';
	else if(S_ISBLK(st_buf.st_mode))
		c = 'b';
	else if(S_ISFIFO(st_buf.st_mode))
		c = 'f';
	else if(S_ISLNK(st_buf.st_mode))
		c = 'l';
	else if(S_ISSOCK(st_buf.st_mode))
		c = 's';
	buf[0] = c;
	buf[1] = (st_buf.st_mode & S_IRUSR)? 'r': '-';
	buf[2] = (st_buf.st_mode & S_IWUSR)? 'w': '-';
	buf[3] = (st_buf.st_mode & S_IXUSR)? 'x': '-';
	buf[4] = (st_buf.st_mode & S_IRGRP)? 'r': '-';
	buf[5] = (st_buf.st_mode & S_IWGRP)? 'w': '-';
	buf[6] = (st_buf.st_mode & S_IXGRP)? 'x': '-';
	buf[7] = (st_buf.st_mode & S_IRGRP)? 'r': '-';
	buf[8] = (st_buf.st_mode & S_IWGRP)? 'w': '-';
	buf[9] = (st_buf.st_mode & S_IXGRP)? 'x': '-';
	buf[10] = '\0';
	pwp = getpwuid(st_buf.st_uid);
	grp = getgrgid(st_buf.st_gid);
	tmp = localtime(&st_buf.st_mtime);
	strftime(time_buf, 13, "%b %d %H:%M", tmp);

	sprintf(buf+10, "%3ld %-8s %-8s %8ld %s %s",
					st_buf.st_nlink, pwp->pw_name, grp->gr_name,
					st_buf.st_size, time_buf, fn);
	if(S_ISLNK(st_buf.st_mode)) {
		int len, bytes;
		strcat(buf, " -> ");
		len = strlen(buf);
		bytes = readlink(full_path, buf+len, SZ_STR_BUF-len);
		buf[len+bytes] = '\0';
	}
	printf("%s \n", buf);
}

static void
print_detail(DIR *dp, char *path)
{
	struct dirent *dirp;

	while((dirp = readdir(dp)) != NULL)
		print_attr(path, dirp->d_name);
	closedir(dp);
}

static void
get_max_name_len(DIR *dp, int *p_max_name_len, int *p_num_per_line)
{
	struct dirent *dirp;
	int max_name_len = 0;

	while((dirp = readdir(dp)) != NULL) {
		int name_len = strlen(dirp->d_name);
		if(name_len > max_name_len)
			max_name_len = name_len;
	}

	rewinddir(dp);

	max_name_len += 4;

	*p_num_per_line = 80 / max_name_len;
	*p_max_name_len = max_name_len;
}

static void
print_name(DIR *dp)
{
	struct dirent *dirp;
	int max_name_len, num_per_line, cnt = 0;

	get_max_name_len(dp, &max_name_len, &num_per_line);

	while((dirp = readdir(dp)) != NULL) {
		printf("%-*s", max_name_len, dirp->d_name);

		if((++cnt % num_per_line) == 0)
			printf("\n");
	}

	if((cnt % num_per_line) != 0)
		printf("\n");
}



	//  ����ڰ� Ű���忡�� �Է��� ��ɾ� �� ���� cmd_line[]�� ����Ǿ� ����
	//  cmd_line[]�� "ln -s file1 file2"�� ����Ǿ� �ִٰ� ��������.
	//
	//  strtok()�� �ι�° ������ " \t\n"�� ��ū(�ܾ�)�� �����ϴ� ��������.
	//	��, �����̽� ' ', �� '\t', ���� '\n' ���ڰ� ������ ��ū�� �ڸ�.
	//  strtok() �Լ��� cmd_line[]�� �ִ� ���ڿ�(��ü�� �ϳ��� ���ڿ�)����
	// 	ù �ܾ� "ln"�� ã�� �ܾ��� ���� null����('\0')�� ������ 
	// 	�ܾ �ϳ��� ���ڿ��� �����(�ܾ �ڸ��ٰ� ǥ����), 
	//	�� ���ڿ��� ù ���� �ּҸ� ������.
	//  ù �ܾ �߶� ���� ���� strtok() ȣ�� �� ù ���ڷ� cmd_line�� �ְ�, 
	//	������ �� ȣ���� �� ù ���ڷ� NULL�� �ִµ�,
	//	�� ��� �տ��� ó���� �ܾ��� �� ���� �ܾ ã�� �ڸ���.
	// 	���� �Լ� ȣ�� �� �� �̻� ó���� �ܾ ���ٸ� NULL�� ������.
	//
	//  �ᱹ �Լ� ȣ�� [��]���� cmd_line[]�� "ln\0-s\0file1\0file2"�� ����
	//	'\0'�� null����(�� ������)�� ǥ���� ���̰�,
	//	�޸𸮿��� ASCI �ڵ� 0(������)�� �� byte�� ����Ǿ� ����. 
	//	��� ���ڿ� ������ �� ���ڰ� ����(�Լ����� �ڵ����� ������ ��)
	//  ��, cmd_line[]���� �װ��� ���ڿ��� ����Ǿ� �ְ�,
	//	�� ���ڿ��� ���� �ּҴ� optv[]�� argv[]�� �и��Ǿ� ����Ǿ� ����


//***************************************************************************
//
// 	���⼭���� �� ��ɾ� ���� ����
//
//

//cd funtion
void cd(void)
{
	if(argc == 0)
	{
		struct passwd *pwp = getpwuid(getuid());
		argv[0] = pwp->pw_dir;
	}

	if(chdir(argv[0]) < 0)
		longjmp(jump, -1);
	else
		getcwd(cur_work_dir, SZ_STR_BUF);

}


void
cp(void)
{
	int rfd, wfd, len;
	char buf[SZ_FILE_BUF];
	struct stat st_buf;

	if((stat(argv[0], &st_buf) < 0) ||
		((rfd = open(argv[0], O_RDONLY)) < 0) ||
		((wfd = creat(argv[1], st_buf.st_mode)) < 0))
		longjmp(jump, -1);

	while ((len = read(rfd, buf, SZ_FILE_BUF)) > 0) {
		if(write(wfd, buf, len) != len) {
			len = -1;
			break;
		}
	}

	if(len < 0)
		perror(cmd);
	
	close(rfd);
	close(wfd);
}

// echo ������ �Էµ� ���ڿ��� ȭ�鿡 �ٷ� echo�� �ִ� ��ɾ�
// ����: echo [echo�� ���ڿ� �Է�: ���� ���� ����]
//   ��) echo I love you. 
// argv[0] -> "I"
// argv[1] -> "love"
// argv[2] -> "you."
// argc = 3; ������ ������ �� ��ū(�ܾ�)�� ��
void
echo(void)
{
	int i;

	for(i = 0; i < argc; i++) {
		printf("%s", argv[i]);
		printf("\n");
	}
}

//id function
void id(void)
{
	struct passwd *pwp;
	struct group *grp;
	
	pwp = (argc) ? getpwnam(argv[0]) : getpwuid(getuid());

	if( (pwp == NULL) || ((grp = getgrgid(pwp->pw_gid)) == NULL) )
		printf("id : non efficient user name : \"%s\" \n", argv[0]);	
	
	else
		printf("uid=%ld(%s) gid=%ld(%s) \n", pwp->pw_uid, pwp->pw_name, grp->gr_gid, grp->gr_name);

}

// ���丮 ���� ���� �̸��� ���� �ִ� ��ɾ�
// ����: ls [-l] [���丮 �̸�]
//		"-l" �ɼ��� ������ ���� �̸���, ������ ������ �������� ���� ��
//		[���丮 �̸�]�� ������ ���� ���丮�� �ǹ���
// argv[0] -> "���丮�̸�"; [���丮 �̸�]�� �� ���
// argc = [���丮 �̸�]�� �� ��� 1, �� �� ��� 0
// optc = "-l" �ɼ��� �� ��� 1, ���� �ʾ��� ��� 0

void
ls(void)
{
	char *path;
	DIR *dp;

	path = (argc == 0) ? ".": argv[0];

	if((dp = opendir(path)) == NULL)
		longjmp(jump, -1);
	if(optc == 0)
		print_name(dp);
	else
		print_detail(dp, path);
	closedir(dp);
}

//ln function
void ln(void)
{
	if( ( (optc == 1) ? symlink(argv[0], argv[1]) : link(argv[0], argv[1]) ) < 0 )
		longjmp(jump, -1);
}

//mv function
void mv(void)
{
	if( (link(argv[0], argv[1]) < 0) || (unlink(argv[0]) < 0) )
		longjmp(jump, -1);
}

//makedir function
void makedir(void)
{
	if(mkdir(argv[0], 0755) < 0)
		longjmp(jump, -1);
}

//rm function
void removedir(void)
{
	if(rmdir(argv[0]) < 0)
		longjmp(jump, -1);
}

//pwd function
void
pwd(void)
{
	printf("%s \n", cur_work_dir);
}

// �ϳ��� ������ �����ϴ� ��ɾ�
// ����: rm  �����̸�
// argv[0] -> "�����̸�"
void
rm(void)
{
	struct stat buf;

	if( (lstat(argv[0], &buf) < 0) || ( (S_ISDIR(buf.st_mode)) ? (rmdir(argv[0])) : (unlink(argv[0])) ))
		longjmp(jump, -1);
}

//date function
void date(void)
{
	char stm[128];
	time_t ttm = time(NULL);
	struct tm *ltm = localtime(&ttm);
		
	printf("ttm: %s", asctime(ltm));
	printf("ctm: %s", ctime(&ttm));

	strftime(stm, 128, "%Y�� %m�� %d�� %p %l�� %M�� %S��", ltm);
	printf("stm: %s \n", stm);
	strftime(stm, 128, "%d %H %M %S %b %Y %a ", ltm);
	printf("temp : %s \n", stm);
	
}

// ���α׷��� �����ϴ� ��ɾ�
// ����: exit
void
quit(void)
{
	exit(0);
}

void hostname(void)
{
	char hostname[SZ_STR_BUF];
	
	gethostname(hostname, SZ_STR_BUF);
	printf("%s \n", hostname);
}

//chmod function
void changemod(void)
{
	int mode;  		// %o : octal
	sscanf(argv[0], "%o", &mode);

	if( chmod(argv[1], mode) < 0 )
		longjmp(jump, -1);
}

//unixname function
void unixname(void) {
	struct utsname un;

	uname(&un);

	printf("%s", un.sysname);

	if(optc == 1)
		printf("%s %s %s %s", un.nodename, un.release, un.version, un.machine);
	
	printf("\n");

}

//whoami function
void whoami(void) {
	/*
	char* username;
	username = getlogin();

	if(username != NULL)
		printf("s", pwp->pw_name);
	else
		printf("Not terminal equiment, Not userInfo. \n");
	*/

	struct passwd *pwp = getpwuid(getuid());
	printf("%s \n", pwp->pw_name); 

}

//touch function
void touch(void)
{
	int fd; 

	if( (fd = utime(argv[0], NULL)) == 0)
		return;

	else {
		if( (errno != ENOENT) || (fd = creat(argv[0], 0644)) < 0 )
			longjmp(jump, -1);
	}

	close(fd);

}

//cat function
void cat(void)
{
	int fd, len;
	char buf[SZ_FILE_BUF];

	if((fd = open(argv[0], O_RDONLY)) < 0)
		longjmp(jump, -1);	

	while ((len = read(fd, buf, SZ_FILE_BUF)) > 0) {
		if (write(STDOUT_FILENO, buf, len) != len) {
			len = -1;
			break;
		}
	}

	if(len < 0)
		perror(cmd);
	
	close(fd);
}


//sleep function
void Sleep(void)
{
	int sec;
	sscanf(argv[0], "%d", &sec);

	sleep(sec); 		//unistd.h 	include
}




//
// �� ��ɾ� ���� ��
//
//***************************************************************************

// ��� �Լ��� �� �Լ��� ȣ���ϰų� �Լ� �̸��� ���Ǳ� ���� ���� 
// ����ǰų� ���ǵǾ�� �Ѵ�. ���� �Ʒ� cmd_tbl[]�� �� �迭 ���ҿ��� 
// ��ɾ� ó���Լ� �̸��� ����ϱ� ������ �� �Լ����� �Ʒ� cmd_tbl[] ���� 
// ���� �Լ����ǰ� �Ǿ�� �ϰ� ���� �տ� �̹� ���� �Ǿ���.
// �׷��� help() �Լ��� ���Ǵ� cmd_tbl[] �ڿ� �ֱ� ������, cmd_tbl[] �տ� 
// �� �Լ��� �Ʒ�ó�� �̸� �����ؾ� �Ѵ�. help() �Լ����Ǵ� cmd_tbl[]�� 
// ���� ����ϱ� ������ ��¿ �� ���� cmd_tbl[] �ڿ� �־�� �Ѵ�.

//***************************************************************************
//  �� ��ɾ �� ������ ������ ����ü �迭 cmd_tbl[]
//***************************************************************************

#define AC_LESS_1		-1		// ��ɾ� ���� ������ 0 �Ǵ� 1�� ���
#define AC_ANY			-100	// ��ɾ� ���� ������ ���� ���� ���(echo)

// �ϳ��� ��ɾ ���� ���� ����ü
typedef struct {
	char *cmd;			// ��ɾ� ���ڿ�
	void (*func)(void);	// ��ɾ ó���ϴ� �Լ� ���� �ּ�(�Լ� �̸�)
	int  argc;			// ��ɾ� ������ ����(��ɾ�� �ɼ��� ����)
	char *opt;			// �ɼ� ���ڿ�: ��) "-a", �ɼ� ������ ""
	char *arg;			// ��ɾ� ���� ����� �� ����� ��ɾ� ����: 
} cmd_tbl_t;			//		��) cp�� ��� "�������� ��������""

void help(void);

cmd_tbl_t cmd_tbl[] = {
//	  ��ɾ�		��ɾ�		��ɾ�				��ɾ� ���� ��� ��
//	   �̸�			ó��		����		��ɾ�	����� ��ɾ�
//	  ���ڿ� 		�Լ��̸�	����		�ɼ�	����
	{ "cp",			cp,			2,			"",		"��������  ���������" },
	{ "cd",			cd,			AC_LESS_1,	"",		"[���丮�̸�]" },
	{ "chmod", 		changemod, 	2,			"", 	"8����mode �����̸�" },
	{ "echo",		echo,		AC_ANY,		"",		"[������ ����]" },
	{ "help",		help,		0,			"",		"" },
	{ "hostname",	hostname,	0,			"",		"" },
	{ "id",			id, 		AC_LESS_1,	"",		"[����ڰ���]" },		
	{ "ls",			ls,			AC_LESS_1,	"-l",	"[���丮�̸�]" },
	{ "ln",			ln, 		2, 			"-s",	"�������� ��ũ����" },
	{ "date", 		date, 		0, 			"",		"" },
	{ "exit",		quit,		0,			"",		"" },
	{ "pwd", 		pwd,		0,			"",		"" },
	{ "rm",			rm,			1,			"",		"�����̸�" },
	{ "uname",		unixname, 	0,			"-a",	"" },	
	{ "whoami", 	whoami, 	0, 			"", 	"" },
	{ "mkdir",		makedir,	1,			"",		"���丮" },
	{ "rmdir",		removedir,	1,			"",		"���丮" },
	{ "mv",			mv,			2,			"",		"�������� �ٲ��̸�" },
	{ "touch", 		touch, 		1, 			"", 	"�����̸�" },
	{ "cat", 		cat, 		1,			"",		"�����̸�" },
	{ "sleep", 		Sleep,		1, 			"",		"�ʴ����ð�" },
};
// (�� ��ɾ� ����) = (cmd_tbl[] �迭 ��ü ũ��) / (�迭�� �� ���� ũ��)
int num_cmd = sizeof(cmd_tbl) / sizeof(cmd_tbl[0]);

// �� ���α׷��� �����ϴ� ��ɾ� ����Ʈ�� ���� �ִ� ��ɾ�
// ����: help
// �� �Լ��� �Ϲ� ��ɾ ó���ϴ� �Լ������ �޸� ���⿡ ��ġ�� ����
// �ٷ� ���� �ִ� cmd_tbl[]�� �� �Լ��� ���� ����ϱ� �����̴�.
void
help(void)
{
	int  k;

	printf("���� �����Ǵ� ��ɾ� �����Դϴ�.\n");
	for (k = 0; k < num_cmd; ++k)
		print_usage("  ", cmd_tbl[k].cmd, cmd_tbl[k].opt, cmd_tbl[k].arg);
	printf("\n");
}


//run_cmd()
void
run_cmd(void)
{
	pid_t pid;

	if( ( pid = fork()) < 0 )
		longjmp(jump, -1);
	
	else {
		if(pid == 0) {
			int i, cnt = 0;
			char *nargv[100];

			nargv[cnt++] = cmd;

			for(i = 0; i < optc; ++i)
				nargv[cnt++] = optv[i];
	
			for(i = 0; i < argc; ++i)
				nargv[cnt++] = argv[i];

			nargv[cnt++] = NULL;

			if(execvp(cmd, nargv) < 0) {
				perror(cmd);
				exit(1);
			}
		}
		
		else {
			if(waitpid(pid, NULL, 0) < 0)
				longjmp(jump, -1);
		}
	}

}


// proc_cmd():
// �Էµ� ��ɾ�� ������ �̸��� ��ɾ� ����ü�� cmd_tbl[] �迭 ã�� �� ��
// ��ɾ� ���ڿ� �ɼ��� ����� �Է��ߴ��� üũ�ϰ� �߸� �Ǿ����� ������
// ����ϰ� �����̸� �ش� ��ɾ ó���ϴ� �Լ��� ȣ���Ͽ� ������
void
proc_cmd(void)
{
	int  k;

	for (k = 0; k < num_cmd; ++k) {	// �Է��� ��ɾ� ������ cmd_tbl[]���� ã��
		// ��������char*cmd: ����ڰ� �Է��� ��ɾ� ���ڿ� �ּ�, ��)cmd->"ls"
		if (EQUAL(cmd, cmd_tbl[k].cmd)) { // �Է� ��ɾ cmd_tbl���� ã����
	
		/* 		//existing item

			if ((check_arg(cmd_tbl[k].argc) < 0) || 	// ���� ���� üũ
				(check_opt(cmd_tbl[k].opt)  < 0))		// �ɼ� üũ
				// ���� ���� �Ǵ� �ɼ��� �߸� �Ǿ���: ���� ���
				print_usage("����: ", cmd_tbl[k].cmd,
									    cmd_tbl[k].opt, cmd_tbl[k].arg);
				//		 ��) ����: ls  -l  [���丮�̸�] 
			else
				cmd_tbl[k].func();		// ��ɾ ó���ϴ� �Լ��� ȣ��

			return;

		*/

			cmd_idx = k;
			check_arg(cmd_tbl[k].argc);
			check_opt(cmd_tbl[k].opt);
			cmd_tbl[k].func();
			return;
		}
	}
	// ����ڰ� �Է��� ��ɾ cmd_tbl[]���� ã�� ���� ���
 	// printf("%s : �������� �ʴ� ��ɾ��Դϴ�.\n", cmd);
	run_cmd();
}



//***************************************************************************
// 	main() �Լ�
//***************************************************************************
int 
main(int argc, char *argv[])
{
	int cmd_count = 1;

	char cmd_line[SZ_STR_BUF]; 	// �Էµ� ��ɾ� ���� ��ü�� ���� ����

	int jmpret;

	setbuf(stdout, NULL);	// ǥ�� ��� ���� ����: printf() ��� ȭ�� ���
	setbuf(stderr, NULL);	// ǥ�� ���� ��� ���� ����

	help();						// ��ɾ� ��ü ����Ʈ ���

	//pwd name -> cur_work_dir[](save)
	//SZ_STR_BUF: cur_work_dir[SZ_STR_BUF] size
	getcwd(cur_work_dir, SZ_STR_BUF);

	if( (jmpret = setjmp(jump)) != 0 ) {
		if(jmpret == -1)
			perror(cmd);
		else if(jmpret == -2) {
				print_usage("����: ", cmd_tbl[cmd_idx].cmd,
									    cmd_tbl[cmd_idx].opt, cmd_tbl[cmd_idx].arg);
		}	
	}

	for (;;) {
		// ��� ������Ʈ ���: "$ "
		printf("<%s> %d:  ", cur_work_dir, cmd_count);
		gets(cmd_line);	// Ű���忡�� �� ���� �Է� �޾� cmd_line[]�� ����

		// �Է¹��� �� ��(cmd_line[])�� ��ɾ�, �ɼ�, ���ڸ� ���� ���ڿ��� �и�
		if (get_argv_optv(cmd_line) != NULL) {
			cmd_count++;
			proc_cmd();	// �� ��ɾ� ó�� 
		}
		// else ��ɾ�� ���� �ʰ� ���͸� ģ ��� ��ɾ� ��ȣ�� �������� ����
	}
}
