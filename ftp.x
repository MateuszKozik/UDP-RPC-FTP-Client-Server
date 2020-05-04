struct dane{
	char przesylaneDane[1024];
	char nazwaPliku[30];
	int wielkoscPliku;
	int licznik;
};

struct pobranieNazwy {
	char nazwaPliku[30];
};

struct daneKontrolne {
	char nazwaPliku[30];
	int polozenie;
};

program FTP {
    version FTP_VER{
	int 	wysylaniePliku(dane)=1;
	int		rozmiarPliku(pobranieNazwy)=2;
	int		czyPlikIstnieje(pobranieNazwy)=3;
	dane    pobieraniePliku(daneKontrolne)=4;
    }=1;
}=0x30000004;	
