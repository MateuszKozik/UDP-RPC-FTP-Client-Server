#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<ncurses.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define SERV_PORT 45000

int main(void)
{
    struct sockaddr_in serwer_addres, klient_addres;
    int listenfd;
    int klient_len;
    int recv_len;
    int pid;
    char bufor[1024];

    //utworzenie socketu
    listenfd=socket(AF_INET, SOCK_DGRAM, 0);
	memset((char *) &serwer_addres, '1', sizeof(serwer_addres));

    serwer_addres.sin_family = AF_INET;
    serwer_addres.sin_port = SERV_PORT;
    serwer_addres.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd , (struct sockaddr*)&serwer_addres, sizeof(serwer_addres));

 	//pobranie rozmiaru klienta
 	klient_len = sizeof(klient_addres); 

 	initscr();

 	//okna do wgrywania
 	WINDOW * wRamka = newwin(12, 37, 7, 3);
 	WINDOW * wNaglowek = newwin(5, 35, 1, 4);
	WINDOW * wgrywanie = newwin(10, 35, 8, 4);

	scrollok(wgrywanie, TRUE);

	box(wRamka,0,0);
	box(wgrywanie,0,0);
	box(wNaglowek,0,0);
	wborder(wgrywanie,' ',' ',' ',' ',' ',' ',' ',' ');

 	refresh();
 	wrefresh(wRamka);
 	wrefresh(wgrywanie);

 	wattron(wNaglowek, A_BOLD);
	mvwprintw(wNaglowek,2, 2,"ZAPISYWANIE PLIKOW NA SERWERZE");
	wattroff(wNaglowek, A_BOLD);
	wrefresh(wNaglowek);

	//okna do pobierania
	WINDOW * pRamka = newwin(12, 37, 7, 41);
	WINDOW * pNaglowek = newwin(5, 35, 1, 42);
	WINDOW * pobieranie = newwin(10, 35, 8, 42);

	scrollok(pobieranie, TRUE);

	box(pRamka,0,0);
	box(pNaglowek,0,0);
	box(pobieranie,0,0);
	wborder(pobieranie,' ',' ',' ',' ',' ',' ',' ',' ');

	refresh();
	wrefresh(pRamka);
	wrefresh(pNaglowek);
	wrefresh(pobieranie);

	wattron(pNaglowek, A_BOLD);
	mvwprintw(pNaglowek,2, 4,"POBIERANIE PLIKOW Z SERWERA");
	wattroff(pNaglowek, A_BOLD);
	wrefresh(pNaglowek);
	

    while(1){
			
    	//pobranie typu operacji
    	char typ[20];
		recvfrom(listenfd, bufor, 20, 0, (struct sockaddr *)&klient_addres, &klient_len);
		strcpy(typ, bufor);
		memset(bufor,0,1024);

		//pobieranie pliku od klienta
		if(strcmp(typ, "Wgraj plik") == 0){

		 	//zmienna przechowujaca nazwę pliku
			char plik[20];
				
			//pobranie nazwy pliku
			recvfrom(listenfd, bufor, 20, 0, (struct sockaddr *)&klient_addres, &klient_len);
			strcpy(plik, bufor);
			wprintw(wgrywanie,"\n\nTrwa pobieranie pliku \"%s\"", plik);
			wrefresh(wgrywanie);

			memset(bufor,0,1024);

			//pobranie rozmiaru pliku
			recvfrom(listenfd, bufor, 20, 0, (struct sockaddr *)&klient_addres, &klient_len);
			unsigned long rozmiar = atoi(bufor);
			memset(bufor,0,1024);

			//otworzenie pliku w trybie do zapisu binarnego
			FILE *f;
			f=fopen(plik,"wb");
			int licznik=1;
			  
			//pobieranie pliku
			while(licznik*1024<rozmiar)
			{
			    recvfrom(listenfd, bufor, 1024, 0,(struct sockaddr *)&klient_addres, &klient_len);
			    fwrite(bufor,1024, 1, f);
			    memset(bufor,0,1024);
			    licznik++;
			}

			//pobranie ostatniego bloku danych, ktory zostal pominiety przy wykonywaniu petli
			recvfrom(listenfd, bufor, (rozmiar%1024), 0, (struct sockaddr *)&klient_addres, &klient_len);
			fwrite(bufor,(rozmiar%1024), 1, f);
			memset(bufor,0,1024);

			wprintw(wgrywanie,"\nPlik \"%s\" zostal pobrany", plik);
			wrefresh(wgrywanie);

			//zamknieci pliku
			fclose(f);
		}
	
		//wysylanie pliku do klienta
	   	else if(strcmp(typ, "Pobierz plik") == 0) {

	   		//zmienna przechowująca nazwę pliku
	   		char plik[20];

	   		//pobranie nazwy pliku
	   		recvfrom(listenfd, bufor, 20, 0,(struct sockaddr *)&klient_addres, &klient_len);

	   		//utworzenie procesu potmnego
	   		pid=fork();

	   		//obsluga przekazana do procesu potomnego 
	   		if(pid==0) {
	   			int id = getpid();
				strcpy(plik, bufor);
				wprintw(pobieranie,"\n\n[%d] Plik do wyslania \"%s\"",id, plik);
				wrefresh(pobieranie);

				memset(bufor,0,1024);
			
				char sprawdz[20];

				//otworzenie pliku w trybie do odczytu binarnego
			    FILE *f;
			    f=fopen(plik,"rb");
			    if(access(plik, F_OK)) {
			        strcpy(sprawdz, "n");
			        sendto(listenfd, sprawdz, 20 , 0 ,(struct sockaddr *)&klient_addres, klient_len);

            		wprintw(pobieranie,"\n[%d] Nie ma takiego pliku \"%s\"",id, plik);
            		wrefresh(pobieranie);

            		//zamkniecie socketu
			        close(listenfd);
			        exit(0);

            	}
            	else {
            		strcpy(sprawdz, "j");
					sendto(listenfd, sprawdz, 20 , 0 ,(struct sockaddr *)&klient_addres, klient_len);

            		//pobranie rozmiaru pliku
				    fseek(f, 0, SEEK_END);
	    			unsigned long rozmiar = (unsigned long)ftell(f);
	    			fseek(f, 0, SEEK_SET);

	    			//konwersja rozmiaru na napis
				    char rozmiarPliku[10];
				    sprintf(rozmiarPliku, "%d", rozmiar);

				    //przeslanie rozmiaru pliku do klienta
				    sendto(listenfd, rozmiarPliku, 20 , 0 ,(struct sockaddr *)&klient_addres, klient_len);		      

				    //wczytanie pierwszego bloku danych do buforora
				    fread(bufor, 1024,1,f);

				    //przesylanie kolenych blokow danych az do zakonczenia pliku
				    int licznik=1;
				    while(licznik*1024<rozmiar){

				    	//uspienie procesu
				    	sleep(1);

				       	sendto(listenfd, bufor, 1024 , 0 ,(struct sockaddr *)&klient_addres, klient_len);
				        memset(bufor,0,1024);
				        fread(bufor, 1024,1,f);
				        licznik++;
				    }

				    //przeslanie ostatniego bloku danych, ktory zostal pominiety przy wykonywaniu petli
				    fread(bufor, (rozmiar % 1024),1,f);
				    sendto(listenfd, bufor, (rozmiar % 1024) , 0 ,(struct sockaddr *)&klient_addres, klient_len);
				    memset(bufor,0,1024);

				    //zamkniecie pliku
				    fclose(f);

				    wprintw(pobieranie,"\n[%d] Plik \"%s\" zostal wyslany",id, plik);
				    wrefresh(pobieranie);

				    //zamkniecie socketu
				    close(listenfd);
				    exit(0);
            	}
	   		}
	   		//blad podczas tworzenia procesu potomnego
	   		if(pid < 0){
	   			wprintw(pobieranie, "\nNie udalo sie utworzyc nowego procesu");
	   			wrefresh(pobieranie);
	   			exit(1);
	   		}	
	   	}
    }

  	//zamkniecie socketu
    close(listenfd);

    return 0;
}