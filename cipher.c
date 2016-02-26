#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h> // for open flags
#include <time.h> // for time measurement
#include <assert.h>
#include <errno.h> 
#include <string.h>


int main(int argc, char** argv)
{
	assert(argc == 4);


	char filename_in[100], filename_out[100];
	struct dirent *dp;
	DIR *dfd, *dfdo;

	char *dir_in, *dir_out;
	dir_in = argv[1];
	dir_out = argv[3];
	int ok = 1;


	// open directory stream
	assert((dfd = opendir(dir_in)) != NULL);
	if ((dfdo = opendir(dir_out)) == NULL){
		mkdir(dir_out, 0777);
	}

	int fd;
	fd = open(argv[2], O_RDWR);
	if (fd < 0){
		printf("Error opening file: %s\n", strerror(errno));
		return errno;
	}


	char buf[4096], bufKey[4096], buf_output[4096];


	dir_in = argv[1];
	dir_out = argv[3];

	while ((dp = readdir(dfd)) != NULL)
	{

		ok = 1;
		// full path to file
		sprintf(filename_in, "%s/%s", dir_in, dp->d_name);
		sprintf(filename_out, "%s/%s", dir_out, dp->d_name);

		struct stat statbuf1;
		assert(stat(filename_in, &statbuf1) != -1);
		// skip directories
		if ((statbuf1.st_mode & S_IFMT) == S_IFDIR)
		{
			continue;
		}

		int fd_input = open(filename_in, O_RDWR);
		if (fd_input < 0){
			printf("Error opening file: %s\n", strerror(errno));
			return errno;
		}
		int fd_output = open(filename_out, O_CREAT | O_RDWR);
		if (fd_output < 0){
			printf("Error opening file: %s\n", strerror(errno));
			return errno;
		}

		while (ok){
			int bytes_in = read(fd_input, buf, 4096);
			assert(bytes_in != -1);
			if (bytes_in == 0){
				ok = 0;
				break;
			}
			int bytes_key = read(fd, bufKey, bytes_in);
			assert(bytes_key != -1);

			int i;
			int bytes_in_copy = bytes_in;

			if (bytes_in == bytes_key){
				for (i = 0; i < bytes_key; i++){
					buf_output[i] = buf[i] ^ bufKey[i];
				}
			}

			else if (bytes_key < bytes_in){
				int i, index_in = 0;
				while (bytes_in > 0){
					for (i = 0; i < bytes_key; i++){
						buf_output[index_in] = buf[index_in] ^ bufKey[i];
						index_in++;
					}
					bytes_in = bytes_in - bytes_key;
					if (bytes_in == 0){
						break;
					}
					assert(!close(fd));
					fd = open(argv[2], O_RDWR);
					if (fd < 0){
						printf("Error opening file: %s\n", strerror(errno));
						return errno;
					}
					bytes_key = read(fd, bufKey, bytes_in);
					assert(bytes_key != -1);





				}
			}
			assert(write(fd_output, buf_output, bytes_in_copy) == bytes_in_copy);
		}



		assert(!close(fd)); // close file
		assert(!close(fd_input)); // close file
		assert(!close(fd_output)); // close file
		fd = open(argv[2], O_RDWR);
		if (fd < 0){
			printf("Error opening file: %s\n", strerror(errno));
			return errno;
	}

	}

	assert(!close(fd)); // close file
	closedir(dfd);
	closedir(dfdo);

	return 0;


}

