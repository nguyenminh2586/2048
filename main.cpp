#include<iostream>
#include<iomanip>
#include<ctime>
#include<cstdlib>
using namespace std;

int board[4][4];
int dirLine[]={1,0,-1,0};// direction mapping
int dirColumn[]={0,1,0,-1};// direction mapping

pair<int,int> generteUnoccupiedPosition()//generate random line
{
    int occupied=1,line,column;
    while(occupied)
    {
        line=rand()%4;
        column=rand()%4;
        if(board[line][column]==0)
            occupied=0;
    }
    return make_pair(line,column);
}

void addPiece()
{
    pair<int,int> pos=generteUnoccupiedPosition();
    board[pos.first][pos.second]=2;
}

void newGame()
{
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            board[i][j]=0;
    addPiece();
}

void printUI()
{
    system("cls");
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
            if(board[i][j]==0)
                cout<<setw(6)<<".";
            else
                cout<<setw(6)<<board[i][j];
        cout<<'\n';
    }
    cout<<"n: new game,  q: quit,  w: up,  s: down,  a: left,  d: right\n";
}

bool canDoMove(int line,int column,int nextLine,int nextColumn)//combine
{
    if(nextLine<0 || nextColumn<0 || nextLine>=4 || nextColumn>=4
       || board[line][column]!=board[nextLine][nextColumn] && board[nextLine][nextColumn]!=0)
        return false;
    return true;
}

void applyMove(int direction)//moving the pieces,linestep
{
    int startLine = 0, startColumn = 0, lineStep = 1, columnStep = 1;
    if (direction == 0) {
        startLine = 3;
        lineStep = -1;
    }
    if (direction == 1) {
        startColumn = 3;
        columnStep = -1;
    }
    int movePossible, canAddPiece = 0;
    do {
        movePossible = 0;
        for (int i = startLine; i >= 0 && i < 4; i += lineStep)
            for (int j = startColumn; j >= 0 && j < 4; j += columnStep) {
                int nextI = i + dirLine[direction];
                int nextJ = j + dirColumn[direction];
                if (board[i][j] && canDoMove(i, j, nextI, nextJ)) {
                    if (board[nextI][nextJ] == 0) {
                        board[nextI][nextJ] = board[i][j];
                        board[i][j] = 0;
                        movePossible = 1;
                        canAddPiece = 1; // Đánh dấu có di chuyển hợp lệ
                    } else if (board[nextI][nextJ] == board[i][j]) {
                        board[nextI][nextJ] += board[i][j];
                        board[i][j] = 0;
                        movePossible = 1;
                        canAddPiece = 1; // Đánh dấu có hợp nhất
                    }
                }
            }
        printUI();
    } while (movePossible);
    if (canAddPiece)
        addPiece(); // Sinh số ngẫu nhiên nếu có di chuyển hợp lệ
}

bool isGameOver()
{
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            if(board[i][j]==0)
                return false;
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            for(int dir=0;dir<4;dir++)
            {
                int nextI=i+dirLine[dir];
                int nextJ=j+dirColumn[dir];
                if(nextI>0 && nextJ>0 && nextI<4 && nextJ<4)
                {
                    if(board[i][j]==board[nextI][nextJ])
                        return false;
                }
            }
    return true;
}

bool hasWon()
{
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            if(board[i][j]==2048)
                return true;
    return false;
}

int main()
{
    srand(time(0));
    char commandToDir[256];
    commandToDir['s']=0;//direction
    commandToDir['d']=1;
    commandToDir['w']=2;
    commandToDir['a']=3;
    newGame();
    while(true)
    {
        printUI();
        if(isGameOver())
        {
            cout<<"GAME OVER!";
            break;
        }
        if(hasWon())
        {
            cout<<"YOU WIN!";
            break;
        }
        char command;
        cin>>command;
        if(command=='n')
            newGame();
        else
            if(command=='q')
                break;
            else
            {
                int currentDirection=commandToDir[command];
                applyMove(currentDirection);
            }

    }
    return 0;
}
