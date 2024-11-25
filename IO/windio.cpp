#include"windio.h"


FILE* fopen(const char* path, const char* mode)
{
	FILE* ptr = (FILE*)malloc(sizeof(FILE));
	if (ptr == NULL)
		return NULL;

	int flags = 0;
	if (strcmp(mode, "a") == 0)
	{
		flags = O_CREAT | O_WRONLY |O_APPEND;
		int fd = open(path, flags, PERM);
		if (fd < 0)
		{
			free(ptr);
			return NULL;
		}
		ptr->_fileno = fd;

		char* p = (char*)malloc(sizeof(char) * MASZ);
		if (p == NULL)
		{
			close(fd);
			free(ptr);
			return NULL;
		}
		ptr->_buf_base = p;
		ptr->_buf_end = ptr->_buf_base + MASZ;

		ptr->_write_ptr = ptr->_buf_base;
		ptr->_write_end = ptr->_write_ptr;

		ptr->_read_end = NULL;
		ptr->_read_pos = NULL;
		ptr->_read_ptr = NULL;

		if (isatty(ptr->_fileno))
			ptr->_flags = _IO_MAGIC | _IO_NO_READS | _IOLBF | _IOFBF;
		else
			ptr->_flags = _IO_MAGIC | _IO_NO_READS | _IOFBF;
	}
	else if(strcmp(mode, "w") == 0)
	{
		flags = O_CREAT |O_WRONLY | O_TRUNC;
		int fd = open(path, flags, PERM);
		if (fd < 0)
		{
			free(ptr);
			return NULL;
		}
		ptr->_fileno = fd;

		char* p = (char*)malloc(sizeof(char) * MASZ);
		if (p == NULL)
		{
			close(fd);
			free(ptr);
			return NULL;
		}
		ptr->_buf_base = p;
		ptr->_buf_end = ptr->_buf_base + MASZ;

		ptr->_write_ptr = ptr->_buf_base;
		ptr->_write_end = ptr->_write_ptr;

		ptr->_read_end = NULL;
		ptr->_read_pos = NULL;
		ptr->_read_ptr = NULL;

		if (isatty(ptr->_fileno))
			ptr->_flags = _IO_MAGIC | _IO_NO_READS | _IOLBF | _IOFBF;
		else
			ptr->_flags = _IO_MAGIC | _IO_NO_READS | _IOFBF;
	}
	else if (strcmp(mode, "r"))
	{
		flags = O_RDONLY;
		int fd = open(path, flags);
		if (fd < 0)
		{
			free(ptr);
			return NULL;
		}
		ptr->_fileno = fd;

		char* p = (char*)malloc(sizeof(char) * MASZ);
		if (p == NULL)
		{
			close(fd);
			free(ptr);
			return NULL;
		}
		ptr->_buf_base = p;
		ptr->_buf_end = ptr->_buf_base + MASZ;

		ptr->_read_ptr = ptr->_buf_base;
		ptr->_read_pos = NULL;
		ptr->_read_end = ptr->_read_ptr;

		ptr->_write_end = NULL;
		ptr->_write_ptr = NULL;

		if (isatty(ptr->_fileno))
			ptr->_flags = _IO_MAGIC | _IO_NO_WRITES | _IOLBF | _IOFBF;
		else
			ptr->_flags = _IO_MAGIC | _IO_NO_WRITES | _IOFBF;
	}
	else
	{
		free(ptr);
		return NULL;
	}
	return ptr;
}

int fflush(FILE* stream)
{
	if (stream->_flags & _IO_MAGIC)
	{
		if (stream->_flags & _IO_NO_WRITES)
			return -1;
		else
		{
			write(stream->_fileno, stream->_write_ptr, stream->_write_end - stream->_write_ptr);
			stream->_write_end = stream->_write_ptr;
			return 0;
		}
	}
	else
	{
		return -1;
	}
}

size_t fwrite(const char* ptr, size_t size, size_t nmemb, FILE* stream)
{
	if (stream->_flags & _IO_MAGIC)
	{
		if (stream->_flags & _IO_NO_WRITES)
			return 0;
		else
		{
			if (stream->_write_ptr + size * nmemb < stream->_buf_end)
			{
				memcpy(stream->_write_end, ptr, size * nmemb);
				stream->_write_end += size * nmemb;
				size_t ret = stream->_write_end - stream->_write_ptr;
				if (stream->_flags & _IOFBF)
				{
					if (stream->_write_end + 1 == stream->_buf_end)
						fflush(stream);
				}
				if (stream->_flags & _IOLBF)
				{
					if (*(stream->_write_end - 1) == '\n')
						fflush(stream);
				}
				return ret;
			}
			else
			{
				unsigned long long n = size * nmemb;
				unsigned long long cur = 0;
				while (cur + MASZ <= n)
				{
					memcpy(stream->_write_end, ptr, MASZ);
					stream->_write_end += MASZ;
					fflush(stream);
					cur += MASZ;
				}
				memcpy(stream->_write_end, ptr, n - cur);
				stream->_write_end += n - cur;
				if (stream->_flags & _IOLBF)
				{
					if (*(stream->_write_end - 1) == '\n')
						fflush(stream);
				}
				return (size_t)n;
			}
		}
	}
	else
		return 0;
	
}

//size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream)
//{
//	if (stream->_flags & _IO_MAGIC)
//	{
//		if (stream->_flags & _IO_NO_READS)
//			return 0;
//		else
//		{
//			ssize_t n = read(stream->_fileno, stream->_read_end, stream->_buf_end - stream->_read_end);
//			stream->_read_pos = stream->_read_ptr;
//			stream->_read_end += n;
//			if (n <= size * nmemb)
//			{
//				memcpy(ptr, stream->_read_pos, n);
//				stream->_read_end = stream->_read_ptr;
//				return n;
//			}
//			else
//			{
//				memcpy(ptr, stream->_read_ptr, size * nmemb);
//				stream->_read_pos += size * nmemb;
//				return size * nmemb;
//			}
//		}
//	}
//	else
//		return 0;
//}

int fclose(FILE* stream)
{
	if (stream->_flags & _IO_MAGIC)
	{
		if (stream->_flags & _IO_NO_WRITES);
		else
			fflush(stream);

		int ret = close(stream->_fileno);
		free(stream->_buf_base);
		free(stream);
		return ret;
	}
	else
		return -1;
}