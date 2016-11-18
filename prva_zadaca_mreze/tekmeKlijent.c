

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "tekmeProtokol.h"

int obradiDODAJ( int sock );
void obradiKOLIKO( int sock );
void obradiBOK( int sock );

#define MAXKLUBOVA     100
int brojKlubova = 0, brojGolova[MAXKLUBOVA], primio[MAXKLUBOVA], zabio[MAXKLUBOVA];
char imeKluba[MAXKLUBOVA][100];
int trebaprimiti;

int main( int argc, char **argv )
{
	if( argc != 3 )
		error2( "Upotreba: %s hostname, port\n", argv[0] );

	char hostname[30]; strcpy( hostname, argv[1] );
	int port;          sscanf( argv[2], "%d", &port );

	// socket...
	int mojSocket = socket( PF_INET, SOCK_STREAM, 0 );
	if( mojSocket == -1 )
		myperror( "socket" );

	// connect...
	struct sockaddr_in adresaServera;

    struct hostent *hostInfo;
    hostInfo = gethostbyname( hostname);
    if( hostInfo == NULL ) myperror( "gethostbyname" );
    struct in_addr binarniIP=*((struct in_addr *)hostInfo->h_addr);

	adresaServera.sin_family = AF_INET;
	adresaServera.sin_port = htons( port );
	adresaServera.sin_addr= binarniIP;
	memset( adresaServera.sin_zero, '\0', 8 );



	if( connect( mojSocket,
		        (struct sockaddr *) &adresaServera,
		        sizeof( adresaServera ) ) == -1 )
		myperror( "connect" );


	printf( "Koliko utakmica je odigrano?\n " );
    int brtekmi;
    scanf( "%d", &brtekmi );

    while(brtekmi<=0)
    {
        printf( "Molim ponovo upisite koliko utakmica je odigrano?\n " );
        scanf( "%d", &brtekmi );
    }

    printf( "Rezultat upisi u obliku:\n ime_Kluba1 ime_Kluba2 x:y \n npr: Manchester_United Manchester_City 3:2\n\n ");

	while( brtekmi )
	{
		if(obradiDODAJ( mojSocket )==1) brtekmi--;
	}

	obradiBOK( mojSocket );
	obradiKOLIKO(mojSocket);
	close( mojSocket );

	return 0;
}

void obradiBOK( int sock )
{
	posaljiPoruku( sock, BOK, "" );

	int vrstaPoruke;
	char *odgovor;
	if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
		error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );

	if( vrstaPoruke != ODGOVOR )
		error1( "Doslo je do pogreske u komunikaciji sa serverom (nije poslao ODGOVOR)...\n" );

	if( strcmp( odgovor, "OK" ) != 0 )
		printf( "Greska: %s\n", odgovor );
	else
	    printf( "OK\n" );
}

int obradiDODAJ( int sock )
{
	char klub1[50], klub2[50];
	int gol1, gol2, a=0;

	printf( "Upisi rezultat: \n" );
	scanf( "%s %s %d : %d", klub1, klub2 , &gol1, &gol2 );

	char poruka[110];
	sprintf( poruka, "%s %s %d %d", klub1, klub2, gol1, gol2 );

	posaljiPoruku( sock, DODAJ, poruka );

	int vrstaPoruke; char *odgovor;
	if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
		error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );

	if( vrstaPoruke != ODGOVOR )
		error1( "Doslo je do pogreske u komunikaciji sa serverom (nije poslao ODGOVOR)...\n" );

	if( strcmp( odgovor, "OK" ) != 0 )
		printf( "Greska: %s\n", odgovor );
	else
	    {printf( "OK\n" ); a=1;}

	free( odgovor );
	return a;
}


void obradiKOLIKO( int sock )
{

    int gol,i,primljeno,zabijeno;
    char klub[50];
	int vrstaPoruke;
	char *odgovor;
	if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
		error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );

	if( vrstaPoruke != BROJKLUBOVA )
		error1( "Doslo je do pogreske u komunikaciji sa serverom (nije poslao broj klubova)...\n" );

	sscanf(odgovor, "%d", &trebaprimiti);

	free( odgovor );

    for(i=0;i<trebaprimiti;i++)
    {
        if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
		error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );

        if( vrstaPoruke != TABLICA )
            error1( "Doslo je do pogreske u komunikaciji sa serverom (nije poslao neki element tablice)...\n" );

        sscanf( odgovor, "%s %d %d %d", klub, &gol, &primljeno, &zabijeno );
        brojGolova[brojKlubova]=gol;
		primio[brojKlubova]=primljeno;
		zabio[brojKlubova]=zabijeno;
        strcpy(imeKluba[brojKlubova],klub);
        brojKlubova++;

        free( odgovor );
    }

    for (i=0; i<brojKlubova;i++)
    printf("%s %d   %d %d\n", imeKluba[i], brojGolova[i], primio[i], zabio[i]);

}

