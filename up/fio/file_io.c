// "fio_bin_fwrite.c"�� ������ include�� ��� �ѹ��� include�ϱ� ����
#pragma warning(disable:4996)

#ifndef _FILE_IO_C
#define _FILE_IO_C

//*****************************************************************************
//   FILE ���� �ݱ� �Լ���:  C ǥ�� fopen() fclose() fsetpos()
//*****************************************************************************

static FILE *fp;		//�̹� ����Ǿ� ����

int
r_open_fp(char *name) // ���� �ҷ�����: �б������ ����
{
	return((fp = fopen(name, "r")) ? 0 : -1);
	// �� ������ return( (fp = fopen(name, _____)) != NULL ? 0 : -1 );�� ������
	// NULL�� (void *)0, �� ������ ��(�ּ� ��)�� 0��
	// FILE *p = NULL; �̰��� p = (void *)0�� ����, p���� ��� 0�� �����
	// if (p)�ϸ�, �� ���ǹ��� p�� 0�̹Ƿ� false��
	// ���� p�� NULL�� �ƴϸ� true��
}


int 
w_open_fp(char *name) // �����ϱ�: ������ ��������� ����
{ // ������ ������ ���� ũ�� 0�� ����, ������ ���� ����
	return ((fp = fopen(name, "w")) ? 0 : -1);
}

int n_open_fp(char *name) // �ٸ�(��) �̸����� �����ϱ�: ��������� ����
{ // ���� �б������ ���� ����
	if ((fp = fopen(name, "r"))) {// ������ ������ ������ ��� (fp != NULL)
		fclose(fp); // ���� fp ������ �ݰ�
		return(-2); // ������ �̸��� ������ �̹� �����ϹǷ� ����
	}// ������ �������� ���� ���(fp == NULL): ��������� ���� ����
	return((fp = fopen(name, "w")) ? 0 : -1);
}

int
rw_open_fp(char *name) // ������ �б�/��������� ���� (���� ������ ����)
{ // ���� ���ڵ� ���� ���� �޴� ���� �� ȣ���
	return((fp = fopen(name, "r+")) ? 0 : -1);
} // �� �Լ��� ȣ���Ͽ� ������ �������� ���� ��� �Ʒ� �Լ��� ȣ���

int
rwc_open_fp(char *name)		//������ �б�/��������� ���� (���� ������ ����)
{
	return ((fp = fopen(name, "w+")) ? 0 : -1);
}

//	DB ������ �ݴ´�.
int
close_fp(void)
{
	return(fclose(fp));
}

int 
setpos_fp(int pos)		// fp ������ �а� ���� ��ġ(file position)�� pos�� �ű�
{
	return(fseek(fp, pos, SEEK_SET));
}

//===========================================================================
// TEXT FILE I/O ���� �Լ��� : C ǥ�� fscanf() fprintf()
//===========================================================================

static void
trunc_space(char *p)
{
	while (*p) ++p;
	for (--p; isspace((int)*p); --p) ;
	*(++p) = 0;
}

//	DB ���Ͽ��� ���ڵ� �ϳ��� �о� �޸� r�� �����Ѵ�.
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

//	DB ���Ͽ� ���ڵ� r�� �����Ѵ�.
int
fprintf_rec(rec_t *r)
{
	return(fprintf(fp, "%-10d %-11s %-15s %1d %-38s\n",
		r->key, r->name, r->dep, r->grade, r->addr));
}

//	DB ���Ͽ��� ������ ��������� �о� �ش� ������ �����Ѵ�.
int
fscanf_hd(head_t *h)
{
	return(fscanf(fp, "%s %d %d %s %f %d %d %d\n",
		h->name, &h->head_sz, &h->rec_sz, h->program,
		&h->version, &h->start_key, &h->rec_num, &h->fio_type));
}

//	DB ���Ͽ� ������ ������� ���� ���� �����Ѵ�.
int
fprintf_hd(head_t *h)
{
	return(fprintf(fp, "%-19s %-2d %-2d %-19s %-3.1f %-6d %-6d %1d\n",
		h->name, h->head_sz, h->rec_sz, h->program,
		h->version, h->start_key, h->rec_num, h->fio_type));
}



//===========================================================================
// TEXT FILE I/O ���� �Լ��� : C ǥ�� sscanf() sprintf() fgets() fputs()
//===========================================================================

static char sbuf[BUF_LEN];

//	DB ���Ͽ��� ���ڵ� �ϳ��� �о� �޸� r�� �����Ѵ�.
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

//	DB ���Ͽ� ���ڵ� r�� �����Ѵ�.
int
sprintf_rec(rec_t *r)
{
	sprintf(sbuf, "%-10d %-11s %-15s %1d %-38s\n",
		r->key, r->name, r->dep, r->grade, r->addr);
	return(fputs(sbuf, fp));
}

//	DB ���Ͽ��� ������ ��������� �о� �ش� ������ �����Ѵ�.
int
sscanf_hd(head_t *h)
{
	if (fgets(sbuf, BUF_LEN, fp) == NULL)
		return (0);

	return(sscanf(sbuf, "%s %d %d %s %f %d %d %d\n",
		h->name, &h->head_sz, &h->rec_sz, h->program,
		&h->version, &h->start_key, &h->rec_num, &h->fio_type));
}

//	DB ���Ͽ� ������ ������� ���� ���� �����Ѵ�.
int
sprintf_hd(head_t *h)
{
	sprintf(sbuf, "%-19s %-2d %-2d %-19s %-3.1f %-6d %-6d %1d\n",
		h->name, &h->head_sz, &h->rec_sz, h->program,
		&h->version, &h->start_key, &h->rec_num, &h->fio_type);
	return(fputs(sbuf, fp));
}

//===========================================================================
// Binary FILE I/O ���� �Լ��� : C ǥ�� fread() fwrite()
//===========================================================================

//	DB ���Ͽ��� ���ڵ� �ϳ��� �о� �޸� r�� �����Ѵ�.
int
fread_rec(rec_t *r)
{
	return(fread(r, sizeof(rec_t), 1, fp));
}

//	DB ���Ͽ� ���ڵ� r�� �����Ѵ�.
int
fwrite_rec(rec_t *r)
{
	return(fwrite(r, sizeof(rec_t), 1, fp));
}

//	DB ���Ͽ��� ������ ��������� �о� �ش� ������ �����Ѵ�.
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

//	DB ���Ͽ� ������ ������� ���� ���� �����Ѵ�.
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
// Binary FILE I/O ���� �Լ��� : UNIX API open() closde() read() write()
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

//	DB ������ �ݴ´�.
int
close_fd(void)
{
	return(close(fd));
}

//	DB ���Ͽ��� ���ڵ� �ϳ��� �о� �޸� r�� �����Ѵ�.
int
read_rec(rec_t *r)
{
	//read(���ϱ����, ���Ͽ��� �о� �� ������ ������ �޸� �ּ�, �޸� ũ��);
	return (read(fd, r, sizeof(rec_t)) );
}

//	DB ���Ͽ� ���ڵ� r�� �����Ѵ�.
int
write_rec(rec_t *r)
{
	//write(���ϱ����, ���Ͽ� �� �޸� �ּ�, �޸� ũ��(���� �� ������ ����));
	return (write(fd, r, sizeof(rec_t)) );
}

//	DB ���Ͽ��� ������ ��������� �о� �ش� ������ �����Ѵ�.
// ���Ͽ��� ��带 �о�� �޸� h�� �����Ѵ�.
int 
read_hd(head_t *h)
{ // write()�� ������� ������� �о� ���� ��
  //read(fd, ���Ͽ��� �о� �� ������ ������ �޸� �ּ�, �޸� ũ��);
	read(fd, h->name, NAME_LEN);
	read(fd, &h->head_sz, sizeof(int));
	read(fd, &h->rec_sz, sizeof(int));
	read(fd, h->program, NAME_LEN);
	read(fd, &h->version, sizeof(float));
	read(fd, &h->start_key, sizeof(int));
	read(fd, &h->rec_num, sizeof(int));
	read(fd, &h->fio_type, sizeof(int));
	
	return(sizeof(head_t)); // ������ �߻����� �ʾҴٰ� �����ϰ� ��� ũ�� ����
}

//	DB ���Ͽ� ������ ������� ���� ���� �����Ѵ�.
int 
write_hd(head_t *h) // ���Ͽ� ��� h�� �����Ѵ�.
{
	// char �迭�� �̸��� �ּ���, int�� float ����� �ּҸ� ���� ������ �־�� ��
	//write(fd, ���Ͽ� �� �޸� �ּ�, �޸� ũ��(���Ͽ� �� ������ ����));
	write(fd, h->name, NAME_LEN);
	write(fd, &h->head_sz, sizeof(int));
	write(fd, &h->rec_sz, sizeof(int));
	write(fd, h->program, NAME_LEN);
	write(fd, &h->version, sizeof(float));
	write(fd, &h->start_key, sizeof(int));
	write(fd, &h->rec_num, sizeof(int));
	write(fd, &h->fio_type, sizeof(int));
		
	return(sizeof(head_t)); // ������ �߻����� �ʾҴٰ� �����ϰ� ��� ũ�� ����
}

int 
setpos_fd(int pos)
{
	return(lseek(fd, pos, SEEK_SET));
}

#endif  /* _FILE_IO_C */

