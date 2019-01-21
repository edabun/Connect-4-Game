/**************************************************/
/*                    Abundabar                   */
/**************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define ROW 7
#define COL 7
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void printBoard(char *board);
void gameRules();
int Xturn(char *board,int num);
int Yturn(char *board,int num);
int checkH(char *board);
int checkV(char *board);
int checkDiag(char *board);
int diagonal(char *board,int a, int b,int c ,int d);
int checkFull(char *board);

void die_with_error(char *error_msg){
    printf("%s", error_msg);
    exit(-1);
}

int main(int argc, char *argv[]){
    char board[ROW * COL];
    memset(board,' ',ROW * COL);
    int end=0;
    int server_sock, client_sock, port_no, client_size, n;
    char buffer[256];
    struct sockaddr_in server_addr, client_addr;
    if (argc < 2) {
        printf("Usage: %s port_no", argv[0]);
        exit(1);
    }

    // Create a socket for incoming connections
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
       die_with_error("Error: socket() Failed.");

    // Bind socket to a port
    bzero((char *) &server_addr, sizeof(server_addr));
    port_no = atoi(argv[1]);
    server_addr.sin_family = AF_INET; // Internet address
    server_addr.sin_addr.s_addr = INADDR_ANY; // Any incoming interface
    server_addr.sin_port = htons(port_no); // Local port

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
       die_with_error("Error: bind() Failed.");

    // Mark the socket so it will listen for incoming connections
    listen(server_sock, 5);

    // Accept new connection
    client_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
    if (client_sock < 0) die_with_error("Error: accept() Failed.");

    // Communicate
    system("clear");
    gameRules();
    printBoard(board);
    while(1){
        //send
        printf(ANSI_COLOR_RED "\nPlayer 1 (X) \nEnter number coordinate: " ANSI_COLOR_RESET);

        while(1){
          bzero(buffer, 256);
          fgets(buffer, 255, stdin);

          if((buffer[0]-48) < 1 || (buffer[0]-48) > 7){
            system("clear");
            printBoard(board);
            printf(ANSI_COLOR_RED "\nOut of bounds.\nEnter number coordinate: " ANSI_COLOR_RESET);
            continue;
          }else{
            while(!Xturn(board, buffer[0]-48)){
              system("clear");
              printBoard(board);
              printf(ANSI_COLOR_RED "\nColumn full!\nEnter number coordinate: " ANSI_COLOR_RESET);
              fgets(buffer, 255, stdin);
            }
            n = send(client_sock, buffer, strlen(buffer), 0);
            if (n < 0) die_with_error("Error: send() Failed.");
            system("clear");
            printBoard(board);
            break;
          }

        }

        if(end!=checkV(board) || end!=checkH(board) || end!=checkDiag(board) || !checkFull(board)){
          return 0;
        }

        //recv
        bzero(buffer, 256);
        puts("\nYour opponent's turn.\n");
        n = recv(client_sock, buffer, 255, 0);
        Yturn(board,(buffer[0]-48));
        system("clear");
        printBoard(board);
        if (n < 0) die_with_error("Error: recv() Failed.");

        if(end!=checkV(board) || end!=checkH(board) || end!=checkDiag(board) || !checkFull(board)){
          return 0;
        }

    }

    close(client_sock);
    close(server_sock);

    return 0;
}

//print board
void printBoard(char *board){
  int row,col;
  printf(ANSI_COLOR_MAGENTA "\n   -----CONNECT FOUR-----\n\n");
  for(row=0;row<ROW;row++){
    for(col=0;col<COL;col++){
      if(board[COL * row + col] == 'X'){
        printf(ANSI_COLOR_MAGENTA "|");
        printf(ANSI_COLOR_RED " %c " ANSI_COLOR_RESET,board[COL * row + col]);
      }else
        if(board[COL * row + col] == 'O'){
          printf(ANSI_COLOR_MAGENTA "|");
          printf(ANSI_COLOR_BLUE" %c " ANSI_COLOR_RESET,board[COL * row + col]);
        }else
          printf(ANSI_COLOR_MAGENTA "| %c ",board[COL * row + col]);
    }
    printf(ANSI_COLOR_MAGENTA "|");
    printf("\n-----------------------------\n" ANSI_COLOR_RESET);
  }

  printf("  1   2   3   4   5   6   7\n");
}

//game rules
void gameRules(){
  printf("Objective:\nConnect four of your pieces either horizontally, vertically, or diagonally.\n\n");
  printf("GENERAL RULES:\n");
  printf("Server makes the first move. Players take turn in placing their pieces\n");
  printf("To make a move, enter a number from 1 to 7 only.\nEach piece will be automatically placed in each column.\n");
  printf("When a column is already full, place your piece on other columns.\n");
}

//player 1 turn
int Xturn(char *board,int num){

  int row;
  num--;
    for(row=ROW-1;row>=0;row--){
      if(board[COL * row + num]==' '){
        board[COL * row + num] = 'X';
        return 1;
      }
    }
  return 0;
}

//player 2
int Yturn(char *board,int num){

  int row;
  num--;

    for(row=ROW-1;row>=0;row--){
      if(board[COL * row + num]==' '){
        board[COL * row + num] = 'O';
        return 1;
      }
    }

  return 0;
}

//check horizontally
int checkH(char *board){
  int row,col,index;
  for(row=0;row<ROW;row++){
    for(col=0;col<COL-3;col++){
      index=COL*row+col;
      if(board[index]==board[index+1] && board[index+1] == board[index+1*2] && board[index+1*2] == board[index+1*3] && board[index] !='\0' && board[index]=='X'){
        printf(ANSI_COLOR_RED "\nPlayer 1 wins!\n" ANSI_COLOR_RESET);
        return 1;
      }else if(board[index]==board[index+1] && board[index+1] == board[index+1*2] && board[index+1*2] == board[index+1*3] && board[index]!='\0' && board[index]=='O'){
        printf(ANSI_COLOR_BLUE "\nPlayer 2 wins!\n" ANSI_COLOR_RESET);
        return 1;
      }
    }
  }
  return 0;
}

//check vertically
int checkV(char *board){
  int row,col,index;
  const int height=7;

  for(row=0;row<ROW-3;row++){
    for(col=0;col<COL;col++){
      index=COL*row+col;
        if(board[index] == board[index+height] && board[index+height] == board[index+height*2] && board[index+height*2] == board[index+height*3] && board[index] != '\0' && board[index]=='X'){
          printf(ANSI_COLOR_RED "\nPlayer 1 wins!\n" ANSI_COLOR_RESET);
          return 1;
        }else
        if(board[index] == board[index+height] && board[index+height] == board[index+height*2] && board[index+height*2] == board[index+height*3] && board[index] != '\0' && board[index]=='O'){
          printf(ANSI_COLOR_BLUE "\nPlayer 2 wins!\n" ANSI_COLOR_RESET);
          return 1;
        }
    }
  }
  return 0;
}

//check diagonally
int checkDiag(char *board){
  int row,col,index,ctr=0,right=6,left=8;

  for(row=0;row<ROW-3;row++){
    for(col=0;col<COL;col++){
      index=COL*row+col;
            if(ctr <=3 && (board[index] == board[index+left] && board[index+left] == board[index+left*2] && board[index+left*2] == board[index+left*3] && board[index] == 'X' && board[index]!='\0')){
              printf(ANSI_COLOR_RED "\nPlayer 1 wins!\n" ANSI_COLOR_RESET);
              return 1;
            }else
            if(ctr <=3 && (board[index] == board[index+left] && board[index+left] == board[index+left*2] && board[index+left*2] == board[index+left*3] && board[index] == '0' && board[index]!='\0')){
              printf(ANSI_COLOR_BLUE "\nPlayer 2 wins!\n" ANSI_COLOR_RESET);
              return 1;
            }else
            if(ctr >=3 && (board[index] == board[index+right] && board[index+right] == board[index+right*2] && board[index+right*2] == board[index+right*3] && board[index] == 'X' && board[index]!='\0')){
              printf(ANSI_COLOR_RED "\nPlayer 1 wins!\n" ANSI_COLOR_RESET);
              return 1;
            }
            if(ctr >=3 && (board[index] == board[index+right] && board[index+right] == board[index+right*2] && board[index+right*2] == board[index+right*3] && board[index] == '0' && board[index]!='\0')){
              printf(ANSI_COLOR_BLUE "\nPlayer 2 wins!\n" ANSI_COLOR_RESET);
              return 1;
            }
        ctr++;
      }
      ctr = 0;
    }
    return 0;
}

int checkFull(char *board){
  int row,col;
  for(row=0;row<ROW;row++){
    for(col=0;col<COL;col++){
      while(board[COL * row + col] == ' ')
        return 1;
    }
  }
  printf("\nIt's a tie!\n");
  return 0;
}
