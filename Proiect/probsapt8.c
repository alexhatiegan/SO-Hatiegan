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

int main(int argc, char *argv[]) {
    if (argc != 3) {
      printf("Usage: %s <nume_fisier.bmp>,%s\n", argv[0],argv[1]);
        return 1;
    }
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
    while((dir_point=readdir(p_dir))!=NULL){
      int fl_name = open(dir_point->d_name,O_RDONLY);
      if(fl_name == -1){
	perror("Eroare deschidere");
	exit(-1);
      }
      int pid,status;
      if((pid=fork())<0){
	exit(-1);
      }
      if(pid==0){
	char fisier[400];
	snprintf(fisier,sizeof(fisier),"%s/%s_statisitca.txt",argv[2],dir_point->d_name);
	int stat_fd = open(fisier, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (stat_fd == -1) {
	  perror("Nu am putut crea fisierul statistica.txt");
	  return 1;
       }
      printf("%s\n",dir_point->d_name);
      struct stat fis;
      if(fstat(fl_name,&fis)==-1)
	exit(-1);
      //Verificam daca fisierul este BMP
      if(S_ISREG(fis.st_mode)){
	char string[100];
	strcpy(string,dir_point->d_name);
	char *p=strtok(string,".");
	p=strtok(NULL,".");
	if(strcmp(p,"bmp")==0){
    struct BMPHeader header;
    int bmp_fd = open(dir_point->d_name, O_RDONLY);
    if (bmp_fd == -1) {
        perror("Nu am putut deschide fisierul");
        return 1;
    }
    read(bmp_fd, type, 2);

    int bytes_read = read(bmp_fd, &header, sizeof(struct BMPHeader));
    if (bytes_read != sizeof(struct BMPHeader)) {
        perror("Eroare la citirea antetului BMP");
        close(bmp_fd);
        return 1;
    }

    //struct stat file_info;
    if (fstat(bmp_fd, &fis) == -1) {
        perror("Nu am putut accesa informatiile fisierului");
        close(bmp_fd);
        return 1;
    }

    char statistics[1024];
    snprintf(statistics, sizeof(statistics), "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %ld\n%s\n",
    dir_point->d_name, header.height, header.width, fis.st_size, fis.st_uid,
    ctime(&fis.st_mtime), fis.st_nlink, permisii_to_string(fis.st_mode));

    if (write(stat_fd, statistics, strlen(statistics)) == -1) {
        perror("Eroare la scrierea informatiilor in statistica.txt");
    }

    close(bmp_fd);
	}
      }
      if(S_ISREG(fis.st_mode)){
	char string2[100];
	//struct stat file_info;
	strcpy(string2,dir_point->d_name);
	char *p2 =strtok(string2,".");
	p2=strtok(NULL,".");
	if(strcmp(p2,"bmp")!=0){
	  int file = open(dir_point->d_name,O_RDONLY);
	  if(file==-1){
	    exit(-1);
	  }
	  char statistics2[1024];
	  snprintf(statistics2,sizeof(statistics2),"Nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\n%s\n",dir_point->d_name,fis.st_size,fis.st_uid,ctime(&fis.st_mtime),fis.st_nlink,permisii_to_string(fis.st_mode));
	  if(write(stat_fd,statistics2,strlen(statistics2))==-1){
	    perror("EROARE scriere informatii in statistica.txt");
	  }
	}	
      }
      if(S_ISDIR(fis.st_mode)){
	int file = open(dir_point->d_name,O_RDONLY);
	if(file == -1){
	  exit(-1);
	}
	char statistics3[1024];
	snprintf(statistics3,sizeof(statistics3),"Nume director: %s\nIdentificatorul utilizatorului: %d\n%s\n",dir_point->d_name,fis.st_uid,permisii_to_string(fis.st_mode));
	if(write(stat_fd,statistics3,strlen(statistics3))==-1){
	  perror("EROARE scriere informatii in statistica.txt");
	}
      }
      if(S_ISLNK(fis.st_mode) && lstat(dir_point->d_name,&fis)){
	char statistics4[1024];
	char *link_name=malloc(fis.st_size+1);
	readlink(dir_point->d_name,link_name,fis.st_size); //am salvat numele legaturii intr-o variabila si am citit continutul legaturii simbolice
	struct stat link_st;
	if(stat(link_name,&link_st)==0 && S_ISREG(link_st.st_mode)){
	  snprintf(statistics4,sizeof(statistics4),"Nume legatura: %s\ndimensiune: %ld\ndimensiune fisier: %ld\n,%s",dir_point->d_name,fis.st_size,link_st.st_size,permisii_to_string(fis.st_mode));
	  if(write(stat_fd,statistics4,strlen(statistics4))==-1){
	    perror("EROARE scriere informatii in statistica.txt");
	  }
	}
      }
      close(stat_fd);
      exit(0);
    }
    }
    return 0;

}
