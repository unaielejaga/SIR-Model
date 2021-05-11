#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h> 
#include <errno.h> 
#include <termios.h> 
#include <unistd.h> 
#include <string.h>

#define LAVADOMANOS 0.21
#define FFP1 0.78
#define FFP2 0.92
#define FUGA1 0.5
#define FUGA2 0.6
#define DIST50 0.6
#define DIST100 0.7
#define TRATA 0.15


/*
Estructura Array:
{%PerLavadoManos, %PerFFP1, %PerFFP2, %PerFuga1, %PerFuga2, %PerDist50, %PerDist100, %PerTrata, NumHabit, TienpoSim, I0}
*/

/*
Creamos estructura para el calculo del valor de Beta (tasa de transmisión)
*/

typedef struct Beta {
    long lavadoManos;
    long ffp1;
    long ffp2;
    long fuga1;
    long fuga2;
    long dist50;
    long dist100;
}BETA;

enum ESTADOS {COMPROBARMASCARILLA = 0, COMPROBARDISTANCIA = 1, COMPROBARLAVADO = 2, FIN = 3, ERROR = 4};

long double calculoBeta(long *buffer);
double calcularGamma(long *buffer);
double dS(double t, double S, double I, double beta, double N);
double dI(double t, double S, double I, double beta, double gamma, double N);
double dR(double t, double I, double gamma);
void calculoRK4(double *S, double *I, double *R, double dias, double gamma, double beta, double I0, double N);
void leerSerial(int serial_port, long *buffer);
void escribirSerial(double* S, double* I, double* R, int serial_port, double dias);



long double calculoBeta(long *buffer){

    /*
    Inicializamos todas las variables para su posterior calculo para que así sea más comprensible
    */

    BETA beta;
    double tasa;
    short continuar = 1;

    beta.lavadoManos = buffer[0];
    beta.ffp1 = buffer[1];
    beta.ffp2 = buffer[2];
    beta.fuga1 = buffer[3];
    beta.fuga2 = buffer[4];
    beta.dist50 = buffer[5];
    beta.dist100 = buffer[6];

    enum ESTADOS estado = COMPROBARMASCARILLA;

    do{
        switch (estado){

        case COMPROBARMASCARILLA:
            
            if((beta.ffp1 + beta.ffp2) > 100 ){ //Comprobación de que no haya más personas con mascarilla que habitantes
                printf("Es imposible que haya personas que utilicen las dos mascarillas a la vez\n");
                estado = ERROR;
                break;
            }else{
                if((beta.ffp1 + beta.ffp2) < (beta.fuga1 + beta.fuga2)){ //Comprobamos que no haya más personas con fugas en la mascarilla que personas con mascarilla
                    printf("Es imposible que haya mas personas con fugas que con mascarilla\n");
                    estado = ERROR;
                    break;
                }else{
                    
                    double sinmasc = fabs(100 - (beta.ffp1 + beta.ffp2)); //Personas sin mascarilla
                    /*
                    Ya que no conocemos el numero de personas que tiene la mascarilla FFP1 con la fuga de 1% calculamos un random del porcentaje de fuga del 1%
                    para que nos devuelva el porcentaje de personas con FFP1 que tienen fugas del 1%, si el porcentaje es mayor al porcentaje de personas que utilizan
                    la FFP1, las personas con fuga del 1% con FFP1 serán todas las personas contengan una mascarilla FFP1. El resto se le atribuirá a las personas con 
                    FFP2
                    */
                    int randfuga1 = rand() % beta.fuga1; //Calculamos random 
            
                    double porffp1fuga1; //FFP1 con fuga 1%
                    double porffp2fuga1; //FFP2 con fuga 1%

                    if(randfuga1 > beta.ffp1) //Comprobamos si es mayor el porcentaje del 1% al de FFP1
                        porffp1fuga1 = beta.ffp1;
                    else
                        porffp1fuga1 = randfuga1;

                    porffp2fuga1 = fabs(beta.fuga1 - porffp1fuga1); //Asignamos porcentaje de FFP2 con fuga del 1%

                    double ffp1rest = fabs(beta.ffp1 - porffp1fuga1); //FFP1 sin fugas
                    double ffp2rest = fabs(beta.ffp2 - porffp2fuga1); //FFP2 con fugas

                    /*
                    Hacemos lo mismo que antes pero ahora para la fuga del 2%
                    */

                    int randfuga2 = rand() % beta.fuga2;
                    double porffp1fuga2;
                    double porffp2fuga2;

                    if(randfuga2 > ffp1rest)
                        porffp1fuga2 = ffp1rest;
                    else
                        porffp1fuga2 = randfuga2;

                    porffp2fuga2 = fabs(beta.fuga2 - porffp1fuga2);

                    ffp1rest = fabs(ffp1rest - porffp1fuga2);
                    ffp2rest = fabs(ffp2rest - porffp2fuga2);

                    /*
                    Calculamos la tasa de transmisión para cada uno de los casos
                    */

                    double tasaffp1fuga1 = fabs(porffp1fuga1 - (porffp1fuga1 * (FFP1 - FUGA1)));
                    double tasaffp2fuga1 = fabs(porffp2fuga1 - (porffp2fuga1 * (FFP2 - FUGA1)));
                    double tasaffp1fuga2 = fabs(porffp1fuga2 - (porffp1fuga2 * (FFP1 - FUGA2)));
                    double tasaffp2fuga2 = fabs(porffp2fuga2 - (porffp2fuga2 * (FFP2 - FUGA2)));
                    
                    double tasaffp1 = fabs(ffp1rest - (ffp1rest * FFP1));
                    double tasaffp2 = fabs(ffp2rest - (ffp2rest * FFP2));

                    tasa = tasaffp1fuga1 + tasaffp2fuga1 + tasaffp1fuga2 + tasaffp2fuga2 + tasaffp1 + tasaffp2 + sinmasc;

                    printf("Tasa FFP1 F1: %f\nTasa FFP2 F1: %f\nTasa FFP1 F2: %f\nTasa FFP2 F2: %f\nTasa FFP1: %f\nTasa FFP2: %f\nTasa SinMasc: %f\n", tasaffp1fuga1, tasaffp2fuga1, tasaffp1fuga2, tasaffp2fuga2, tasaffp1, tasaffp2, sinmasc);

                    estado = COMPROBARDISTANCIA;
                }
               
            }

            break;

        case COMPROBARDISTANCIA:

            /*
            Calculamos la tasa de transmisión con el procentaje de personas que mantiene una distancia minima de 50cm o de 100cm
            */

            if((beta.dist50 + beta.dist100) > 100){ //Comprobamos si las personas que mantienen la distancia es mayor al numero total de habitantes
                printf("No puede haber mas personas con distanciamiento que habitantes\n");
                estado = ERROR;
                break;
            }else{
                /*
                Calculamos randomente el porcentaje de personas que mantienen la distancia minima de 50cm y 100cm que tengan mascarilla.
                Este procentaje no lo añadiremos a la tasa ya que el impacto que tendría en esta sería prácticamente nulo, por lo tanto, solo añadiremos
                las personas que mantienen la distancia pero no llevas mascarilla.
                */
                int pordist50masc = rand() %beta.dist50;
                int pordist100masc = rand() %beta.dist100;

                if(pordist50masc > ((beta.ffp1 + beta.ffp2))){ //Calculamos si el porcentaje de personas con distancia de 50cm es mayor al porcentaje de personas que llevan mascarilla
                    pordist50masc = (beta.ffp1 + beta.ffp2);
                    pordist100masc = 0;
                }else if(pordist100masc > ((beta.ffp1 + beta.ffp2))){ //Calculamos si el porcentaje de personas con distancia de 100cm es mayor al porcentaje de personas que llevan mascarilla
                    pordist100masc = (beta.ffp1 + beta.ffp2);
                    pordist50masc = 0;
                }
                
                /*
                Calculamos la tasa de transmisión para cada uno de los casos
                */

                double tasasindist = fabs(100 - (beta.dist50 + beta.dist100));
                double tasadist50 = (beta.dist50 - pordist50masc) - ((beta.dist50 - pordist50masc) * DIST50);
                double tasadist100 = (beta.dist100 - pordist100masc)- ((beta.dist100 - pordist100masc) * DIST100);

                /*
                En este caso la tasa se resta ya que el echo de que las personas que mantengan la distancia de seuridad hace que la tasa de transmisión se reduzca
                */

                tasa = tasa - tasadist50 - tasadist100;

                printf("Tasa Dist50: %f\nTasa Dist100: %f\nTasa SinDist: %f\n", tasadist50, tasadist100, tasasindist);
            }

            estado = COMPROBARLAVADO;
            
            break;

        case COMPROBARLAVADO:

            /*
            Calculamos la tasa de transmisión con el porcentaje de personas que se lavan adecuadamente las manos
            */

            if(beta.lavadoManos > 100) { //Comprobamos si el numero de personas que se lavan las manos es mayor al numero de habitantes

                printf("Es imposible que haya mas personas que se laven las manos que habiantes\n");
                estado = ERROR;
                break;

            }else{
                /*
                Calculamos randomente el porcentaje de personas que se lavan las manos adecuadamente y que además estén protejidos con mascarilla
                ya que el impacto que tendrán en la tasa será prácticamente nulo
                */
                int porlavadoprotegido = rand() %beta.lavadoManos;

                if(porlavadoprotegido > (beta.ffp1 + beta.ffp2)){ //Comprobamos si el porcentaje de personas que se lavan adecuadamente las manos es mayor que el porcentaje de persoasn que utilizan la mascarilla
                    porlavadoprotegido = (beta.ffp1 + beta.ffp2);
                }
                
                /*
                Calculamos la tasa de transmisión para este caso
                */

                double porlavado = fabs(beta.lavadoManos - porlavadoprotegido);
                double tasalavado = porlavado - (porlavado * LAVADOMANOS);

                tasa = tasa - tasalavado;

                printf("Tasa LavadoManos: %f\n", tasalavado);
            }
            

            estado = FIN;
            
            break;

        case FIN:

            /*
            Si la tasa de transmisión llegase a ser menor del 2%, no sería real, por eso si la tasa es menor al 2% se mantendrá en ese 2%. Ya que es imposible
            estar protegido al 100% del virus, siempre va a haber algún error (en este caso como máximo hasta el 98%)
            */

            if(tasa/100 < 0.02)
                tasa = 2;

            printf("Todo ha ido perfecto\n");
            continuar = 0;

            break;
        
        case ERROR:

            /*
            Comprobamos si ha habido algun error en los paramentros pasados
            */

            printf("Ha habido un error en algún proceso\n");
            continuar = 0;
            tasa = 0; //Ponemos la tasa a 0, ya que ha habido un error en alguna parte
            break;
        
        default:

            /*
            Comprobamos si ha habido algún estado no contemplado
            */

            printf("No se ha encontrado el siguiente estado\n");
            continuar = 0;
            tasa = 0; //Ponemos la tasa a 0, ya que ha habido un error en alguna parte
            break;

        }
    } while (continuar);
    
    return tasa/100;
}

/*
Calculamos la tasa de recuperación del virus, ya que el periodo medio de recuperación de la COVID-19 es de 14 dias, 1/gamma = 14.
Además calcularemos la tasa con los avances de tratamientos contra el virus ya que estos influyen en la tasa de recuperación
*/

double calcularGamma(long *buffer){
    double trata = buffer[7];

    return((1/14)+((trata*TRATA)/100)); //Calculamos la tasa de recuperación
}

/*
Calculo de la ecuación diferencial de S (Susceptibles)
*/

double dS(double t, double S, double I, double beta, double N){
    return((-beta*S*I)/N);
}

/*
Calculo de la ecuación diferencial de I (Infecatdos)
*/

double dI(double t, double S, double I, double beta, double gamma, double N){
    return(((beta*S*I)/N) - (I*gamma));
}

/*
Calculo de la ecuación diferencial de R (Recuperados/Muertos)
*/

double dR(double t, double I, double gamma){
    return(gamma*I);
}

/*
Para el cálculo de estas tres ecuaciones diferenciales utilizaremos el algoritmo de Runge-Kutta de cuarto orden, que se utiliza para la resolución 
de ecucaciones diferenciales. Para el calculo de esta algoritmia necesitaremos 4 ecuaciones que se mostrarán posteriromente. La primera ecuación k1 
determina la pendiente al principio del intervalo, k2 es la pendiente en el punto medio del intervalo usando k1, k3 es también la pendiente en el 
punto medio del intervalo usando k2 y por último, k4 es la pendiente al final del intervalo. El resultado de las ecuaciones se hará promediando las
cuatro pendientes. Tendremos que usar estas cuatro eccuaciones para cada una de las ecuaciones diferenciales; para S, I y R
*/

void calculoRK4(double *S, double *I, double *R, double dias, double gamma, double beta, double I0, double N){

    double R0 = 0; //El numero de Recuperados al principio será de 0
    double S0 = N - I0; //El numero de Susceptibles al rpincipio será la resta entre el numero total de habitantes menos el número inicial de Infectados
    double t = 1; //Calcularemos los resultados cada día

    /*
    Inicializamos las variables con los valores inciales de la epidemia
    */

    S[0] = S0; 
    I[0] = I0;
    R[0] = R0;

    double Si = 0;
    double Ii = 0;
    double Ri = 0;

    double Sk1 = 0;
    double Sk2 = 0;
    double Sk3 = 0;
    double Sk4 = 0;

    double Ik1 = 0;
    double Ik2 = 0;
    double Ik3 = 0;
    double Ik4 = 0;

    double Rk1 = 0;
    double Rk2 = 0;
    double Rk3 = 0;
    double Rk4 = 0;
    
    for(int i = 0; i < dias-1; i++){

        Si = S[i];
        Ii = I[i];
        Ri = R[i];

        /*
        Calculo de k1
        */

        Sk1 = dS(i, Si, Ii, beta, N);
        Ik1 = dI(i, Si, Ii, beta, gamma, N);
        Rk1 = dR(i, Ii, gamma);

        /*
        Calculo de k2
        */

        Sk2 = dS((i + t/2), (Si + (Sk1*t)/2), (Ii + (Ik1*t)/2), beta, N);
        Ik2 = dI((i + t/2), (Si + (Sk1*t)/2), (Ii + (Ik1*t)/2), beta, gamma, N);
        Rk2 = dR((i + t/2), (Ii + (Ik1*t)/2), gamma);

        /*
        Calculo de k3
        */

        Sk3 = dS((i + t/2), (Si + (Sk2*t)/2), (Ii + (Ik2*t)/2), beta, N);
        Ik3 = dI((i + t/2), (Si + (Sk2*t)/2), (Ii + (Ik2*t)/2), beta, gamma, N);
        Rk3 = dR((i + t/2), (Ii + (Ik2*t)/2), gamma);

        /*
        Calculo de k4
        */

        Sk4 = dS((i + t), (Si + Sk3*t), (Ii + Ik3*t), beta, N);
        Ik4 = dI((i + t), (Si + Sk3*t), (Ii + Ik3*t), beta, gamma, N);
        Rk4 = dR((i + t), (Ii + Ik3*t), gamma);

        /*
        Calculo promediando las cuatro pendientes (resultados)
        */

        S[i+1] = Si + t*(Sk1 + 2*Sk2 + 2*Sk3 + Sk4)/6;
        I[i+1] = Ii + t*(Ik1 + 2*Ik2 + 2*Ik3 + Ik4)/6;
        R[i+1] = Ri + t*(Rk1 + 2*Rk2 + 2*Rk3 + Rk4)/6;

    }

}

void leerSerial(int serial_port, long *buffer){

printf("Esperando Recivir datos\n");
   char read_buf [256]; //Creamos un buffer de lectura


  memset(&read_buf, '\0', sizeof(read_buf)); //Lo inicializamos a vacio

  int num_bytes = read(serial_port, &read_buf, sizeof(read_buf)); //Sacamos el numero de bytes leidos del serial
  
  if (num_bytes < 0) { //Si el numero de bytes es menor que 0 entonces hay un error ya que no se ha podido leer nada
      printf("Error reading: %s", strerror(errno));
  }

  const char sep[2] = ","; //Creamos un char de sepracion ya que los datos vendrán separados por comas
  char *token;
    /*
    Dividimos el string obtenido por comas y metemos cada dato en el buffer
    */
  token = strtok(read_buf, sep);

  long valor = 0;
  int pos = 0;

  while(valor != -1){ //Si el valor es -1 entonces es el final de la lectura
      valor = atol(token);
      buffer[pos] = valor;
      pos++;
      token = strtok(NULL, sep);
  }

}

void escribirSerial(double* S, double* I, double* R, int serial_port, double dias){

    /*
    Escribimos en el puerto serial todo los datos calculados con un separador "," de por medio
    */

    char separador[1] = ",";

    for(int i=0; i < dias-1; i++){ //Escribimos los calculos de S
        char array[256];
        sprintf(array,"%f", S[i]);
        write(serial_port, array, sizeof(double));
        write(serial_port, separador, sizeof(separador));
    }

    char s[2] = "-1"; //Separamos con un -1 para finalizar
    write(serial_port, s, sizeof(s));
    write(serial_port, separador, sizeof(separador));

    for(int i=0; i < dias-1; i++){ //Escribimos los calculos de I
        char array[256];
        sprintf(array,"%f", I[i]);
        write(serial_port, array, sizeof(double));
        write(serial_port, separador, sizeof(separador));
    }

    char i[2] = "-2"; //Separamos con un -2 para finalizar
    write(serial_port, i, sizeof(s));
    write(serial_port, separador, sizeof(separador));

    for(int i=0; i < dias-1; i++){ //Escribimos los calculos de R
        char array[256];
        sprintf(array,"%f", R[i]);
        write(serial_port, array, sizeof(double));
        write(serial_port, separador, sizeof(separador));
    }

    char r[2] = "-3"; //Escribimos un -3 para finalizar
    write(serial_port, r, sizeof(s));
}



int main(){ 

    /*
    Confugracion de los diferentes flags para el correcto funcionamiento de la comunicacion serial en Linux 
    */

    int serial_port = open("/dev/ttyUSB0", O_RDWR); //Definumos cual es el puerto serial y lo abrimos

    
    struct termios tty;

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    tty.c_oflag &= ~OPOST; 
    tty.c_oflag &= ~ONLCR; 


    tty.c_cc[VTIME] = 232;    
    tty.c_cc[VMIN] = 100; 
  
    /*
    Configuracion del baundrate de la comucacion serial
    */

    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);
    
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i de tcsetattr: %s\n", errno, strerror(errno));
    }

    /*
    Comienzo del programa principal
    */

    long buffer[12]; //Creamos buffer vacio para la lectura de datos de la conexion serial

    leerSerial(serial_port, buffer); //Leemos los datos

    /*
    Calculo de los coeficientes beta, gamma y ecuaciones diferneciales para el modelo SIR
    */

    double beta = calculoBeta(buffer);
    double gamma = calcularGamma(buffer);
    printf("Beta: %f\n", beta);
    printf("Gamma: %f\n", gamma);
    double S[buffer[9]];  double I[buffer[9]];  double R[buffer[9]];
    calculoRK4(S, I, R, buffer[9], gamma, beta, buffer[10], buffer[8]);

    sleep(1); //Hacemos un sleep para separar la zona de lectura y de escritura

    escribirSerial(S, I, R, serial_port, buffer[9]); //Escribimos en el puerto serial los datos obtenidos de los calculos


    return 0; //Acaba el programa
}