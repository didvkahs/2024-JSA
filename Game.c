#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h> // equivalent to window.h file
    #include <termios.h> // to use getch and kbhit
    #include <sys/select.h>
   
   
    // equivalent function to coino.h kbhit and getch;
    int LinuxKbhit(void)
    {
        struct termios old, new;
        int ch;
       
        tcgetattr(STDIN_FILENO, &old);
        new = old;
       
        new.c_lflag &= ~(ICANON|ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new);
       
        ch = getchar();
       
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
       
        return ch;
    }
   
    int LinuxGetch(void)
    {
        int ch;
        struct termios buf, save;
        tcgetattr(0, &save);
        buf = save;
        buf.c_lflag &= ~(ICANON | ECHO);
        buf.c_cc[VMIN] = 1;
        buf.c_cc[VTIME] = 0;
        tcsetattr(0, TCSAFLUSH, &buf);
        ch = getchar();
        tcsetattr(0, TCSAFLUSH, &save);
        return ch;
    }
#endif

//macro function
#define NEXT(node) ((node)->Link[(node)->Heading])
#define PREV(node) ((node)->Link[!(node)->Heading])
#define CENTI_VERDICT(centi, x, y) (Map[(centi)->X + x + 100 *((centi)->Y + y)] == ' ' || Map[(centi)->X + x + 100 *((centi)->Y + y)] == 'W')
#define USER_VERDICT(user, x, y) (Map[(user)->X + x + 100 *((user)->Y + y)] == ' ' || Map[(user)->X + x + 100 *((user)->Y + y)] == '@')
#define INDEX(x, y) (Map[x + 100 * y])

//constant
#define MAPSIZE 5500 // 100 * 55
#define MAPHORIZONTAL 100
#define OBSTACLE 150
#define REMAININGS 10
#define MOBSIZE 20
#define BLOOD 4
#define SPEED 90
#define COORDX 50
#define COORDY 0

//Types
typedef enum
{
    BOOL_FALSE,
    BOOL_TRUE
}Bool_t;

typedef enum
{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT = -1
}Dir_t;

typedef struct Player_s
{
    char Blood;
    char Remainings;
    int X;
    int Y;
}Player_t, * pPlayer_t;

typedef struct Monster_s
{
    int X;
    int Y;
    Bool_t Row; // right direction true
    Bool_t Col; // down direction true
    Bool_t Heading;
    struct Monster_s* Link[2];
}Monster_t, * pMonster_t;

typedef struct MonsterInfo_s
{
    pMonster_t Head;
    pMonster_t Tail;
    struct MonsterInfo_s* Next;
}MonsterInfo_t, * pMonsterInfo_t;

typedef struct Bullet_s
{
    int X;
    int Y;
    Bool_t Load;
    struct Bullet_s* Next;
}Bullet_t, * pBullet_t;

//Global Variables
pMonsterInfo_t MonsterInfoList = NULL;
char Map[MAPSIZE];

//Fuctions
void GameSetUp(Player_t**, Monster_t**, Bullet_t**);
void MovMonster(void);
void SplitMonster(int);
void ShootBullet(Bullet_t**);
void MovPlayer(Player_t**, Bullet_t**);
void FreeMonster(void);

//main
int main(void)
{
    pPlayer_t user1 = NULL;
    pBullet_t bullet = NULL;
    pMonster_t centipede = NULL;

    GameSetUp(&user1, &centipede, &bullet);
   
    #ifdef _WIN32
        const COORD pos = {0,0};
        const int speed = 90;
       
        for(;;)
        {
           SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
           printf("%s", Map);
           MovMonster();
           //MovPlayer(&user1, &bullet);
           //ShootBullet(&bullet);
           Sleep(speed);
        }
    #else
        const float speed = 0.7;

        for(;;)
        {
            system("clear");
            printf("%s", Map);
            MovMonster();
            //MovPlayer(&user1, &bullet);
            //ShootBullet(&bullet);
            sleep(speed);
        }
    #endif


    FreeMonster();
    free(centipede);
    return 0;
}

void GameSetUp(Player_t** user, Monster_t** mob, Bullet_t** arm)
{
    *user = malloc(sizeof(Player_t));
    assert(*user != NULL);
    *mob = malloc(sizeof(Monster_t) * MOBSIZE);
    assert(*mob != NULL);
    *arm = malloc(sizeof(Bullet_t) * REMAININGS);
    assert(*user != NULL);
    MonsterInfoList = malloc(sizeof(MonsterInfo_t));
    assert(MonsterInfoList != NULL);

    // user setting  
    (*user)->Blood = BLOOD;
    (*user)->Remainings = REMAININGS;
    (*user)->X = COORDX;
    (*user)->Y = 53; // player coord y position

    // mob setting
    for(int i = 0; i < MOBSIZE; ++i)
    {
        (*mob + i)->X = COORDX;
        (*mob + i)->Y = COORDY;
        (*mob + i)->Row = BOOL_TRUE;
        (*mob + i)->Col = BOOL_TRUE;
        (*mob + i)->Heading = BOOL_TRUE;
        NEXT(*mob + i) = *mob + i + 1;
        PREV(*mob + i) = *mob + i - 1;
    }
    PREV(*mob) = NEXT(*mob + MOBSIZE - 1) = NULL;

    //infolist setting
    MonsterInfoList->Head = *mob;
    MonsterInfoList->Tail = *mob + MOBSIZE - 1;
    MonsterInfoList->Next = NULL;

    //bullet setting
    for (int i = 0; i < REMAININGS - 1; ++i)
    {
        (*arm + i)->Load = BOOL_TRUE;
        (*arm + i)->Next = *arm + i + 1;
    }
    (*arm + REMAININGS - 1)->Load = BOOL_TRUE;
    (*arm + REMAININGS - 1)->Next = *arm;

    // Map setting
    Map[MAPSIZE - 1] = '\0';

    for (int i = 0; i < MAPSIZE - 1; ++i)
    {
        Map[i] = ' ';
    }

    for (int i = 0; i < 54; ++i)
    {
        Map[99 + 100 * i] = '\n';
    }

    for (int i = 0; i < OBSTACLE; ++i)
    {
        for (;;)
        {
            int x = rand() % 50;
            int y = rand() % 22;

            if (INDEX(x * 2, y * 2) == ' ')
            {
                INDEX(x * 2, y * 2) = 'M';
                break;
            }
        }
    }
}

void FreeMonster(void)
{
    pMonsterInfo_t infoNode = NULL;
   
    assert(MonsterInfoList != NULL);
    while (MonsterInfoList != NULL)
    {
        infoNode = MonsterInfoList->Next;
        free(MonsterInfoList);
        MonsterInfoList = infoNode;
    }
}

void MovPlayer(Player_t** user, Bullet_t** arm)
{
    unsigned char key;
   
   assert(*user != NULL);
   assert(*arm != NULL);
   
    INDEX((*user)->X, (*user)->Y) = ' ';
   
    #ifdef _WIN32
    if(_kbhit())
    {
        key = _getch();
    }
    #else
    if(LinuxKbhit())
    {
        key = LinuxGetch();
    }
    #endif
   
     if(key == 224)
    {
        switch(key)
        {
            case 72:/*up*/
                if(USER_VERDICT(*user, 0, -1))
                {
                    (*user)->Y = (*user)->Y - 1;
                }
                break;
            case 80:/*down*/
                if(USER_VERDICT(*user, 0, 1))
                {
                    (*user)->Y = (*user)->Y + 1;
                }
                break;
            case 75:/*left*/
                if(USER_VERDICT(*user, -1, 0))
                {
                    (*user)->X = (*user)->X - 1;
                }
                break;
            case 77:/*right*/
                if(USER_VERDICT(*user, 1, 0))
                {
                        (*user)->X = (*user)->X + 1;
                }
                break;
        }
        INDEX((*user)->X, (*user)->Y) = 'W';
    }
     else if (key | 32 == 's')
	{
	    (*arm)->X = (*user)->X;
	    (*arm)->Y = (*user)->Y;
	}
}

void MovMonster(void)
{
    pMonsterInfo_t monsterInfo = MonsterInfoList;
    assert(monsterInfo != NULL);
    pMonster_t monster = monsterInfo->Tail;
    assert(monster != NULL);

    while (monsterInfo != NULL)
    {
        INDEX(monster->X, monster->Y) = ' ';

        // prev node follow head node
        while (PREV(monster) != NULL)
        {
            pMonster_t prev = PREV(monster);
            monster->X = prev->X;
            monster->Y = prev->Y;
            monster->Row = prev->Row;
            monster->Col = prev->Col;
            monster = prev;
        }

        // head node movement
        if (monster->Row && CENTI_VERDICT(monster, (-1), 0))
        {
            //left
            assert(CENTI_VERDICT(monster, (-1), 0) == BOOL_FALSE);
            monster->X = monster->X - 1;
        }
        else if(monster->Row == BOOL_FALSE && CENTI_VERDICT(monster, 1, 0))
        {
            //right
            assert(CENTI_VERDICT(monster, 1, 0) == BOOL_FALSE);
            monster->X = monster->X + 1;
        }
        else if (monster->Col && CENTI_VERDICT(monster, 0, 1))
        {
            //down
            assert(CENTI_VERDICT(monster, 0, 1) == BOOL_FALSE);
            monster->Row = !(monster->Row);
            monster->Y = monster->Y + 1;
        }
        else if (monster->Col == BOOL_FALSE && CENTI_VERDICT(monster, 0, (-1)))
        {
            //up
            assert(CENTI_VERDICT(monster, 0, (-1)) == BOOL_FALSE);
            monster->Row = !(monster->Row);
            monster->Y = monster->Y - 1;
        }
        else if (monster->Col && CENTI_VERDICT(monster, 0, (-1)))
        {
            //if down is not possible go up
            assert(CENTI_VERDICT(monster, 0, (-1)) == BOOL_FALSE);
            monster->Col = !(monster->Col);
            monster->Row = !(monster->Row);
            monster->Y = monster->Y - 1;
        }
        else if (monster->Col == BOOL_FALSE && CENTI_VERDICT(monster, 0, 1))
        {
            //if up is not possible go down
            assert(CENTI_VERDICT(monster, 0, 1) == BOOL_FALSE);
            monster->Col = !(monster->Col);
            monster->Row = !(monster->Row);
            monster->Y = monster->Y + 1;
        }
        else
        {
            monster->X = monster->Row ? monster->X + 1 : monster->X - 1;
            monster->Row = !(monster->Row);
        }

        INDEX(monster->X, monster->Y) = '@';
        monsterInfo = monsterInfo->Next;
    }
}

void SplitMonster(int index)
{
    pMonsterInfo_t mobList = MonsterInfoList;
    assert(mobList != NULL);
    pMonsterInfo_t newInfoNode = malloc(sizeof(MonsterInfo_t));
    assert(newInfoNode != NULL);
    
    while(mobList != NULL)
    {
        pMonster_t mobNode = mobList->Head;
        assert(mobNode != NULL);
        
        if(mobNode->X + MAPHORIZONTAL * mobNode->Y == index)
        {
            pMonster_t addressTemp = mobNode;

            while(mobNode!= NULL)
            {
                INDEX(mobNode->X, mobNode->Y) = 'M';
                mobNode = NEXT(mobNode);
            }
            PREV(NEXT(addressTemp)) = NULL;
            NEXT(PREV(addressTemp)) = NULL;
            mobList->Head = NULL;
            mobList->Tail = NULL;
            free(newInfoNode);
        }
        else
        {
            mobNode = NEXT(mobNode);
            
            while(mobNode!= NULL)
            {
                if(mobNode->X + MAPHORIZONTAL * mobNode->Y == index)
                {
                    newInfoNode->Head = mobList->Tail;
                    newInfoNode->Tail = NEXT(mobNode);
                    mobList->Tail = PREV(mobNode);
                    NEXT(PREV(mobNode)) = NULL;
                    PREV(NEXT(mobNode)) = NULL;

                    newInfoNode->Next = mobList;
                    mobList = newInfoNode;

                    break;
                }
                
                mobNode = NEXT(mobNode);
            }
        }
        mobList = mobList->Next;
    }
}

void ShootBullet(Bullet_t** arm)
{
    for (int i = 0; i < REMAININGS; ++i)
    {
        if ((*arm)->Load == BOOL_FALSE)
        {
            for (int j = 1; j < 5; ++j)
            {
                switch (INDEX((*arm)->X, (*arm)->Y + j))
                {
                case 'M':
                    INDEX((*arm)->X, (*arm)->Y + j) = 'm';
                    (*arm)->Load = BOOL_TRUE;
                    break;
                case 'm':
                    INDEX((*arm)->X, (*arm)->Y + j) = 'n';
                    (*arm)->Load = BOOL_TRUE;
                    break;
                case 'n':
                    INDEX((*arm)->X, (*arm)->Y + j) = ' ';
                    (*arm)->Load = BOOL_TRUE;
                    break;
                case '@':
                    (*arm)->Load = BOOL_TRUE;
                    SplitMonster((*arm)->X + MAPHORIZONTAL * ((*arm)->Y + j));
                    break;
                default:
                    INDEX((*arm)->X, (*arm)->Y) = ' ';
                    (*arm)->Y = (*arm)->Y + j;
                    INDEX((*arm)->X, (*arm)->Y) = '!';
                }
            }
        }
        else
        {
            continue;
        }
    }
}
