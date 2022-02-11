#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "file.h"

/**
 * 获取文件大小
 * @param file [in] 文件路径
 * @return 整数 0=失败 其他表示成功
 */
static int get_file_size(const char *file)
{
	if (file != NULL && 0 == access(file, F_OK))
	{
		struct stat ft;
		memset(&ft, 0, sizeof(struct stat));

		if (0 == stat(file, &ft))
		{
			return ft.st_size;
		}
	}
	return 0;
}

/**
 * 获取文件内容
 * @param file [in] 文件路径
 * @return 指针 NULL=失败 其他表示成功
 */
char *read_info_from_file(const char *file)
{
	int fileSize = get_file_size(file);
	if (0 == fileSize)
	{
		return NULL;
	}

	char *buff = (char *)malloc(sizeof(char) * (fileSize + 1));
	if (NULL == buff)
	{
		return NULL;
	}

	FILE *fp = fopen(file, "r");
	if (fp != NULL)
	{
		int len = fread(buff, sizeof(char), fileSize, fp);
		fclose(fp);

		return buff;
	}

	free(buff);
	return NULL;
}
