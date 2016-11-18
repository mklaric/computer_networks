// Server.c

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

void komunicirajSaKlijentom( int sock );
void obradiDODAJ( int sock, char *poruka );
void sortiraj();
void obradiKOLIKO( int sock );

#define MAXKLUBOVA     100
int brojKlubova = 0, brojGolova[MAXKLUBOVA], primio[MAXKLUBOVA], zabio[MAXKLUBOVA];
char imeKluba[MAXKLUBOVA][100];


int main( int argc, char **argv )
{
	if( argc != 2 )
		error2( "Upotreba: %s port\n", argv[0] );

	int port; sscanf( argv[1], "%d", &port );

	// socket...
    int listenerSocket = socket( PF_INET, SOCK_STREAM, 0 );
	if( listenerSocket == -1 )
		myperror( "socket" );

	// bind...
	struct sockaddr_in mojaAdresa;

	mojaAdresa.sin_family = AF_INET;
	mojaAdresa.sin_port = htons( port );
	mojaAdresa.sin_addr.s_addr = INADDR_ANY;
	memset( mojaAdresa.sin_zero, '\0', 8 );

	if( bind( listenerSocket,
		      (struct sockaddr *) &mojaAdresa,
		      sizeof( mojaAdresa ) ) == -1 )
		myperror( "bind" );

	// listen
	if( listen( listenerSocket, 10 ) == -1 )
		myperror( "listen" );

	// accept

        struct sockaddr_in klijentAdresa;
		int lenAddr = sizeof( klijentAdresa );

		int commSocket = accept( listenerSocket,(struct sockaddr *) &klijentAdresa, &lenAddr );

		if( commSocket == -1 )
			myperror( "accept" );

		char *dekadskiIP = inet_ntoa( klijentAdresa.sin_addr );
		printf( "Prihvatio konekciju od %s [socket=%d]\n", dekadskiIP, commSocket );

		for(int i=0;i<100;i++)
		{
			primio[i]=0;
			zabio[i]=0;
		}

		komunicirajSaKlijentom( commSocket );

		close( commSocket );
		printf( "Zavrsio komunikaciju sa %s [socket=%d]\n", dekadskiIP, commSocket );


	return 0;
}

void komunicirajSaKlijentom( int sock )
{
	int vrstaPoruke, gotovo = 0;
	char *poruka;

	while( !gotovo )
	{
		if( primiPoruku( sock, &vrstaPoruke, &poruka ) != OK )
		{
			printf( "Greska u komunikaciji sa klijentom [socket=%d]...\n", sock );
			gotovo = 1;
			continue;
		}

		switch( vrstaPoruke )
		{
			case DODAJ: obradiDODAJ( sock, poruka ); break;
			case BOK: posaljiPoruku( sock, ODGOVOR, "OK" ); gotovo = 1; break;
			default: posaljiPoruku( sock, ODGOVOR, "Nepostojeci kod poruke!\n" );
		}

		free( poruka );
	}
	sortiraj();
	obradiKOLIKO( sock );

}

void sortiraj()
{
    int i,j,maxi;
    int gol,primljeno,zabijeno; char klub[100];
    for (i=0; i<brojKlubova; i++)
    {
        maxi=i;
        for(j=i+1;j<brojKlubova;j++)
        {
            if(brojGolova[j]>brojGolova[maxi]) maxi=j;
        }
        gol=brojGolova[i]; strcpy(klub,imeKluba[i]); primljeno=primio[i]; zabijeno=zabio[i];
        brojGolova[i]=brojGolova[maxi]; strcpy(imeKluba[i],imeKluba[maxi]); zabio[i]=zabio[maxi]; primio[i]=primio[maxi];
        brojGolova[maxi]=gol; strcpy(imeKluba[maxi],klub); zabio[maxi]=zabijeno; primio[maxi]=primljeno;

    }
}

void obradiDODAJ( int sock, char *poruka )
{
	int gol1, gol2;
	char klub1[50] ,klub2[50];

	if( sscanf( poruka, "%s %s %d %d\n", klub1, klub2, &gol1, &gol2 ) != 4 || gol1 < 0 || gol2<0 || strcmp(klub1, klub2)==0 )
	{
		posaljiPoruku( sock, ODGOVOR, "Pogresan unos klubova ili golova" );
		return;
	}

    // prvo gleda imamo li te klubove u listi, zatim one koje nemamo dodaje u listu
	int i, postoji1 = 0, postoji2=0;
	for( i = 0; i < brojKlubova; ++i )
	{
	    if( strcmp( imeKluba[i], klub1 ) == 0 )
		{
			zabio[i]+=gol1;
			primio[i]+=gol2;
		 	postoji1 = 1;
			break;}
	}

    for( i = 0; i < brojKlubova; ++i )
	{
		if( strcmp( imeKluba[i], klub2 ) == 0 )
		{ 
			zabio[i]+=gol2;
			primio[i]+=gol1;
			postoji2 = 1;
			break;	}
	}

	if( postoji1==0 || postoji2==0 )
	{
		if( brojKlubova+1 < MAXKLUBOVA )
		{
		    if(postoji1==0)
			{ strcpy( imeKluba[brojKlubova], klub1 );
			brojGolova[brojKlubova] = 0;
			zabio[brojKlubova]+=gol1;
			primio[brojKlubova]+=gol2;
			++brojKlubova;}

			if(postoji2==0)
			{strcpy( imeKluba[brojKlubova], klub2 );
			brojGolova[brojKlubova] = 0;
			zabio[brojKlubova]+=gol2;
			primio[brojKlubova]+=gol1;
			++brojKlubova;}

        }
		else
			posaljiPoruku( sock, ODGOVOR, "Prepunili smo listu klubova" );
	}

	if(gol1>gol2) for( i = 0; i < brojKlubova; ++i )
                   if( strcmp( imeKluba[i], klub1 ) == 0 )
                   {
                       brojGolova[i]+= 2;
                       break;
                    }
    if(gol1==gol2)
    {
        for( i = 0; i < brojKlubova; ++i )
            if( strcmp( imeKluba[i], klub1 ) == 0 )
            {
                brojGolova[i]+= 1;
                break;
            }

        for( i = 0; i < brojKlubova; ++i )
        if( strcmp( imeKluba[i], klub2 ) == 0 )
            {
                brojGolova[i]+= 1;
                break;
            }

    }

    if(gol2>gol1) for( i = 0; i < brojKlubova; ++i )
                    if( strcmp( imeKluba[i], klub2 ) == 0 )
                    {
                      brojGolova[i]+= 2;
                      break;
                    }

    posaljiPoruku( sock, ODGOVOR, "OK" );
}



void obradiKOLIKO( int sock )
{
	int i, postoji = 0;
	char poruka[10];
	sprintf(poruka, "%d", brojKlubova);
	posaljiPoruku(sock, BROJKLUBOVA, poruka);

	for( i = 0; i < brojKlubova; ++i )
    {
        char novaPoruka[200];
		sprintf( novaPoruka, "%s %d %d %d", imeKluba[i] , brojGolova[i], primio[i], zabio[i] );
		posaljiPoruku( sock, TABLICA, novaPoruka );
	}

}
