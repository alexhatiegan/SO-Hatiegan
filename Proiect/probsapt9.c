#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <stdint.h>

// Structura pentru a stoca informațiile din antetul BMP
char type[3];
struct BMPHeader {
    int size;
    int reserved;
    int offset;
    int header_size;
    int width;
    int height;
    short planes;
    short bpp;
    int compression;
    int image_size;
    int x_ppm;
    int y_ppm;
    int colors;
    int important_colors;
};

void conversion_to_gray(const char *director1, const char *director2, struct stat fis){
  char string[50];
  strcpy(string,director1);
  char *p=strtok(string,".");
  p=strtok(NULL,".");
  if(strcmp(p,"bmp")==0){
    struct BMPHeader header;
    int bmp_fd = open(director1, O_RDONLY);
    if (bmp_fd == -1) {
      perror("Nu am putut deschide fisierul");
      exit(-1);
      //return 1;
    }
    read(bmp_fd, type, 2);
    
    int bytes_read = read(bmp_fd, &header, sizeof(struct BMPHeader));
    if (bytes_read != sizeof(struct BMPHeader)) {
      perror("Eroare la citirea antetului BMP");
      close(bmp_fd);
      exit(-1);
      //return 1;
    }
    
    //struct stat file_info;
    if (fstat(bmp_fd, &fis) == -1) {
      perror("Nu am putut accesa informatiile fisierului");
      close(bmp_fd);
      exit(-1);
      //return 1;
    }

    uint8_t colors[4];
    uint8_t gray;
    for( int i=0;i<header.width;i++ )
      for( int j=0;j<header.height;j++ ){
	read(bmp_fd,colors,4);
	gray = 0.299 * colors[0] + 0.587 * colors[1] + 0.114 * colors[2];
	colors[0] = gray;
	colors[1] = gray;
	colors[2] = gray;
	lseek(bmp_fd,SEEK_CUR,-4);
	write(bmp_fd,colors,4);
      }
  }
}

const char *permisii_to_string(mode_t permisiuni) {
    static char perm_string[100]; 
    snprintf(perm_string, sizeof(perm_string), "Drepturi de acces user:%c%c%c\nDrepturi de acces grup:%c%c%c\nDrepturi de acces altii:%c%c%c",
        (permisiuni & S_IRUSR) ? 'R' : '-',
        (permisiuni & S_IWUSR) ? 'W' : '-',
        (permisiuni & S_IXUSR) ? 'X' : '-',
        (permisiuni & S_IRGRP) ? 'R' : '-',
        (permisiuni & S_IWGRP) ? 'W' : '-',
        (permisiuni & S_IXGRP) ? 'X' : '-',
        (permisiuni & S_IROTH) ? 'R' : '-',
        (permisiuni & S_IWOTH) ? 'W' : '-',
        (permisiuni & S_IXOTH) ? 'X' : '-'
    );
    return perm_string;
}

int write_statistics_director(const char *director1, const char *director2, struct stat fis, int stat_fd){
  if(S_ISDIR(fis.st_mode)){
    char statistics[1024];
    snprintf(statistics,sizeof(statistics),"Nume director: %s\nIdentificatorul utilizatorului: %d\n%s\n",director1,fis.st_uid,permisii_to_string(fis.st_mode));
    if(write(stat_fd,statistics,strlen(statistics))==-1){
      perror("EROARE scriere informatii in statistica.txt");
    }
  }
  int count_lines = 8;
  return count_lines;
}//functie pentru scriere daca este director

int write_statistics_link(const char *director1, const char *director2, struct stat fis, int stat_fd){
  if(S_ISLNK(fis.st_mode)){
    char statistics[1024];
    char *link_name=malloc(fis.st_size+1);
    readlink(director1,link_name,fis.st_size); //am salvat numele legaturii intr-o variabila si am citit continutul legaturii simbolice
    struct stat link_st;
    if(stat(link_name,&link_st)==0 && S_ISREG(link_st.st_mode)){
      snprintf(statistics,sizeof(statistics),"Nume legatura: %s\ndimensiune: %ld\ndimensiune fisier: %ld\n,%s",director1,fis.st_size,link_st.st_size,permisii_to_string(fis.st_mode));
      if(write(stat_fd,statistics,strlen(statistics))==-1){
	perror("EROARE scriere informatii in statistica.txt");
      }
    }
  }
  int count_lines = 7;
  return count_lines;
}//functie pentru scriere daca este legatura simbolica

int write_statistics_regular_bmp(const char *director1, const char *director2, struct stat fis, int stat_fd){
  if(S_ISREG(fis.st_mode)){
    char string[100];
    strcpy(string,director1);
    char *p=strtok(string,".");
    p=strtok(NULL,".");
    if(strcmp(p,"bmp")==0){
      struct BMPHeader header;
      int bmp_fd = open(director1, O_RDONLY);
      if (bmp_fd == -1) {
        perror("Nu am putut deschide fisierul");
	exit(-1);
        //return 1;
      }
      read(bmp_fd, type, 2);
      
      int bytes_read = read(bmp_fd, &header, sizeof(struct BMPHeader));
      if (bytes_read != sizeof(struct BMPHeader)) {
        perror("Eroare la citirea antetului BMP");
        close(bmp_fd);
	exit(-1);
        //return 1;
      }
      
      //struct stat file_info;
      if (fstat(bmp_fd, &fis) == -1) {
        perror("Nu am putut accesa informatiile fisierului");
        close(bmp_fd);
	exit(-1);
        //return 1;
      }
      
      char statistics[1024];
      snprintf(statistics, sizeof(statistics), "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %ld\n%s\n",
	       director1, header.height, header.width, fis.st_size, fis.st_uid,
	       ctime(&fis.st_mtime), fis.st_nlink, permisii_to_string(fis.st_mode));
      
      if (write(stat_fd, statistics, strlen(statistics)) == -1) {
        perror("Eroare la scrierea informatiilor in statistica.txt");
      }
      
      close(bmp_fd);
    }
  }
  int count_lines = 11;
  return count_lines;
}

int write_statistics_regular(const char *director1, const char *director2, struct stat fis, int stat_fd){
  if(S_ISREG(fis.st_mode)){
    char string2[100];
    //struct stat file_info;
    strcpy(string2,director1);
    char *p2 =strtok(string2,".");
    p2=strtok(NULL,".");
    if(strcmp(p2,"bmp")!=0){
      int file = open(director1,O_RDONLY);
      if(file==-1){
	exit(-1);
      }
      char statistics[1024];
      snprintf(statistics,sizeof(statistics),"Nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\n%s\n",director1,fis.st_size,fis.st_uid,ctime(&fis.st_mtime),fis.st_nlink,permisii_to_string(fis.st_mode));
      if(write(stat_fd,statistics,strlen(statistics))==-1){
	perror("EROARE scriere informatii in statistica.txt");
      }
    }	
  }
  int count_lines = 10;
  return count_lines;
}//functie pentru scriere daca este fisier obisnuit

int main(int argc, char *argv[]) {
    if (argc != 4) {
      printf("Usage: %s <nume_fisier.bmp>,%s,%s\n", argv[0],argv[1],argv[2]);
        return 1;
    }
    char *ch = argv[3];
    struct dirent *dir_point;
    DIR *p_dir,*pp_dir;
    p_dir = opendir(argv[1]);
    pp_dir = opendir(argv[2]);
    if(p_dir==NULL){
      perror("Eroare deschidere director");
      exit(-1);
    }
    if(pp_dir==NULL){
      perror("Eroare deschidere director");
      exit(-1);
    }
    
    chdir(argv[1]);
    
    while((dir_point=readdir(p_dir))!=NULL){
      if(strcmp(dir_point->d_name,".") == 0 || strcmp(dir_point->d_name,"..") == 0){
	continue;
      }
      int pfd[2];
      if(pipe(pfd)<0){
	printf("Eroare pipe\n");
	exit(-1);
      }
      int count_lines = 0;
      int fl_name = open(dir_point->d_name,O_RDONLY);
      if(fl_name == -1){
	perror("Eroare deschidere");
	exit(-1);
      }
      int pid,status;
      char fisier[400];
      snprintf(fisier,sizeof(fisier),"../%s/%s_statistica.txt",argv[2],dir_point->d_name);
      int stat_fd = open(fisier, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
      if (stat_fd == -1) {
	perror("Nu am putut crea fisierul statistica.txt");
	exit(-1);
      }
      printf("%s\n",dir_point->d_name);
      struct stat fis;
      if(fstat(fl_name,&fis) == -1)
	exit(-1);
      lstat(dir_point->d_name,&fis);
      //Parcurgem si verificam de care tip sunt fisierele din director
      char aux[50];
      strcpy(aux,dir_point->d_name);
      char *p = strtok(aux,".");
      p = strtok(NULL,".");

      int pdf[2],pfd2[2];
      if(pipe(pfd) == -1){
	perror("Eroare pipe");
	exit(-1);
      }
      if(pipe(pfd2) == -1){
	perror("Eroare pipe");
	exit(-1);
      }
      
      if(S_ISREG(fis.st_mode)){
	if(strcmp(p,"bmp") == 0){
	  if((pid = fork()) < 0){
	    printf("Eroare la fork");
	    exit(-1);
	  }
	  if(pid == 0){
	    count_lines = 0;
	    count_lines = write_statistics_regular_bmp(dir_point->d_name,argv[2],fis,stat_fd);
	    printf("Numarul de linii scrise este %d\n", count_lines);
	    exit(0);
	  }
	  waitpid(pid,&status,0);
	  printf("S-a incheiat procesul cu pid-ul %d si codul %d\n",pid,status);
	  if((pid = fork()) < 0){
	    printf("Eroare la fork");
	    exit(-1);
	  }
	  if(pid == 0){
	    conversion_to_gray(dir_point->d_name,argv[2],fis);
	    exit(0);
	  }
	  waitpid(pid,&status,0);
	  printf("S-a incheiat procesul cu pid-ul %d si codul %d\n",pid,status);
	}
	if(strcmp(p,"bmp") != 0){
	  if((pid = fork()) < 0){
	    printf("Eroare la fork");
	    exit(-1);
	  }
	  char continut[1024];
	  if(pid == 0){
	    count_lines = 0;
	    count_lines = write_statistics_regular(dir_point->d_name,argv[2],fis,stat_fd);
	    printf("Numarul de linii scrise este %d\n", count_lines);
	    int pipe_fis = open(dir_point->d_name,O_RDONLY);
	    if(pipe_fis == -1){
	      printf("Eroare deschidere fisier\n");
	      exit(-1);
	    }
	    close(pfd[0]);
	    dup2(pfd[1],STDOUT_FILENO);//redirectioneaza stdout catre capatul de scriere
	    close(pfd[1]);
	    execlp("cat","cat",dir_point->d_name,NULL);
	    //close(pfd[1]);
	    exit(0);
	  }
	  close(pfd[1]);
	  char buffer[1024];
	  ssize_t bytesRead;
	  
	  // Citirea datelor din pipe pfd[0]
	  while ((bytesRead = read(pfd[0], buffer, sizeof(buffer))) > 0) {
            // Procesează sau afișează datele
            write(STDOUT_FILENO, buffer, bytesRead);
	  }
	  if((pid = fork()) < 0){
	    printf("Eroare la fork");
	    exit(-1);
	  }
	  if(pid == 0){
	    close(pfd[1]);
	    close(pfd2[0]);
	    dup2(pfd[0],0);
	    close(pfd[0]);
	    dup2(pfd2[1],1);
	    close(pfd2[1]);
	    execlp("bash","bash","script.sh",ch,NULL);
	    perror("execlp\n");
	  }
	  close(pfd[0]);
	  close(pfd[1]);
	  close(pfd2[1]);
	  int suma = 0;
	  while(read(pfd2[0],buffer,sizeof(continut)) > 0){
	    suma += suma + atoi(continut);
	  }
	  printf("Au fost identificate in total %d propozitii corecte care contin caracterul %s\n",suma,ch);
	  close(pfd2[0]);
	  waitpid(pid,&status,0);
	  printf("S-a incheiat procesul cu pid-ul %d si codul %d\n",pid,status);
	}
      }
      if(S_ISDIR(fis.st_mode)){
	if((pid = fork()) < 0){
	  printf("Eroare la fork");
	  exit(-1);
	}
	if(pid == 0){
	  count_lines = 0;
	  count_lines = write_statistics_director(dir_point->d_name,argv[2],fis,stat_fd);
	  printf("Numarul de linii scrise este %d\n", count_lines);
	  exit(0);
	}
	waitpid(pid,&status,0);
	printf("S-a incheiat procesul cu pid-ul %d si codul %d\n",pid,status);
      }
      if(S_ISLNK(fis.st_mode)){
	if((pid = fork()) < 0){
	  printf("Eroare la fork");
	  exit(-1);
	}
	if(pid == 0){
	  count_lines = 0;
	  count_lines = write_statistics_link(dir_point->d_name,argv[2],fis,stat_fd);
	  printf("Numarul de linii scrise este %d\n", count_lines);
	  exit(0);
	}
	waitpid(pid,&status,0);
	printf("S-a incheiat procesul cu pid-ul %d si codul %d\n",pid,status);
      }
      close(stat_fd);
    }
    return 0;

}
