#include <iostream>
#include <cstdlib>
#include <vector>
#include <queue>
#include "player.h"
#include "environment.h"
#include <algorithm>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>

using namespace std;

const double masinf=9999999999.0, menosinf=-9999999999.0;
int DIFERENCIAFICHAS=1;
int PESOBLOQUEDIAGONAL=2;
int PESOBLOQUEVERTICAL= 2;
int PESOBLOQUEHORIZONTAL= 3;// Secuencias que nos bloquean una columna, son muy perjudiciales, nos limitan los movimientos
int FICHASDESTRUIDAS= 1;
int EXPLOTARBOMBA=1000;


void ActualizarPesos(const Environment estado,int fichasMias, int fichasRival, int jugador){

    // Si tengo menos fichal que el rival y el tablero tiene bastantes fichas somos mas agresivos
   if(fichasMias+3 < fichasRival && (fichasMias+fichasRival) > 21 && estado.N_Jugada()==40){
       PESOBLOQUEDIAGONAL+=5;
       PESOBLOQUEVERTICAL+=5;
       EXPLOTARBOMBA=2;
       cout << "SOY MAS AGRESIVO " << jugador << endl;
       char a;
       cin >> a;
   }
}
// Constructor
Player::Player(int jug){
    jugador_=jug;
}

// Actualiza el estado del juego para el jugador
void Player::Perceive(const Environment & env){
    actual_=env;
}

double Puntuacion(int jugador, const Environment &estado){
    double suma=0;

    for (int i=0; i<7; i++)
      for (int j=0; j<7; j++){
         if (estado.See_Casilla(i,j)==jugador){
            if (j<3)
               suma += j;
            else
               suma += (6-j);
         }
      }

    return suma;
}


// Funcion de valoracion para testear Poda Alfabeta
double ValoracionTest(const Environment &estado, int jugador){
    int ganador = estado.RevisarTablero();

    if (ganador==jugador)
       return 99999999.0; // Gana el jugador que pide la valoracion
    else if (ganador!=0)
            return -99999999.0; // Pierde el jugador que pide la valoracion
    else if (estado.Get_Casillas_Libres()==0)
            return 0;  // Hay un empate global y se ha rellenado completamente el tablero
    else
          return Puntuacion(jugador,estado);
}

// ------------------- Los tres metodos anteriores no se pueden modificar

// Funcion que devuelve los niveles que estan completados
void ContarFichas(const Environment estado, int &m, int &r, int jugador){
    m=r=0;
    for(int i = 0 ; i < 7 ; i++)
        for(int j = 0 ; j < 7 ; j++)
            if(estado.See_Casilla(i,j)%3 == jugador)
                m++;
            else if( estado.See_Casilla(i,j)%3 == (jugador+1)%3)
                r++;

}


// Funcion para contar los niveles completos
int NivelesCompletos(const Environment estado){
    bool completo=true;
    int niveles=0;
    for(int i = 0 ; i < 5 && completo ; i++){ // fila
        for(int j = 0 ; j < 7 && completo; j++){ // columna
            if(estado.See_Casilla(i,j)!= 1 && estado.See_Casilla(i,j)!=2){ //Es alguno de los dos jugadores
                completo=false;
            }
        }
        if(completo)
            niveles++;
    }

    return niveles;
}


int CalcularValoracion(int tam,int opcion){
    int valoracion = 0 ;
    if(opcion ==1)
        switch (tam){
            case 1:
                valoracion+=1;
                break;
            case 2:
                valoracion+=2;
                break;
            case 3:
                valoracion+=6;
                break;

        }
    else
        switch (tam){
            case 1:
                valoracion+=0;
                break;
            case 2:
                valoracion+=2;
                break;
            case 3:
                valoracion+=6;
                break;

        }

    return valoracion;

}

double ValoracionContarFichas(const Environment estado, int jugador){
    int fichasMias=0, fichasRival=0;

    for(int f=0; f < 7 ; f++)
        for(int c=0; c < 7 ; c++)
            if(estado.See_Casilla(f,c)==jugador)
                fichasMias++;
            else if(estado.See_Casilla(f,c)== (jugador+1)%3)
                fichasRival++;

    return (fichasRival-fichasMias)*DIFERENCIAFICHAS; // Diferencia fichas

}
// Vamos a comprobar los grupos de fichas que existen y a valorar la situacion
int ValoracionAgrupacionesFichas(const Environment estado, int jugador){
    int valoracion=0;
    int mult=1;

    /*Ahora buscamos grupos de fichas de ambos jugadores de forma que los grupos de mi jugador restan y los del jugador rival suman */

    int tamMiGrupoH=0, tamRivalGrupoH=0, tamMiGrupoV=0, tamRivalGrupoV=0;

    for(int f=0 ; f < 7 ; f++){
        tamMiGrupoH=tamRivalGrupoH=tamMiGrupoV=tamRivalGrupoV=0;
        for(int c=0; c < 7 ; c++){
            //HORIZONTAL
           /* //Para mi jugador
            if(estado.See_Casilla(f,c)==jugador)
                tamMiGrupoH++;
            else{
                valoracion -= CalcularValoracion(tamMiGrupoH,1)*PESOBLOQUEHORIZONTAL;
                tamMiGrupoH=0;
            }
            */
            //Para el rival
            if(estado.See_Casilla(f,c)==(jugador+1)%3)
                tamRivalGrupoH++;
            else{
                valoracion+= CalcularValoracion(tamMiGrupoH,2)*PESOBLOQUEHORIZONTAL;
                tamRivalGrupoH=0;
            }

            //VERTICAL
            //Para mi jugador
            /*
            if(estado.See_Casilla(c,f)==jugador)
                tamMiGrupoV++;
            else{
                valoracion -= CalcularValoracion(tamMiGrupoV,1)*PESOBLOQUEVERTICAL;
                tamMiGrupoV=0;
            }*/
            //Para el rival
            if(estado.See_Casilla(c,f)==(jugador+1)%3)
                tamRivalGrupoV++;
            else{
                valoracion += CalcularValoracion(tamRivalGrupoV,2)*PESOBLOQUEVERTICAL;
                tamRivalGrupoV=0;
            }
        }
    }

   // DIAGONAL DE ABAJO-IZQUIERDA HACIA ARRIBA-DERECHA
    int tamMiGrupoD=0, tamRivalGrupoD=0, tamMiGrupoD2=0, tamRivalGrupoD2=0;
    int c=0;
    for(int k=0; k < 4 ; k++){ // Diagonal 0 - 1,4 - 2,5 - 3,6
        tamMiGrupoD=tamRivalGrupoD=0;
        for(int f = 0,c=k ; f < 7 && c < 7 ; f++,c++){

            //Para mi jugador diagonales 0,1,2,3
           /* if(estado.See_Casilla(f,c)==jugador)
                tamMiGrupoD++;
            else{
                valoracion -= CalcularValoracion(tamMiGrupoD,1)*PESOBLOQUEDIAGONAL;
                tamMiGrupoD=0;
            }
            //Diagonales 4,5,6
            if( f!=c){
                if(estado.See_Casilla(c,f)==jugador)
                    tamMiGrupoD2++;
                else{
                    valoracion-= CalcularValoracion(tamMiGrupoD2,1)*PESOBLOQUEDIAGONAL;
                    tamMiGrupoD2=0;
                }
            }*/

            //Para el rival diagonales 0,1,2,3
            if(estado.See_Casilla(f,c)==(jugador+1)%3)
                tamRivalGrupoD++;
            else{
                valoracion+= CalcularValoracion(tamRivalGrupoD2,2)*PESOBLOQUEDIAGONAL;
                tamRivalGrupoD=0;
            }
             //Diagonales 4,5,6
            if(f!=c){
                if(estado.See_Casilla(c,f)==jugador)
                    tamRivalGrupoD2++;
                else{
                    valoracion-= CalcularValoracion(tamRivalGrupoD2,2)*PESOBLOQUEDIAGONAL;
                    tamRivalGrupoD2=0;
                }
            }
        }
    }

    // DIAGONAL DE ARRIBA-IZQUIERDA HACIA ABAJO-DERECHA
    for(int k=0; k < 4 ; k++){ // Diagonal 0 - 1,4 - 2,5 - 3,6
        tamMiGrupoD=tamRivalGrupoD=0;
        for(int f = 6, c=k ; f >= 0 && c < 7 ; f--,c++){

            //Para mi jugador diagonales 0,1,2,3
           /* if(estado.See_Casilla(f,c)==jugador)
                tamMiGrupoD++;
            else{
                valoracion-= CalcularValoracion(tamMiGrupoD,1)*PESOBLOQUEDIAGONAL;
                tamMiGrupoD=0;
            }
            //Diagonales 4,5,6
            if( f+c!=6){
                if(estado.See_Casilla(c,f)==jugador)
                    tamMiGrupoD2++;
                else{
                    valoracion-= CalcularValoracion(tamMiGrupoD2,1)*PESOBLOQUEDIAGONAL;
                    tamMiGrupoD2=0;
                }
            }
            */
            //Para el rival diagonales 0,1,2,3
            if(estado.See_Casilla(f,c)==(jugador+1)%3)
                tamRivalGrupoD++;
            else{
                valoracion+= CalcularValoracion(tamRivalGrupoD,2)*PESOBLOQUEDIAGONAL;
                tamRivalGrupoD=0;
            }
             //Diagonales 4,5,6
            if(f!=c){
                if(estado.See_Casilla(c,f)==jugador)
                    tamRivalGrupoD2++;
                else{
                    valoracion-= CalcularValoracion(tamRivalGrupoD2,2)*PESOBLOQUEDIAGONAL;
                    tamRivalGrupoD2=0;
                }
            }
        }
    }

    return valoracion;
}


// Función que busca donde esta la bomba de un jugador si esta existe
bool BuscarBomba(const Environment estado, int jugador, int f, int c){

    bool res= false;
    f=c=0;
    for(int i = 0 ; i < 7 && !res ; i++){
        for(int j = 0 ; j < 7 && !res; j++){
            if(estado.See_Casilla(i,j)==(jugador+3)){
                res=true;
                f=i;
                c=j;
            }

        }

    }
    return res;
}


// Para las jugadas maestras sabemos que existe una bomba de jugador
/* ¿Que son jugadas maestras? Son jugadas que nos encaminan hacia la victoria, se da importancia a poner fichas en la misma fila donde tenemos la bomba,
colocar la bomba sobre fichas de nuestro rival, poner la bomba en una diagonal del rival, no poner una ficha sobre nuestra bomba */
int JugadasMaestras(const Environment estado, int jugador){
    int f=0 , c=0, valoracion=0,fichas=1,posicionesRestantes=0;
    bool salir=false;
    if( BuscarBomba(estado, jugador, f, c) && c < 7 && f < 7){
        /*
        //Dar importancia a jugadas donde nuestras fichas estan en la misma fila que nuestra bomba
        for(int c=0; c < 7 ; c++){
            if(estado.See_Casilla(f,c)==jugador)
                valoracion+=FICHASDESTRUIDAS;
        }
         //Quitar importantancia a jugadas donde mis fichas estan encima de mi ficha bomba
        if(f<7 && estado.See_Casilla(f+1,c) == jugador ){
            valoracion -=500;
        }
        */
        //Poner una bomba en la ultima fila se considerara muy malo
        if(f==6)
            valoracion-=500;

        //Dar importancia a jugadas con ficha bomba sobre fichas enemigas
        if( f>0 && f <7 && estado.See_Casilla(f-1,c)== (jugador+1)%3 ){
            //¿Cuantas fichas del rival hay debajo?
            fichas=1;
            posicionesRestantes=0;
            salir=false;

            for(int i = 2 ; i <= 3 && (f-i) >=0 ; i++){
                if(estado.See_Casilla(f-i,c) == (jugador+1)%3){
                    fichas++;
                }
            }
            // Comprobamos que hay por arriba

            for(int i = 1 ; i < 7 && (f+i)<7 && !salir  ; i++){
                if(estado.See_Casilla(f+i,c)==(jugador+1)%3)
                    fichas++;
                else if(estado.See_Casilla(f+i,c)==0)
                    posicionesRestantes++;
                else
                    salir=true;
            }
            if(fichas >=4 && !estado.Have_BOOM((jugador+1)%3)) // Quiere decir que si exploto la bomba he ganado
                valoracion+= masinf-10;
            /*
            else if(fichas+posicionesRestantes >= 4 ){ // Hay posibilidades de ganar por aqui
                valoracion+=fichas*5;
            }
            */
        }

        //Dar importancia a jugadas donde mi bomba esta en una diagonal de abajo hacia arriba del enemigo
        if( (f>0 && c>0 && estado.See_Casilla(f-1,c-1) == (jugador+1)%3 && estado.See_Casilla(f+1,c)!=jugador) ||( f<7 && c<7 && estado.See_Casilla(f+1,c+1) == (jugador+1)%3 && estado.See_Casilla(f+1,c)!=jugador) ){
            fichas =1;
            posicionesRestantes=0;
            salir=false;
            // Comprobamos lo que tiene por debajo
             for(int i = 2 ; i < 3 && f-i >=0 && c-i >=0 && !salir ; i++){
                if(estado.See_Casilla(f-i,c-i) == (jugador+1)%3)
                    fichas++;
                else
                    salir = true;

            }
            salir=false;
            // Comprobamos que hay en la diagonal por arriba
            for(int i = 1 ; i < 7 && f+i <7 && c+i <7 && !salir  ; i++){
                if(estado.See_Casilla(f+i,c+i) == (jugador+1)%3){
                    fichas++;
                }
                else if(estado.See_Casilla(f+i,c+i) == 0){
                    posicionesRestantes++;
                }
                else{
                    salir=true;
                }
            }
            if(fichas >=4 && f < 6 && estado.See_Casilla(f+1,c)==(jugador+1)%3 && !estado.Have_BOOM((jugador+1)%3)) // Si exploto la bomba gano la partida
                valoracion+=masinf-10;
            /*
            else if(fichas+posicionesRestantes >= 4){ // Hay posibilidades de ganar por aqui
                valoracion+=fichas*5;
            }
            */
        }

       //Dar importancia a jugadas donde mi bomba esta en una diagonal de arriba hacia abajo del enemigo
        if( (f>0 && c<6 && estado.See_Casilla(f-1,c+1) == (jugador+1)%3) ||( f<6 && c>0 && estado.See_Casilla(f+1,c-1) == (jugador+1)%3) ){
            fichas =1;
            posicionesRestantes=0;
            salir=false;
            // Comprobamos lo que tiene por debajo
             for(int i = 2 ; i <= 3 && f-i >=0 && c+i <7  ; i++){
                if(estado.See_Casilla(f-i,c+i) == (jugador+1)%3){
                    fichas++;
                }
            }
            // Comprobamos que hay en la diagonal por arriba
            for(int i = 1 ; i < 7 && f+i <7 && c-i >=0 && !salir  ; i++){
                if(estado.See_Casilla(f+i,c-i) == (jugador+1)%3){
                    fichas++;
                }
                else if(estado.See_Casilla(f+i,c-i) == 0){
                    posicionesRestantes++;
                }
                else{
                    salir=true;
                }
            }
            if(fichas >=4 && f < 6 && estado.See_Casilla(f+1,c)==(jugador+1)%3 && !estado.Have_BOOM((jugador+1)%3))
                valoracion+=masinf-10;
            /*
            else if(fichas+posicionesRestantes >= 4){ // Hay posibilidades de ganar por aqui
                valoracion+=fichas*5;
            }
            */


        }

    }
    return valoracion;
}



/*Estas situaciones son algunas en las que vemos una secuencia de fichas del adversario que le impide tirar en una columna,
si llegamos a alguna situacion de estas podemos dominar */
double SituacionesDeseables(const Environment estado, int jugador){
    double valoracion=0;
    bool encontrada=false, secuenciaDe3=false,hayBomba=false, esVacia=false;
    int oponente =(jugador+1)%3, p,mias, suyas;

    for(int f=0; f < 7 ; f++){
            // Buscamos algo asi 2 2 2 0 2 2 2
            encontrada=true;
            for(int c=0; c < 7 && encontrada; c++){
                if(c!=3){
                    if(estado.See_Casilla(f,c ) != oponente ) // Tiene que ser oponente
                        encontrada = false;
                }
                else{
                    if(estado.See_Casilla(f,c) != 0) // Tiene que ser 0
                        encontrada = false;
                }
            }
            if(encontrada)
                return 10000; // Situacion perfecta



            // Buscamos algo asi 1/2 X 2 2 2 X 1/2     1/2 1/2 X 2 2 2 X      X 2 2 2 X 1/2 1/2   2 2 2 X 1/2 1/2 1/2  1/2 1/2 1/2 X 2 2 2
            // tenemomos que comprobar si existe la secuencia de 3 en horizontal
            secuenciaDe3=false;
            for(int c=0; c < 5 && !secuenciaDe3 ; c++){
                    if(estado.See_Casilla(f,c) == oponente && estado.See_Casilla(f,c+1)== oponente && estado.See_Casilla(f,c+2)== oponente){
                        secuenciaDe3=true;
                        p=c; // Aqui comienza la secuencia
                    }
            }

            hayBomba=false;
            for(int i = 0 ; i < 7 && !hayBomba && secuenciaDe3; i++)
                hayBomba= estado.See_Casilla(f,i) == 4 || estado.See_Casilla(f,i)== 5;

            if(secuenciaDe3&&!hayBomba){ // En este punto sabemos que no hay bomba y hay secuencia de 3
                mias=suyas=0;
                if(p==0){ // 2 2 2 X 1/2 1/2 1/2
                        if( estado.See_Casilla(f,3) != jugador ){ // O mi bomba o nada
                            esVacia=false;
                            for(int i = 4 ; i < 7 && !esVacia; i++){ // 4 5 6
                                if(estado.See_Casilla(f,p+i) == oponente){
                                    suyas++;
                                }
                                else if( estado.See_Casilla(f,p+i) == 0){
                                    esVacia=true;
                                }
                                else{
                                    mias++;
                                }

                            }
                            if(!esVacia ){

                                    if (mias < suyas)
                                        return 5000; // Situacion deseable
                                    else if( mias ==3){
                                        return -10000; // Condenado a empatar
                                    }

                            }
                        }
                }
                else if(p==1){ // X 2 2 2 X 1/2 1/2
                        if(estado.See_Casilla(f,0)!= jugador && estado.See_Casilla(f,4)!=jugador) // O nada o mi bomba
                            if(estado.See_Casilla(f,5)!= 0 && estado.See_Casilla(f,6)!=0)
                                return 10000; // Situacion perfecta

                }
                else if(p==2){ // 1/2 X 2 2 2 X 1/2
                        if(estado.See_Casilla(f,1)!=jugador && estado.See_Casilla(f,5)!= jugador) // O nada o mi bomba
                            if(estado.See_Casilla(f,0)!=0 && estado.See_Casilla(f,6)!=0)
                                return 10000; // Situacion perfecta
                }
                else if(p==3){ // 1/2 1/2 X 2 2 2 X
                        if(estado.See_Casilla(f,2)!= jugador && estado.See_Casilla(f,6))
                            if(estado.See_Casilla(f,0)!= 0 && estado.See_Casilla(f,1)!=0)
                                return 10000; // Situacion perfecta-

                }
                else if(p==4){ // 1/2 1/2 1/2 X 2 2 2

                        if( estado.See_Casilla(f,p-1)!= jugador ){ // O mi bomba o nada
                            esVacia=false;
                            for(int i = 4 ; i < 7 && !esVacia; i++){ // 0 1 2

                                if(estado.See_Casilla(f,i-p) == oponente){
                                    suyas++;
                                }
                                else if( estado.See_Casilla(f,i-p) == 0){
                                    esVacia=true;
                                }
                                else{
                                    mias++;
                                }
                            }
                           if(!esVacia ){

                                    if (mias < suyas){
                                        return 5000; // Situacion deseable
                                    }
                                    else if( mias == 3){
                                        return -10000; // condenado a empatar
                                    }

                           }
                        }

                }

            }

            return 0;


    }
}


//__________________________________________________________________________________________________
//               OPCION PRIMERA HEURISTICA
//__________________________________________________________________________________________________

// Funcion heuristica (ESTA ES LA QUE TENEIS QUE MODIFICAR)
double Valoracion(const Environment &estado, int jugador,int prof){
    double valoracion=0;
    int ganador = estado.RevisarTablero();
    int m, s;

    if (ganador==jugador)
            return 99999999.0-prof; // Gana el jugador que pide la valoracion
    else if (ganador!=0)
            return -99999999.0+prof; // Pierde el jugador que pide la valoracion
    else if (estado.Get_Casillas_Libres()==0)
            return 0;  // Hay un empate global y se ha rellenado completamente el tablero



    // A partir de aquí comienza las valoraciones de las jugadas que no son victoria, derrota o empate
    // Valoramos la cantidad de fichas y las agrupaciones entre ellas
   // valoracion += ValoracionAgrupacionesFichas(estado, jugador);
     ContarFichas(estado,m,s,jugador);
     int niveles = NivelesCompletos(estado);

     valoracion = ValoracionContarFichas(estado, jugador)*niveles*s/4; // Esta valoración aumenta conforme mas niveles hay completos

    // Valoramos situaciones en las que nuestro rival tiene bloquedas columnas
    valoracion += SituacionesDeseables(estado,jugador);


    valoracion += ValoracionAgrupacionesFichas(estado,jugador);
/*
    if(estado.Have_BOOM(jugador)){
        valoracion += JugadasMaestras(estado,jugador);
    }
     if(estado.Have_BOOM((jugador+1)%3)){
        valoracion -= JugadasMaestras(estado,(jugador+1)%3);
    }*/
    if(jugador==2)
        if(estado.N_Jugada()%4==0 && estado.Have_BOOM(jugador)==0)
            valoracion += EXPLOTARBOMBA*1000;// Porque hemos explotado la bomba, y vamos a colocar otra


    return valoracion;
}



// Funcion heuristica (ESTA ES LA QUE TENEIS QUE MODIFICAR)
double Valoracion2(const Environment &estado, int jugador,int prof){
    double valoracion=0;
    int ganador = estado.RevisarTablero();
    int m, r;

    if (ganador==jugador)
            return 99999999.0-prof; // Gana el jugador que pide la valoracion
    else if (ganador!=0)
            return -99999999.0+prof; // Pierde el jugador que pide la valoracion
    else if (estado.Get_Casillas_Libres()==0)
            return 0;  // Hay un empate global y se ha rellenado completamente el tablero



    // A partir de aquí comienza las valoraciones de las jugadas que no son victoria, derrota o empate
    // Valoramos la cantidad de fichas y las agrupaciones entre ellas
    ContarFichas(estado, m, r, jugador);
    valoracion += ValoracionContarFichas(estado, jugador);

    // Valoramos situaciones en las que nuestro rival tiene bloquedas columnas
   // valoracion += SituacionesDeseables(estado,jugador);

    // Valoramos situaciones en las que nuestro rival tiene bloquedas columnas
   // valoracion += SituacionesDeseables(estado,jugador);

/*
    if(estado.Have_BOOM(jugador)){
        valoracion += JugadasMaestras(estado,jugador);
    }
     if(estado.Have_BOOM((jugador+1)%3)){
        valoracion -= JugadasMaestras(estado,(jugador+1)%3);
    }
    if(estado.N_Jugada()%4==0 && estado.Have_BOOM(jugador)==0)
        valoracion+=EXPLOTARBOMBA;// Porque hemos explotado la bomba, y vamos a colocar otra
*/
    return valoracion;
}

// Algotirmo minimax con poda_alfabeta
/* Nos gustaria que cada 4 tiradas se explotara la bomba*/
double Poda_AlfaBeta(Environment actual_, int jugador_, int profundidad, int PROFUNDIDAD_ALFABETA, Environment::ActionType &accion, double alpha, double beta) {

    //Elegimos dentro de PROFUNDIDAD_ALFABETA movimientos que tablero es el mas beneficioso para nosotros
    if(profundidad==PROFUNDIDAD_ALFABETA || actual_.JuegoTerminado()  ) { // Estado terminal
            if( jugador_ == 2 )
                return  Valoracion(actual_, jugador_,profundidad);
            else
                return Valoracion(actual_,jugador_,profundidad); // Valoracion2(actual_,jugador_,profundidad);

    }

    Environment V[8]; // PUT1, PUT2, PUT3, PUT4, PUT5, PUT6, PUT7, BOOM
    int n_gen = actual_.GenerateAllMoves(V); // posibles movimientos del espacio de movimientos que tenemos, sirve para saber los hijos que tiene el nodo actual
    double aux;


       if(profundidad%2==0) { // Esto es nodo MAX 0 2 4 6
            for(int i=0; i<n_gen; i++) {
                aux=Poda_AlfaBeta(V[i],jugador_,profundidad+1,PROFUNDIDAD_ALFABETA,accion,alpha,beta); // Llamamos recursivamente al siguiente nodo hijo
                if(aux>alpha) {
                    alpha=aux;
                    if(profundidad==0) accion=static_cast< Environment::ActionType >(V[i].Last_Action(jugador_)); // La accion elegida para llegar hasta el nodo hijo que tenia un valor mayor que alpha
                }
                if(beta<=alpha) return alpha; // Poda, se para la recursividad de este nodo
            }
            return alpha;

        }
        else{ // MIN 1 3 5 7
            for(int i=0; i<n_gen; i++) {
                aux=Poda_AlfaBeta(V[i],jugador_,profundidad+1,PROFUNDIDAD_ALFABETA,accion,alpha,beta);
                if(aux<beta) {
                    beta=aux;
                    if(profundidad==0) accion=static_cast<Environment::ActionType >(V[i].Last_Action(jugador_)); // La accion elegida para llegar hasta el nodo que tenia unv valor menor que beta
                }
                if(beta<=alpha) return beta; // Poda, se para la recursividad de este nodo
            }
            return beta;
        }

}

//__________________________________________________________________________________________________
//
//__________________________________________________________________________________________________





// Invoca el siguiente movimiento del jugador
Environment::ActionType Player::Think(){
    int m, r ;
    const int PROFUNDIDAD_MINIMAX = 6;  // Umbral maximo de profundidad para el metodo MiniMax
    const int PROFUNDIDAD_ALFABETA = 8; // Umbral maximo de profundidad para la poda Alfa_Beta

    Environment::ActionType accion; // acción que se va a devolver
    bool aplicables[8]; // Vector bool usado para obtener las acciones que son aplicables en el estado actual. La interpretacion es
                        // aplicables[0]==true si PUT1 es aplicable
                        // aplicables[1]==true si PUT2 es aplicable
                        // aplicables[2]==true si PUT3 es aplicable
                        // aplicables[3]==true si PUT4 es aplicable
                        // aplicables[4]==true si PUT5 es aplicable
                        // aplicables[5]==true si PUT6 es aplicable
                        // aplicables[6]==true si PUT7 es aplicable
                        // aplicables[7]==true si BOOM es aplicable



    double valor; // Almacena el valor con el que se etiqueta el estado tras el proceso de busqueda.

    int n_act; //Acciones posibles en el estado actual


    n_act = actual_.possible_actions(aplicables); // Obtengo las acciones aplicables al estado actual en "aplicables"
    int opciones[10];

    // Muestra por la consola las acciones aplicable para el jugador activo
    //actual_.PintaTablero();
   /* cout << " Acciones aplicables ";
    (jugador_==1) ? cout << "Verde: " : cout << "Azul: ";
    for (int t=0; t<8; t++)
      if (aplicables[t])
         cout << " " << actual_.ActionStr( static_cast< Environment::ActionType > (t)  );
    cout << endl;
    */

    //--------------------- COMENTAR Desde aqui
    /*
    cout << "\n\t";
    int n_opciones=0;
    JuegoAleatorio(aplicables, opciones, n_opciones);

    if (n_act==0){
      (jugador_==1) ? cout << "Verde: " : cout << "Azul: ";
      cout << " No puede realizar ninguna accion!!!\n";
      accion = Environment::actIDLE;
    }
    else if (n_act==1){
           (jugador_==1) ? cout << "Verde: " : cout << "Azul: ";
            cout << " Solo se puede realizar la accion "
                 << actual_.ActionStr( static_cast< Environment::ActionType > (opciones[0])  ) << endl;
            accion = static_cast< Environment::ActionType > (opciones[0]);

         }
         else {
            int aleatorio = rand()%n_opciones;
            cout << " -> " << actual_.ActionStr( static_cast< Environment::ActionType > (opciones[aleatorio])  ) << endl;
            accion = static_cast< Environment::ActionType > (opciones[aleatorio]);
         }
    */
   //--------------------- COMENTAR Hasta aqui


    //--------------------- AQUI EMPIEZA LA PARTE A REALIZAR POR EL ALUMNO ------------------------------------------------

    if(jugador_== 1){

        cout << "JUGADOR heruristica pro"<< jugador_ <<endl;

        // Opcion: Poda AlfaBeta
        // NOTA: La parametrizacion es solo orientativa
        valor = Poda_AlfaBeta(actual_, jugador_, 0, PROFUNDIDAD_ALFABETA, accion, menosinf, masinf);
        cout << "Accion " << accion << endl;
        cout << "Valor MiniMax pro: " << valor << "  Accion: " << actual_.ActionStr(accion) << endl;
    }
    else{
        cout << "JUGADOR Heuristica contar fichas"<< jugador_ <<endl;
        valor = Poda_AlfaBeta(actual_, jugador_, 0, PROFUNDIDAD_ALFABETA, accion, menosinf, masinf);
        cout << "Accion " << accion << endl;
        cout << "Valor MiniMax contar fichas: " << valor << "  Accion: " << actual_.ActionStr(accion) << endl;
    }



    return accion;
}

