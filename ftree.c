#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "hash.h"
#include "ftree.h"
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>


int dirExists(char *path){
	int i = -1;
	DIR *dir = opendir(path);
	if (dir){
		i = 1;
	}
	else{
		i = 0;
	}
	return i;
}


int fileExists(char *path){
	if (access(path, F_OK) != -1){
		return 1;
	}
	return 0;
}

void createFileStruct(char *path, struct fileinfo *info){
	struct stat fileStat;
	if (lstat(path, &fileStat) < 0){
	perror("lstat");
	}
	// printf("fileStat size is: %9jd\n", fileStat.st_size);
	// printf("fileStat st_mode is: %o\n", fileStat.st_mode & 0777);
	info->size = fileStat.st_size;
	info->mode = fileStat.st_mode;
	strcpy(info->hash, "hash123"); 
	strcpy(info->path, path);

}


int setup (void) {
  int on = 1;
  struct sockaddr_in self;
  int listenfd;
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  // Make sure we can reuse the port immediately after the server terminates.
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                 &on, sizeof(on)) == -1) {
    perror("setsockopt -- REUSEADDR");
  }

  memset(&self, '\0', sizeof(self));
  self.sin_family = AF_INET;
  self.sin_addr.s_addr = INADDR_ANY;
  self.sin_port = htons(PORT);
  // printf("Listening on %d\n", PORT);

  if (bind(listenfd, (struct sockaddr *)&self, sizeof(self)) == -1) {
    perror("bind"); // probably means port is in use
    exit(1);
  }

  if (listen(listenfd, 5) == -1) {
    perror("listen");
    exit(1);
  }
  return listenfd;
}


int receiveFile(int fd){
	int bytes_read;
  	struct fileinfo *readStruct = (struct fileinfo*)malloc(sizeof(struct fileinfo));
  	char buf[MAXPATH];
  	size_t sizebuf;
  	mode_t modebuf;
  	char hash_buf[HASH_SIZE];  
  	int match_or_nah = 0;
  	FILE *fp;
  	int hash_or_nah = 0;
  	int bytes_wrote;  	
	    // Receive information from client and create a struct for it
      bytes_read = read(fd, buf, MAXPATH);
      if (bytes_read == -1){
      	match_or_nah = MATCH_ERROR;
      	fprintf(stderr, "%s", "MATCH ERROR\n");
			write(fd, &match_or_nah, sizeof(int));
			return 1;
      }
      bytes_read = read(fd, &sizebuf, sizeof(size_t));
      if (bytes_read == -1){
      	fprintf(stderr, "%s", "MATCH ERROR\n");
      	
      	match_or_nah = MATCH_ERROR;
			write(fd, &match_or_nah, sizeof(int));
			return 1;
      }
		bytes_read = read(fd, &modebuf, sizeof(mode_t));
		if (bytes_read == -1){
			fprintf(stderr, "%s", "MATCH ERROR\n");
      	match_or_nah = MATCH_ERROR;
			write(fd, &match_or_nah, sizeof(int));
			return 1;
      }
		bytes_read = read(fd, hash_buf, HASH_SIZE);
		if (bytes_read == -1){
			fprintf(stderr, "%s", "MATCH ERROR\n");
      	match_or_nah = MATCH_ERROR;
			write(fd, &match_or_nah, sizeof(int));
			return 1;
      }
		strncpy(readStruct->path, buf, MAXPATH - 1);
		readStruct->size = sizebuf;
		readStruct->mode = modebuf;
		strncpy(readStruct->hash, hash_buf, HASH_SIZE);
		// printf("path is: %s\n", readStruct->path);
 		// show_hash(readStruct->hash, HASH_SIZE);
		struct stat destStat;
		int match = MATCH;
		int mismatch = MISMATCH;
		if(strcmp(readStruct->hash, "hash123") == 0){
			// printf("hash123 if statement\n");
			if (dirExists(readStruct->path) == 0){
				mkdir(readStruct->path, readStruct->mode);			
			}
			chmod(readStruct->path, readStruct->mode);			
			match_or_nah = match;
			write(fd, &match_or_nah, sizeof(int));
		
		}
		else if(strcmp(readStruct->hash, "end") == 0){
			// printf("end if statement\n");
			return 2;		
		}
		else{
		if(dirExists(readStruct->path) == 1){
			fprintf(stderr, "%s", "MATCH ERROR\n");
			match_or_nah = MATCH_ERROR;
			write(fd, &match_or_nah, sizeof(int));
			return 1;
		
		} 						
		if (fileExists(readStruct->path) == 0){
			// printf("file does not exist if statement\n");
			match_or_nah = MISMATCH;
			write(fd, &mismatch, sizeof(int));		
			hash_or_nah = 0;			
		}
		else{
		lstat(readStruct->path, &destStat);
		
		if(destStat.st_size != readStruct->size){
		// printf("different sizes\n");
		write(fd, &mismatch, sizeof(int));		
		match_or_nah = MISMATCH;
		}
		else{
  		fp = fopen(readStruct->path, "r");
		char hash_value[9];
		strcpy(hash_value, hash(fp));
		// show_hash(hash_value, 8);
		// show_hash(readStruct->hash, 8);
		if (strcmp(hash_value, readStruct->hash) != 0){
			// printf("different hash values\n");
			write(fd, &mismatch, sizeof(int));
			match_or_nah = MISMATCH;
			hash_or_nah = 1;
		}
		else{
			write(fd, &match, sizeof(int));	
			
		}		
		}
		}
		}
  		if (match_or_nah == MISMATCH){
			int bytes_read = 1;
			int chunk_size = 10;
			char buffer[chunk_size];
			int iteration = 0;			
			
			while(bytes_read > 0){
			// open in write mode in first iteration
			if(iteration == 0 && hash_or_nah == 1){
			fp = freopen(readStruct->path, "w", fp);							
			}
			else if(iteration == 0 && hash_or_nah == 0){
			// printf("in create new file if statement\n");
			fp = fopen(readStruct->path, "w");			
			}
			// reopen in append mode in second iteration
			else if(iteration == 1){
			fp = freopen(readStruct->path, "a", fp);
			}
			bytes_read = read(fd, buffer, chunk_size);
			if (bytes_read == -1){
			fprintf(stderr, "%s", "TRANSMIT ERROR");
      	perror("Transmit");
      	match_or_nah = TRANSMIT_ERROR;
			write(fd, &match_or_nah, sizeof(int));
			return 1;
      	}
			// printf("buffer is: %s\n", buffer);
			// printf("read %d bytes\n", bytes_read);			
			bytes_wrote = write(fileno(fp), buffer, bytes_read);
			if (bytes_wrote == -1){
			fprintf(stderr, "%s", "TRANSMIT ERROR");
			perror("Transmit");
			match_or_nah = TRANSMIT_ERROR;
			write(fd, &match_or_nah, sizeof(int));
			return 1;
			}
			iteration += 1;
			if (bytes_read < chunk_size){
				break;			
			}
			}
			// printf("overwrote file in %d iterations\n", iteration);
		}
		
		free(readStruct);

		return 0;
}



void fcopy_server(int port){
  int listenfd;
  int fd;
  struct sockaddr_in peer;
  socklen_t socklen;
  listenfd = setup();
  // printf("setup done\n");
  socklen = sizeof(peer);
      if ((fd = accept(listenfd, (struct sockaddr *)&peer, &socklen)) < 0) {
      perror("accept");
    } while(fd >= 0) {
      // printf("New connection on port %d\n", ntohs(peer.sin_port));
		int i = 0;
		int transmit = 0; 		
		while(i == 0){
			i = receiveFile(fd);
			transmit = TRANSMIT_OK;
			write(fd, &transmit, sizeof(int));
			
      }
      fd = accept(listenfd, (struct sockaddr *)&peer, &socklen);
      if (fd < 0){
			perror("accept");      
      
      }
      }
      
    }

int copyFile(char *src_path, char *dest_path, int soc, int is_dir){
  struct fileinfo *info = (struct fileinfo*)malloc(sizeof(struct fileinfo));
  char fullPath[255];
  char fullDest[255];
  realpath(src_path, fullPath);
  realpath(dest_path, fullDest);
  createFileStruct(fullPath, info);
  write(soc, fullDest, MAXPATH);
  write(soc, &(info->size), sizeof(size_t));
  write(soc, &(info->mode), sizeof(mode_t));
  FILE *fp;
  if (is_dir == 0){
  
  fp = fopen(fullPath, "r");
  if (fp == NULL){
	perror("fullPath");
	return 1;  
  
  }	  
  strncpy(info->hash, hash(fp), HASH_SIZE);
  }
  write(soc, info->hash, HASH_SIZE);
  int matchInt;
  read(soc, &matchInt, sizeof(int));
  if (matchInt == 2){
	free(info);
  }
  else if(matchInt == MATCH_ERROR ){
  	fprintf(stderr, "%s", "MATCH ERROR\n");
	return 1;  
  }
  else if(matchInt == TRANSMIT_ERROR){
  fprintf(stderr, "%s", "TRANSMIT ERROR\n");
  return 1;
  
  }
  else{
  	int chunk_size = 10;
	char buffer[chunk_size + 1];
	int bytes_read = 1;
	rewind(fp);
	
	while (bytes_read > 0){
	bytes_read = read(fileno(fp), buffer, chunk_size);
	write(soc, buffer, bytes_read);
	if (bytes_read < chunk_size){
	break;	
	}
	}	
	free(info);
  }
  return 0;
}


int traverse(char *src_path, char *dest_path, int soc){
	int x;
	x = chdir(src_path);
	if(x == -1){
	perror(src_path);
	return 1;	
	}
	DIR *d;
	struct dirent *dir;
	char src[255];
	realpath(src_path, src);
	d = opendir(src);
	if (d == NULL){
		perror(src);
		return 1;	
	
	}
	char targetFull[255];
	char targetWithFile[255];
	realpath(dest_path, targetFull);
	struct stat dirStat;	
	while ((dir = readdir(d)) != NULL){
				
		x = lstat(dir->d_name, &dirStat);
		if(x == -1){
		perror("lstat");
		return 1;		
		}
		if (S_ISLNK(dirStat.st_mode)) continue;
		if(strcmp(".",dir->d_name) == 0 || strcmp("..",dir->d_name) == 0 || strcmp("a.out",dir->d_name) == 0)
                continue;
      
      int transmit = 0;    
		if (dirExists(dir->d_name) == 0){
			// printf("issa file: %s\n", dir->d_name);
			strncpy(targetWithFile, targetFull, 254);
			strcat(targetWithFile,"/");
			strcat(targetWithFile, dir->d_name);
			// printf("targetWithFile: %s\n", targetWithFile);
			x = copyFile(dir->d_name, targetWithFile, soc, 0);
			// error handling for copyFile
			if (x == 1){
				return 1;			
			}
			read(soc, &transmit, sizeof(int));
			if (transmit != TRANSMIT_OK){ 
			return 1;			
			}
				
			// printf("issa file: %s\n", dir->d_name);
		}
		else{
				// printf("issa dir: %s", dir->d_name);
				x = chdir(dir->d_name);
				if(x == -1){
				perror(src_path);
				return 1;	
				}
				char cwd[255];
				getcwd(cwd, 255);
				strncpy(targetWithFile, targetFull, 254);
				strcat(targetWithFile,"/");
				strcat(targetWithFile, dir->d_name);
				x = copyFile(cwd, targetWithFile, soc, 1);
				// error handling for copyFile
				if (x == 1){
				return 1;			
				}
				read(soc, &transmit, sizeof(int));
				// recursive call
				if (transmit == TRANSMIT_OK){	
				// printf("received transmit_ok \n");
				traverse(cwd, targetWithFile, soc);
				chdir("..");
				}
				else{
									
					return 1;
				}
			}
	
		
		}
	return 0;

}



int fcopy_client(char *src_path, char *dest_path, char *host, int port){
  int soc;
  struct sockaddr_in peer;
  if ((soc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("randclient: socket");
    exit(1);
  }

  peer.sin_family = AF_INET;
  peer.sin_port = htons(PORT);
  if (inet_pton(AF_INET, host, &peer.sin_addr) < 1) {
    perror("randclient: inet_pton");
    close(soc);
    exit(1);
  }

  if (connect(soc, (struct sockaddr *)&peer, sizeof(peer)) == -1) {
    perror("randclient: connect");
    exit(1);
  }
  // open source path and create a fileinfo struct for it
  // source = fopen(src_path, "r");
  
  int x;	
  x = traverse(src_path, dest_path, soc);  
  if (x == 1){
  		close(soc);
  		return 1;
  
  }
  // send a struct to indicate that the traversal is over  
  struct fileinfo end;
  strcpy(end.path, "end");
  strcpy(end.hash, "end");
  end.size = 0;
  end.mode = 0;
  write(soc, end.path, MAXPATH);
  write(soc, &(end.size), sizeof(size_t));
  write(soc, &(end.mode), sizeof(mode_t));
  write(soc, end.hash, HASH_SIZE);
  
  close(soc);
  return 0;
}