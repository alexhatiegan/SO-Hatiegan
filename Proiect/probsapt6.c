#include <stdio.h> //pt intrare/iesire standard
#include <stdlib.h> //pt lucrul cu memorie
#include <string.h> //pt siruri de caractere
#include <unistd.h> //pt manipulare descriptori de fisiere (read, write)
#include <fcntl.h> //pt manipulare descriptori de fisiere, inchidere/deschidere 
#include <sys/stat.h> //pt informatii legate de fisiere, (functia stat)
#include <sys/types.h> //pt drepturi de acces la fisiere
#include <sys/time.h> //pt lucrul cu timpul 
#include <time.h>

// Structura pentru a stoca informațiile din antetul BMPHeader
char type[3]; //pentru a indica tipul de fisier (.bmp)
struct BMPHeader {
    int size; //dimensiune totala fisier bmp
    int reserved; //rezervat pt utilizare ulterioara
    int offset; //pt inceputul datelor de imagine
    int header_size; //dimensiune antet
    int width; //latimea imaginii in pixeli
    int height; //inaltimea imaginii in pixeli
    short planes; //nr de planes
    short bpp; //reprezinta cati biti sunt utilizati pentru a reprezenta culoarea unui singur pixel
    int compression; //reprezinta tipul de compresie
    int image_size; //reprezinta dimensiunea datelor din imagine
    int x_ppm;//reprezinta rezolutia orizontala in pixeli pe metru
    int y_ppm; //reprezinta rezolutia verticala in pixeli pe metru
    int colors; //nr de culori 
    int important_colors; //nr de culori importante
};//informatii din imaginea .bmp

const char *permisii_to_string(mode_t permisiuni) {
    static char perm_string[100]; //pt a stoca rezultatul
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
    ); //folosesc snprintf pentru a formata sirul de caractere 
    return perm_string; //returnare rezultat
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <nume_fisier.bmp>\n", argv[0]); //explicare folosire corecta
        return 1;
    } //Am verificat daca avem nr de argumente corect

    char *filename = argv[1]; //initializare
    struct BMPHeader header; //folosit pentru a stoca informatii din antet
    int bmp_fd = open(filename, O_RDONLY); //deschidere fisier bmp  in modul read

    if (bmp_fd == -1) {
        perror("Nu am putut deschide fisierul");
        return 1;
    }//Verific daca am deschis corect fisierul
    read(bmp_fd, type, 2); //citire pentru a vedea tipul fisierului

    int bytes_read = read(bmp_fd, &header, sizeof(struct BMPHeader)); //initializare, stocam informatiile citite in header
    if (bytes_read != sizeof(struct BMPHeader)) {
        perror("Eroare la citirea antetului BMP");
        close(bmp_fd);
        return 1;
    }//Verific daca am citit corect antetul BMP

    struct stat file_info; //stocare informatii despre fisierul BMP
    if (fstat(bmp_fd, &file_info) == -1) {
        perror("Nu am putut accesa informatiile fisierului");
        close(bmp_fd);
        return 1;
    }//Verific daca am accesat corect informatiile din fisier

    char statistics[1024]; //pt stocare statistici despre fisier
    snprintf(statistics, sizeof(statistics), "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %ld\n%s\n",filename, header.height, header.width, file_info.st_size, file_info.st_uid,ctime(&file_info.st_mtime), file_info.st_nlink, permisii_to_string(file_info.st_mode)); /*am folosit snprintf pt a formatastatistics, afisez: numele fisierului, inaltimea fisierului, lungimea fisierului, dimensiunea fisierului, identificatorul utilizatorului, timpul ultimei modif, nr legatura si drepturile de acces formatate mai sus prin functie */
    int stat_fd = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); //creare fisier statistica.txt
    if (stat_fd == -1) {
        perror("Nu am putut crea fisierul statistica.txt");
        close(bmp_fd);
        return 1;
    }//Verific daca am creat corect fisierul 

    if (write(stat_fd, statistics, strlen(statistics)) == -1) {
        perror("Eroare la scrierea informatiilor in statistica.txt");
    }//Verific daca s-au scris corect informatiile din statistics in statistica.txt

    close(bmp_fd);//inchidere
    close(stat_fd);//inchidere
    return 0;
}


