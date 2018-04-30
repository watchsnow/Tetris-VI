//********************************************************************
// File:        Tetris.cpp
// Created:     2018/04/22 14:18
// FileBase:    Tetris
// FileExt:     cpp
// Author:      Halys
// Purpose:     ----[Framework, data type declaration]
// Version:     V 2.1.5
// Email:       wenht817@gmail.com
// History:     22:4:2018 14:18 by
// Changed:     2018/04/22 14:18
//*********************************************************************

/*==============================================*
 *          Include header files                *
 *----------------------------------------------*/
#include <Windows.h>
#include <time.h>

/*==============================================*
 *               Linked library                 *
 *----------------------------------------------*/
#pragma comment(lib,"winmm.lib")

/*==============================================*
 *              方块细胞之间的参数              *
 *----------------------------------------------*/
#define CELLS_WIDTH     (30)
#define CELLS_DISTANCE  (1)

/*==============================================*
 *                  方块种类                    *
 *----------------------------------------------*/
#define BLOCK_KINDS     (7)
#define BLOCK_ROTATES   (4)
#define BLOCK_WIDTH     (4)     // 方块的边长4
#define BLOCK_SIZE      (16)    // 方块占用的矩阵 4*4 BLOCK_WIDTH * BLOCK_WIDTH

/*==============================================*
 *            消除多少行的分数奖励              *
 *----------------------------------------------*/
static const int ScoreSets[BLOCK_WIDTH] = { 100, 400, 800, 1600 };

/*========================================================*
 * 容纳方块的空间, 逻辑坐标,从左到右width, 从上到下height *
 *--------------------------------------------------------*/
#define CONTAINER_WIDTH     (10)
#define CONTAINER_HEIGHT    (20)

/*==============================================*
 *             设置容器的矩阵坐标               *
 *----------------------------------------------*/
RECT    ContainerRect = { CELLS_DISTANCE,

                          CELLS_DISTANCE,

                          CELLS_DISTANCE + CELLS_DISTANCE +
                          CONTAINER_WIDTH*(CELLS_WIDTH + 1) +
                          CELLS_DISTANCE + CELLS_DISTANCE,

                          CELLS_DISTANCE + CELLS_DISTANCE +
                          CONTAINER_HEIGHT*(CELLS_WIDTH + 1) +
                          CELLS_DISTANCE + CELLS_DISTANCE
                        };

/*==============================================*
 *       设置下一个方块容器的矩阵坐标           *
 *----------------------------------------------*/
RECT    NextBlockRect = { CELLS_DISTANCE + CELLS_DISTANCE +
                          CONTAINER_WIDTH*(CELLS_WIDTH + 1) +
                          CELLS_DISTANCE + CELLS_DISTANCE +
                          CELLS_DISTANCE + CELLS_DISTANCE,

                          CELLS_DISTANCE,

                          CELLS_DISTANCE + CELLS_DISTANCE +
                          CONTAINER_WIDTH*(CELLS_WIDTH + 1) +
                          CELLS_DISTANCE + CELLS_DISTANCE +
                          CELLS_DISTANCE + CELLS_DISTANCE +
                          CELLS_DISTANCE + BLOCK_WIDTH*
                          (CELLS_WIDTH + 1) + CELLS_DISTANCE +
                          CELLS_DISTANCE,

                          CELLS_DISTANCE + CELLS_DISTANCE +
                          BLOCK_WIDTH *(CELLS_WIDTH + 1) +
                          CELLS_DISTANCE + CELLS_DISTANCE
                        };

/*==============================================*
 *              调整一下窗口的大小             *
 *----------------------------------------------*/
// 窗口宽度 = (方块的)空隙 + 容器的一个左边界(就是那一竖) + 方块的宽度 CONTAINER_WIDTH*(空隙+CELLS_WIDTH) + 空隙 + 窗口右方边界+ 空隙就是一竖(|)+下一个容器的左边界
// 在加上方块的宽度(BLOCK_WIDTH)*(空隙+CELLS_WIDTH)+(就是下一个)空隙+ 下一个容器的右边界+ 空隙+ 窗口的两边占用的部分
#define WINDOWS_WIDTH   (CELLS_DISTANCE + CELLS_DISTANCE + CONTAINER_WIDTH  * (CELLS_WIDTH + 1) + CELLS_DISTANCE + CELLS_DISTANCE + CELLS_DISTANCE + CELLS_DISTANCE + BLOCK_WIDTH * (CELLS_WIDTH + 1) + CELLS_DISTANCE + CELLS_DISTANCE + CELLS_DISTANCE+  7)   // 7 为窗口边所占用的
#define WINDOWS_HEIGHT  (CELLS_DISTANCE + CELLS_DISTANCE + CONTAINER_HEIGHT * (CELLS_WIDTH + 1) + CELLS_DISTANCE + CELLS_DISTANCE + CELLS_DISTANCE + 33) // 33 标题栏的高度

/*==============================================*
 *            设置俄罗斯方块的类型              *
 *----------------------------------------------*/

#pragma region 设置俄罗斯方块的类型
typedef struct tagTETRIS
{
    unsigned int timerid;       // 时间
    unsigned int GameState;     // 游戏状态: 0 游戏运行状态, 1 游戏开始, 2 游戏暂停
    unsigned int InitialSpeed;   // 初始化游戏速度
    unsigned int OriginalSpeed; // 游戏本身的速度
    unsigned int Score;         // 游戏的分数

    // 下一个方块
    const unsigned char *next_block;
    unsigned int next_block_kind;           // 下一个方块的类型
    unsigned int next_block_rotate;         // 下一个方块的旋转

    // 当前方块
    const unsigned char *current_block;
    unsigned int current_block_kind;        // 当前方块的类型
    unsigned int current_block_rotate;      // 当前方块的旋转
    // 当前方块的坐标 (直线落下的坐标) 就是方块掉下来的坐标
    int offset_left;                    // 逻辑坐标 left
    int offset_top;                     // 这个是逻辑坐标 top
    int offset_top_destination;         // 用来表示当前方块直线下落的位置

    // 定义左边 右边 顶部句柄左上角 右下角
    int min_left;
    int max_right;
    int min_top;
    int max_bottom;
} TETRIS;
#pragma endregion


// 初始化俄罗斯方块 申请一个数据类型的变量
TETRIS Tetris = { 0 };  // 定义一个变量存放俄罗斯方块

// 定义俄罗斯方块的棋盘 是怎么样的
static unsigned char Container[CONTAINER_HEIGHT][CONTAINER_WIDTH] = { 0 };

/*
行列式
0   1   2   3   4   5   6   7   8   9   x CONTAINER_WIDTH
00  □ □ □ □ □ □ □ □ □
01  □ □ □ □ □ □ □ □ □
02  □ □ □ □ □ □ □ □ □
03  □ □ □ □ □ □ □ □ □
04  □ □ □ □ □ □ □ □ □
05  □ □ □ □ □ □ □ □ □
06  □ □ □ □ □ □ □ □ □
07  □ □ □ □ □ □ □ □ □
08  □ □ □ □ □ □ □ □ □
09  □ □ □ □ □ □ □ □ □
10  □ □ □ □ □ □ □ □ □
11  □ □ □ □ □ □ □ □ □
12  □ □ □ □ □ □ □ □ □
13  □ □ □ □ □ □ □ □ □
14  □ □ □ □ □ □ □ □ □
15  □ □ □ □ □ □ □ □ □
16  □ □ □ □ □ □ □ □ □
17  □ □ □ □ □ □ □ □ □
18  □ □ □ □ □ □ □ □ □
19  □ □ □ □ □ □ □ □ □
y CONTAINER_HEIGHT
*/

// 设置方块的种类
static const  unsigned char BlockSets[BLOCK_KINDS * BLOCK_ROTATES * BLOCK_SIZE] =
{
    /*
    把所有的方块的集合( I J L O S T Z ), 全部搞到这儿来 每个方块一次顺时针旋转 旋转4次, 每个4*4矩阵记录一个形状
    为了保证底部所旋转的不会导致这个方块上升一格,会往上挪一格, 从而就可以继续下降 旋转规则 保持底部水平
    □
    □     □     □
    □     □     □     □□    □□        □□      □
    □   □□        □□  □□  □□        □□    □□□
    长条，方块，左沟，右沟，凸块，左拐子，右拐子
    ┃       █   ┛       ┗
    』┗ ┛┳
    I：一次最多消除四层
    J（左右）：最多消除三层，或消除二层
    L：最多消除三层，或消除二层
    O：消除一至二层
    S（左右）：最多二层，容易造成孔洞
    Z （左右）：最多二层，容易造成孔洞
    T：最多二层

    初始矩阵
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    */
    #pragma region 绘制第一个 I
    /*
    □█□□
    □█□□
    □█□□
    □█□□
    */
    0, 1, 0, 0,
    0, 1, 0, 0,
    0, 1, 0, 0,
    0, 1, 0, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 1, 1, 1,

    0, 1, 0, 0,
    0, 1, 0, 0,
    0, 1, 0, 0,
    0, 1, 0, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 1, 1, 1,
    #pragma endregion

    #pragma region     绘制第二个 J
    /*
    □□□□
    □□█□
    □□█□
    □██□
    */
    0, 0, 0, 0,
    0, 0, 1, 0,
    0, 0, 1, 0,
    0, 1, 1, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 1, 0, 0,
    0, 1, 1, 1,

    0, 0, 0, 0,
    0, 1, 1, 0,
    0, 1, 0, 0,
    0, 1, 0, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 1, 1, 0,
    0, 0, 1, 0,
    #pragma endregion

    #pragma region 绘制第三个 L
    /*
    □□□□
    □█□□
    □█□□
    □██□
    */
    0, 0, 0, 0,
    0, 1, 0, 0,
    0, 1, 0, 0,
    0, 1, 1, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 1, 1, 1,
    0, 1, 0, 0,

    0, 0, 0, 0,
    0, 1, 1, 0,
    0, 0, 1, 0,
    0, 0, 1, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 1, 0,
    1, 1, 1, 0,
    #pragma endregion

    #pragma region  绘制第四个 O
    /*
    □□□□
    □□□□
    □██□
    □██□
    */
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 1, 1, 0,
    0, 1, 1, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 1, 1, 0,
    0, 1, 1, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 1, 1, 0,
    0, 1, 1, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 1, 1, 0,
    0, 1, 1, 0,
    #pragma endregion

    #pragma region 绘制第五个 S
    /*
    □□□□
    □□□□
    □██□
    ██□□
    */
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 1, 1, 0,
    1, 1, 0, 0,

    0, 0, 0, 0,
    0, 1, 0, 0,
    0, 1, 1, 0,
    0, 0, 1, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 1, 1, 0,
    1, 1, 0, 0,

    0, 0, 0, 0,
    0, 1, 0, 0,
    0, 1, 1, 0,
    0, 0, 1, 0,
    #pragma endregion

    #pragma region 绘制第六个 T
    /*
    □□□□
    □□□□
    ███□
    □█□□
    */
    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 1, 1, 0,
    0, 1, 0, 0,

    0, 0, 0, 0,
    0, 1, 0, 0,
    1, 1, 0, 0,
    0, 1, 0, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 1, 0, 0,
    1, 1, 1, 0,

    0, 0, 0, 0,
    0, 1, 0, 0,
    0, 1, 1, 0,
    0, 1, 0, 0,
    #pragma endregion

    #pragma region 绘制第七个 Z
    /*
    □□□□
    □□□□
    ██□□
    □██□
    */
    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 1, 0, 0,
    0, 1, 1, 0,

    0, 0, 0, 0,
    0, 0, 1, 0,
    0, 1, 1, 0,
    0, 1, 0, 0,

    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 1, 0, 0,
    0, 1, 1, 0,

    0, 0, 0, 0,
    0, 0, 1, 0,
    0, 1, 1, 0,
    0, 1, 0, 0,
    #pragma endregion
};

#pragma region day02

// 检查游戏是否结束
static void CheckGameOver(void)
{
    // 第一种: 顶部有方块则游戏结束
    // 第二种: 方块堆积到顶部, 每一行都有方块, 则游戏结束
    // 反推: 有一行没有方块, 那就证明还没有结束
    int i = 0, j = 0;
    // 定义一个方块存在还是不存在
    int LineBlockFlags = 0; // 1 存在方块 0 不存在方块
    int GameOverFlags = 1;  // 0 未结束    1 结束

    // 开始循环
    for(i = 0; i < CONTAINER_HEIGHT && 1 == GameOverFlags; i++)
    {
        LineBlockFlags = 0;

        // 内部循环
        for(j = 0; j < CONTAINER_WIDTH && 0 == LineBlockFlags; j++)
        {
            if(1 == Container[i][j])  // 一旦相等就证明我这个游戏马上结束
            {
                LineBlockFlags = 1;
            }
        }

        // 还没有结束
        if(0 == LineBlockFlags)
        {
            GameOverFlags = 0;
        }
    }

    Tetris.GameState = (0 == GameOverFlags) ? 1 : 0;
}

// 复制游戏里的方块复制他所在的位置包括一些提醒功能
static void CopyToContainer(void)
{
    int i = 0, j = 0;

    for(i = Tetris.min_top; i <= Tetris.max_bottom; ++i)
    {
        for(j = Tetris.min_left; j <= Tetris.max_right; ++j)
        {
            // 判断当前方块的宽度  只复制出现在游戏中出现的方块
            if(1 == *(Tetris.current_block + BLOCK_WIDTH * i + j)
                    && 0 <= Tetris.offset_top + i && 0 <= Tetris.offset_left + j)
            {
                // 直接复制
                // 只要求已经出现在游戏区域(容器)里面的方块
                Container[Tetris.offset_top + i][Tetris.offset_left + j] = 1;
            }
        }
    }
}

// 声明结构体处理队列的相关问题
// struct QUEUE_NODE_T
struct tagQueueNodeT
{
    int data; // 字段的数据内容
    struct tagQueueNodeT *next;  // 下一个字段的数据内容
};

// 通过上面定义的数据类型
typedef struct tagQueueNodeT QueueNodeT;  // 找到任意的位置复制, 相关的区域

// 定义入栈
static unsigned int queue_push(QueueNodeT **head, int data)
{
    // 定义一个当前节点
    QueueNodeT *currentNode = NULL;

    // 创建一个尾节点
    if(NULL == *head)
    {
        // 考虑字段链表头结点还没有出现这个未出现的情况
        // 没有出现就开辟空间
        *head = (QueueNodeT*)malloc(sizeof(QueueNodeT));
        // 马上把头指针赋给当前指针
        currentNode = *head;
    }
    // 头结点不为空
    else
    {
        // 把头结点直接赋给当前节点
        currentNode = *head;

        // 寻找尾节点
        while(NULL != currentNode->next)
        {
            currentNode = (QueueNodeT*)currentNode->next;
        }

        // 创建下一个节点并且转到下一个节点
        {
            currentNode->next = (QueueNodeT *)malloc(sizeof(QueueNodeT));
            // 当前节点直接进行强制转换
            currentNode = (QueueNodeT*)currentNode->next;
        }
    }

    // 给尾节点赋初值
    currentNode->data = data;
    currentNode->next = NULL;
    return 1;
}

// 出栈
static unsigned int queue_pop(QueueNodeT **head, int *data)
{
    // 定义一个中间变量
    QueueNodeT *temp = NULL;
    // 把头结点赋值给中间变量 保证头结点不会被丢掉
    temp = (QueueNodeT*)(*head)->next;
    *data = (*head)->data;

    // 判断头结点是否为空
    if(NULL != *head)
    {
        free(*head);
        *head = NULL;
    }

    *head = temp;
    return 1;
}

// 消除一行要计分
static void CalculateScore(void)
{
    QueueNodeT *head = NULL;
    unsigned int award = 0;
    int count = 0;
    int i = 0, j = 0; // 横纵坐标
    int i_max = 0;
    // 消去一行发生后存入木块的四行(需要保存起来)  只需要考虑在游戏容器里面出现的方块就行了
    //
    // 记录需要消除的一行, 消除行一定发生在存入积木行的四行
    // 只需要考虑已经出现在游戏容器里面的方块就行了 (消除之后就不用管了只需要记录那些还没有消除的就好了)
    i = (0 < Tetris.offset_top + Tetris.min_top) ? Tetris.offset_top + Tetris.min_top : 0;
    i_max = (0 < Tetris.offset_top + Tetris.max_bottom) ? Tetris.offset_top + Tetris.max_bottom : 0;

    //得到i i_max的值就可以赋值宽度   是否进行统计分数  就是通过这个来完成
    for(; i <= i_max; ++i)
    {
        // 统计分数
        count = 0;

        // 内部循环
        for(j = 0; j < CONTAINER_WIDTH; ++j)
        {
            count += Container[i][j];
        }

        if(count == CONTAINER_WIDTH)
        {
            queue_push(&head, i); // 出栈
            ++award;
        }
    }

    // 统计分数之后就消除行
    if(0 != award)
    {
        while(head != NULL)    // 头结点不等于空
        {
            // 统计行数
            int Line = 0;
            queue_pop(&head, &Line);    // 入栈

            // 循环处理
            for(i = Line - 1; i >= 0; i--)   // 行的高度
            {
                // 行的高度是如何进行复制的   就是内存
                memcpy(&Container[i + 1][0], &Container[i][0], sizeof(unsigned char)*CONTAINER_WIDTH);
                // 最顶层不可能出现方块消除 会导致游戏结束 所以要保证倒数第二行是空白的情况下 从倒数第一行复制
            }

            // 最顶层不可能出现方块消除的行 (会导致游戏结束), 所以我们保证了倒数第二行是空白的(从倒数第一行复制),
            // 而且倒数第一行的空白也是合理的, 不需要调整
        }

        // 统计分数  得分
        Tetris.Score += ScoreSets[award - 1];
        // 变速 (等级越高速度越快)(分数高 速度越快)
        Tetris.OriginalSpeed = Tetris.Score / 10000;
        Tetris.OriginalSpeed += Tetris.InitialSpeed; // 初始化速度加原来的速度就变快了
        Tetris.OriginalSpeed %= 50; // 在速度情况下我们设置为一半  就是50%
    }
}

// 计算一下 比如左边的最小距离, 右边的最大距离, 左边的顶部距离, 右边的底部距离, 才可以检查我们的游戏是否结束等等
// 计算方块边界
static void CalculateBlockBoundry(void)
{
    int i = 0, j = 0, isFounded = 0;
    // 1. 计算最小左边的距离
    // CalculateMinLeft
    isFounded = 0;

    for(j = 0; j < BLOCK_WIDTH && 0 == isFounded; ++j)
    {
        // ++ 往下进行计算
        for(i = 0; i < BLOCK_WIDTH && 0 == isFounded; ++i)
        {
            // 判断宽度 计算左边距离
            // 当前的方块加上宽度再加上坐标
            if(*(Tetris.current_block + BLOCK_WIDTH * i + j) == 1)
            {
                // 把介乎给他变换
                Tetris.min_left = j;
                isFounded = 1;
                // 变我们的左边距离
            }
        }
    }

    // 2. 计算最大右边的距离
    // CalculateMaxRight
    isFounded = 0;

    for(j = BLOCK_WIDTH - 1; j >= 0 && 0 == isFounded; --j)
    {
        for(i = 0; i < BLOCK_WIDTH && 0 == isFounded; ++i)
        {
            if(*(Tetris.current_block + BLOCK_WIDTH * i + j) == 1)
            {
                Tetris.max_right = j;
                isFounded = 1;
            }
        }
    }

    // 3. 计算最小顶部的距离
    // CalculateMinTop
    isFounded = 0;

    for(i = 0; i < BLOCK_WIDTH && 0 == isFounded; ++i)
    {
        for(j = 0; j < BLOCK_WIDTH && 0 == isFounded; ++j)
        {
            if(*(Tetris.current_block + BLOCK_WIDTH * i + j) == 1)
            {
                Tetris.min_top = i;
                isFounded = 1;
            }
        }
    }

    // 4. 计算最大低部的距离
    // CalculateMaxBottom
    isFounded = 0;

    for(i = BLOCK_WIDTH - 1; i >= 0 && 0 == isFounded; --i)
    {
        for(j = 0; j < BLOCK_WIDTH && 0 == isFounded; ++j)
        {
            if(*(Tetris.current_block + BLOCK_WIDTH * i + j) == 1)
            {
                Tetris.max_bottom = i;
                isFounded = 1;
            }
        }
    }
}

// 生成方块
static void GenerateBlock(void)
{
    // 判断游戏是否结束
    CheckGameOver();

    if(Tetris.GameState == 0)
    {
        return;
    }

    // 否则判断当前这个方块是否为空
    /*生成方块*/
    if(NULL == Tetris.current_block)
    {
        // 第一次生成方块 否则就下一个
        // 方块生成这种方向类型都是随机的
        srand((unsigned int)time(0));
        Tetris.current_block_kind = rand() % BLOCK_KINDS;
        // 旋转方向
        Tetris.current_block_rotate = rand() % BLOCK_ROTATES;
        // 生成当前方块
        Tetris.current_block = BlockSets + Tetris.current_block_kind * BLOCK_SIZE * BLOCK_ROTATES + Tetris.current_block_rotate * BLOCK_SIZE;
        // 提醒下一个类型是什么样的
        Tetris.next_block_kind = rand() % BLOCK_KINDS;
        Tetris.next_block_rotate = rand() % BLOCK_ROTATES;
    }
    // 否则直接处理 直接处理
    /*当前方块*/
    else
    {
        // 下一个方块
        Tetris.current_block_kind = Tetris.next_block_kind;
        // 旋转方向
        Tetris.current_block_rotate = Tetris.next_block_rotate;
        // 生成当前方块 下一个方块
        Tetris.current_block = Tetris.next_block;
        srand((unsigned int)time(0));
        // 提醒下一个类型是什么样的
        Tetris.next_block_kind = rand() % BLOCK_KINDS;
        Tetris.next_block_rotate = rand() % BLOCK_ROTATES;
    }

    /*下一个方块*/
    // 判断落地的位置 (下一个方块)
    Tetris.next_block = BlockSets + Tetris.next_block_kind * BLOCK_SIZE * BLOCK_ROTATES + Tetris.next_block_rotate * BLOCK_SIZE;
    // (生成方块之后要) 计算方块的边界
    CalculateBlockBoundry();
    // 新方块从中间落下, 一行一行的下落
    // 原始数据椒偏向左边 一般玩游戏的思维都是偏向左边
    Tetris.offset_left = (CONTAINER_WIDTH - BLOCK_WIDTH) / 2 - 1 + 1; // 因为原始的数据一般都是偏向左边, 所以我们要补一个 1
    Tetris.offset_top = -Tetris.max_bottom;
}

// 检测方向是否发生碰撞问题
// 检测方块碰撞问题
static unsigned int DetectCollision(const unsigned char *block, int offset_left, int offset_top)
{
    // 返回 1 发生碰撞  比如超过了左边 或者超过了右边  或者下边
    // 发生检测碰撞(新积木与容器内部的积木碰撞问题): 1 发生碰撞(超过左边界  或者右边界 或者下边框)  0 没有碰撞
    // 通过键盘控制方块的方向   碰到左边边界  肯定不能移动(往左边移动)   碰到右边  就是不可以往右边移动   到了下面就是不可以往下面走
    // 定义一个状态的标记
    unsigned int state = 0;

    if(0 <= offset_left + Tetris.min_left // 检测左边界
            && offset_left + Tetris.max_right <= CONTAINER_WIDTH - 1 // 同时又要检测右边界
            // 新生成的方块允许上边界交叉
            && offset_top + Tetris.max_bottom <= CONTAINER_HEIGHT - 1) // 再检测下边界
    {
        // 一旦满足这个条件 调整左边 i j都为0
        int i = 0, j = 0;

        // 直接判断左右边  还有新生成的方块是否重叠  等等  这些现象  都在这儿实现
        // 循环判断左右
        for(i = Tetris.min_top; i <= Tetris.max_bottom && state == 0; ++i)
        {
            for(j = Tetris.min_left; j <= Tetris.max_right && state == 0; ++j)
            {
                // 判断方块的宽度是否等于 1   还有这个容器的左边top和偏移的
                if(*(block + BLOCK_WIDTH * i + j) == 1
                        && Container[offset_top + i][offset_left + j] == 1)  // 1 新方块允许上边界交叉  但是太允许与下边的积木碰撞重叠  只需要考虑 方块与容器内部的检测 就是说忽略超过上边界部分 应为都是往下落
                {
                    // 新方块可以允许这个边界进行交叉, 但是不允许与下方容器(内部积木)重叠(碰撞)
                    // 只需要考虑方块与容器内积木内碰撞, 忽略超出上边界部分的问题就行了
                    if(offset_top + Tetris.min_top >= 0)
                    {
                        state = 2;  // 证明发生碰撞(新积木)
                    }
                }
            }
        }
    }
    else
        // 发生碰撞  超过 左右边
    {
        state = 1;  // 证明发生碰撞: 已经超过了 左边  或者右边 或者下边框
    }

    return state;   // 返回超过的结果
}

#pragma endregion

#pragma region day03
// 设置俄罗斯方块往左边 往右边 或者往下掉的问题
// 往左边移动
static void StepLeft(void) // 设置往左边
{
    --Tetris.offset_left;

    if(DetectCollision(Tetris.current_block, Tetris.offset_left, Tetris.offset_top))
    {
        ++Tetris.offset_left;
    }
}

static void StepRight(void) // 设置往右边
{
    ++Tetris.offset_left;

    if(DetectCollision(Tetris.current_block, Tetris.offset_left, Tetris.offset_top))
    {
        --Tetris.offset_left;
    }
}

static int StepDown(void) // 设置往下面
{
    // 0: 碰到底边 1: 正常下移
    ++Tetris.offset_top;

    if(DetectCollision(Tetris.current_block, Tetris.offset_left, Tetris.offset_top))
    {
        --Tetris.offset_top;
        // 复制方块
        CopyToContainer();
        // 计分
        CalculateScore();
        // 生成方块
        GenerateBlock();
        return 0;
    }

    // 否则返回 1 就是正常落下
    return 1;
}

// 如果我们重叠之后 满了一条就把他删除
// 跟新当前目标位置
static void UpdateCurrBlockDestinationPosition(void)
{
    // 把顶部的方向确定好
    Tetris.offset_top_destination = Tetris.offset_top;

    // 其实跟新过程当中就是一个死循环(不断的操作 不断循环  不断的更新)
    for(;;)
    {
        if(DetectCollision(Tetris.current_block, Tetris.offset_left, Tetris.offset_top_destination)) // 目标位置更新就是 对我们往下 做出一个正确的判断
        {
            --Tetris.offset_top_destination;
            break; // 满足条件  直接中断
        }

        // 否则  就++
        ++Tetris.offset_top_destination;
    }
}

// 设置旋转方向
static void StepRotateDirection(void)
{
    ++Tetris.current_block_rotate;
    // 根据方块来旋转的
    Tetris.current_block_rotate %= BLOCK_ROTATES;
    // 旋转当前方块的种类  方向都要旋转起来
    Tetris.current_block = BlockSets + Tetris.current_block_kind * BLOCK_SIZE * BLOCK_ROTATES + Tetris.current_block_rotate * BLOCK_SIZE;
    // 计算一下边界  因为有可能你计算的时候有可能计算不了
    // 计算新的边界
    CalculateBlockBoundry();

    // 在边界的外面需要调整
    // 首先设置一下右边界
    if(Tetris.offset_left + Tetris.max_right > (CONTAINER_WIDTH - 1)) // 大于宽度  就是移嘛
    {
        Tetris.offset_left = (CONTAINER_WIDTH - 1) - Tetris.max_right;// Tetris.max_right 方块的最大值
    }

    // 左边界
    if(Tetris.offset_left + Tetris.offset_left < 0)
    {
        Tetris.offset_left = -Tetris.offset_left;
    }

    // 下边界
    if(Tetris.offset_top + Tetris.max_bottom/*方块的最大值*/ > CONTAINER_HEIGHT - 1) // 说明到底了
    {
        Tetris.offset_top = CONTAINER_HEIGHT - 1 - Tetris.max_bottom/*底部的边界*/;
    }

    // 判断下边界 之后 一定要判断一下方向
    // 检测是否碰撞下边界(就是碰到下面)
    if(DetectCollision(Tetris.current_block, Tetris.offset_left, Tetris.offset_top))
    {
        // --分数
        --Tetris.Score;
        // 旋转
        Tetris.current_block_rotate %= BLOCK_ROTATES;
        Tetris.current_block = BlockSets + Tetris.current_block_kind * BLOCK_SIZE * BLOCK_ROTATES + Tetris.current_block_rotate * BLOCK_SIZE;
        // 恢复旋转之前的边界 (万一到下面到下面你要想换到另外一个方向)
        CalculateBlockBoundry();
    }
}
// 绘制边框 画边框
static void OnDrawBorder(HDC hdc_mem) // 绘制边框(绘制框架)
{
    HPEN hPen = { 0 };
    HPEN hOldPen = { 0 };
    // 创建一只画虚线的画笔
    hPen = CreatePen(PS_DASH, 1, RGB(0xC0, 0xC0, 0xC0));
    // 将虚线的画笔分配给DC
    hOldPen = (HPEN)SelectObject(hdc_mem, hPen);
    // 当前方块容器边界
    Rectangle(hdc_mem, ContainerRect.left, ContainerRect.top, ContainerRect.right, ContainerRect.bottom);
    // 下一个方块容器的边界 (和上面的画法是一样的 没多大的区别)
    Rectangle(hdc_mem, NextBlockRect.left, NextBlockRect.top, NextBlockRect.right, NextBlockRect.bottom);
    // 画一个对齐的辅助线
#if 0
    /*列对其的辅助线*/
    MoveToEx(hdc_mem, ContainerRect, left + (CONTAINER_WIDTH / 2) * (CELLS_WIDTH + 1) + CELLS_DISTANCE, CELLS_DISTANCE.top + CELLS_DISTANCE, NULL);
    LineTo(hdc_mem, ContainerRect, left + (CONTAINER_WIDTH / 2) * (CELLS_WIDTH + 1) + CELLS_DISTANCE, CELLS_DISTANCE.bottom - CELLS_DISTANCE);
#else
    // 对标准程序没有影响
    // 采用与现实最后下落的位置, 最后直接来优化体验给用户
#endif // 0
    // 恢复设置
    SelectObject(hdc_mem, hOldPen);
    // 删除画笔
    DeleteObject(hPen);
    return;
}

// 绘制速度
static void OnDrawSpeedSelect(HDC hdc_mem)  // 速度
{
    // 把字符显示到窗口来
    // 定义一个文本的字符
    TCHAR buffer[64] = { 0 };
    int size = 0;
    size = wsprintf(buffer, TEXT("游戏初始速度增加->↑"));
    TextOut(hdc_mem, ContainerRect.left + (CONTAINER_WIDTH / 3)*CELLS_WIDTH, ContainerRect.top + (CONTAINER_HEIGHT / 2)*CELLS_WIDTH + 1 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("游戏初始速度减少->↓"));
    TextOut(hdc_mem, ContainerRect.left + (CONTAINER_WIDTH / 3)*CELLS_WIDTH, ContainerRect.top + (CONTAINER_HEIGHT / 2)*CELLS_WIDTH + 2 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("设置结束和开始速度->S"));
    TextOut(hdc_mem, ContainerRect.left + (CONTAINER_WIDTH / 3)*CELLS_WIDTH, ContainerRect.top + (CONTAINER_HEIGHT / 2)*CELLS_WIDTH + 3 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("游戏当前初始速度->%d"), Tetris.InitialSpeed + 1);
    TextOut(hdc_mem, ContainerRect.left + (CONTAINER_WIDTH / 3)*CELLS_WIDTH, ContainerRect.top + (CONTAINER_HEIGHT / 2)*CELLS_WIDTH + 4 * CELLS_WIDTH, buffer, size);
}

// 绘制方块
static void OnDrawBlock(HDC hdc_mem/*绘制方块要用到句柄*/)
{
    // 说白了就是调用数组
    // 定义三个画刷
    HBRUSH hBrushPrompt = { 0 };        // 提示画刷
    HBRUSH hBrushBlue   = { 0 };        // 蓝色画刷(提示的颜色)
    HBRUSH hBrushOld    = { 0 };        // 旧的画刷
    // 创建一个是实心的画刷 填充图形内部
    // 创建一个实心的画刷, 系统主要是用来填充图形内部的区域位置
    hBrushBlue = CreateSolidBrush(RGB(0, 0, 255)); // 蓝色
    hBrushPrompt = CreateSolidBrush(RGB(0X07, 0XFF, 0X00)); // 绿色
    // 将实心的刷子分配内存的DC句柄
    hBrushOld = (HBRUSH)SelectObject(hdc_mem, hBrushPrompt);
    // 绘制当前方块方向的位置
    // 绘制当前方块的目标位置
    {
        int i = 0, j = 0;

        for(i = 0; i < BLOCK_WIDTH; ++i)
        {
            for(j = 0; j < BLOCK_WIDTH; ++j)
            {
                // 只画出现在容器里出现的方块
                // 只画出现在窗口内的方块
                if(1 == *(Tetris.current_block + BLOCK_WIDTH * i + j)
                        && 0 <= Tetris.offset_left + j
                        && 0 <= Tetris.offset_top_destination + i)
                {
                    // 画一个矩形
                    Rectangle(hdc_mem,
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_left + j) * (CELLS_WIDTH + 1),
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_top_destination + i) * (CELLS_WIDTH + 1),
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_left + j) * (CELLS_WIDTH + 1) + CELLS_WIDTH,
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_top_destination + i) * (CELLS_WIDTH + 1) + CELLS_WIDTH);
                }
            }
        }
    }
    // 将实心画刷直接分配给内存DC
    hBrushOld = (HBRUSH)SelectObject(hdc_mem, hBrushBlue);
    // 处理绿色
    DeleteObject(hBrushPrompt);
    // 绘制当前方块
    {
        int i = 0, j = 0;

        for(i = 0; i < BLOCK_WIDTH; ++i)
        {
            for(j = 0; j < BLOCK_WIDTH; ++j)
            {
                // 只画出现在容器里出现的方块
                // 只画出现在窗口内的方块
                if(1 == *(Tetris.current_block + BLOCK_WIDTH * i + j)
                        && 0 <= Tetris.offset_left + j
                        && 0 <= Tetris.offset_top + i)
                {
                    // 画一个矩形
                    Rectangle(hdc_mem,
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_left + j) * (CELLS_WIDTH + 1),
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_top + i) * (CELLS_WIDTH + 1),
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_left + j) * (CELLS_WIDTH + 1) + CELLS_WIDTH,
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_top + i) * (CELLS_WIDTH + 1) + CELLS_WIDTH);
                }
            }
        }
    }
    // 绘制容器
    {
        int i = 0, j = 0;

        for(i = 0; i < CONTAINER_HEIGHT; ++i)
        {
            for(j = 0; j < CONTAINER_WIDTH; ++j)
            {
                // 只画出现在容器里出现的方块
                // 只画出现在窗口内的方块
                if(1 == Container[i][j])  // 等于 1 就可以绘制
                {
                    // 画一个矩形
                    Rectangle(hdc_mem,
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE + j * (CELLS_WIDTH + 1),
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE +  i * (CELLS_WIDTH + 1),
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE +  j * (CELLS_WIDTH + 1) + CELLS_WIDTH,
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE +  i * (CELLS_WIDTH + 1) + CELLS_WIDTH);
                }
            }
        }
    }
    // 绘制下一个方块
    {
        int i = 0, j = 0;

        for(i = 0; i < BLOCK_WIDTH; ++i)
        {
            for(j = 0; j < BLOCK_WIDTH; ++j)
            {
                // 只画出现在容器里出现的方块
                // 只画出现在窗口内的方块
                if(1 == *(Tetris.next_block + BLOCK_WIDTH * i + j))
                {
                    // 画一个矩形
                    Rectangle(hdc_mem,
                              NextBlockRect.left + CELLS_DISTANCE + CELLS_DISTANCE + j * (CELLS_WIDTH + 1),
                              NextBlockRect.top + CELLS_DISTANCE + CELLS_DISTANCE + i * (CELLS_WIDTH + 1),
                              NextBlockRect.left + CELLS_DISTANCE + CELLS_DISTANCE + j * (CELLS_WIDTH + 1) + CELLS_WIDTH,
                              NextBlockRect.top + CELLS_DISTANCE + CELLS_DISTANCE + i * (CELLS_WIDTH + 1) + CELLS_WIDTH);
                }
            }
        }
    }
    // 把该删除的删除  恢复当前的设置
    // 调用一下
    // 恢复当前设置
    SelectObject(hdc_mem, hBrushOld);
    DeleteObject(hBrushBlue);
    return;
}
#pragma endregion

#pragma region day04
// 在实现的时候 注意一些问题  绘制一些相关的结果 比如输出开始 暂停 退出 旋转 左转等等  这些都必须学会
static void OnDrawResult(HDC hdc_mem)
{
    // 在屏幕上输出我们的相关的字符
    TCHAR buffer[64] = { 0 };
    int size = 0;
    // size输出的是分数
    size = wsprintf(buffer, TEXT("%d"), Tetris.Score);//wsprintf 格式化输出
    // 输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 1 * CELLS_WIDTH, TEXT("SCORE:"), 6);
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 2 * CELLS_WIDTH, buffer, size);
    // 输出速度问题
    size = wsprintf(buffer, TEXT("%d"), Tetris.OriginalSpeed + 1); //wsprintf 格式化输出
    // 输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 3 * CELLS_WIDTH, TEXT("SPEED:"), 6);
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 4 * CELLS_WIDTH, buffer, size);

    // switch 来判断游戏到底是处于结束还是处于暂停
    switch(Tetris.GameState)
    {
        case 0:
            {
                TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 5 * CELLS_WIDTH, TEXT("Game Over!"), 12);
            }
            break;

        case 1:
            {
            }
            break;

        case 2:
            {
                TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 5 * CELLS_WIDTH, TEXT("Game Pause!"), 10);
            }
            break;

        default:
            {
            }
            break;
    }

    SetBkMode(hdc_mem, TRANSPARENT);
    size = wsprintf(buffer, TEXT("开始 = S"));//wsprintf 格式化输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 6 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("暂停 = P"));//wsprintf 格式化输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 7 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("退出 = ESC"));//wsprintf 格式化输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 8 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("旋转 = ↑"));//wsprintf 格式化输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 9 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("左移 = ←"));//wsprintf 格式化输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 10 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("右移 = →"));//wsprintf 格式化输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 11 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("下移 = ↓"));//wsprintf 格式化输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 12 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("落地 = SPACE"));//wsprintf 格式化输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 13 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("重新开始 = C"));//wsprintf 格式化输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 14 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("Next:"));//wsprintf 格式化输出
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + (-4) * CELLS_WIDTH, buffer, size);
    return;
}

// 绘制(把棋盘绘制到我们窗口里面来)
static void OnPaint(HDC hdc)
{
    HDC hdc_mem = { 0 };
    HBITMAP hBitmap = { 0 };
    HBITMAP hBitmapOld = { 0 };
    // 创建内村
    hdc_mem = CreateCompatibleDC(hdc);
    // 创建一个bmp(位图)内存空间
    hBitmap = CreateCompatibleBitmap(hdc, WINDOWS_WIDTH, WINDOWS_HEIGHT);
    // 将BMP这个内存空间直接分配给内存DC(其实就是赋值)
    hBitmapOld = (HBITMAP)SelectObject(hdc_mem, hBitmap);
    // 先用背景色将位图清除干净, 这里使用白色作为背景
    PatBlt(hdc_mem, 0, 0, WINDOWS_WIDTH, WINDOWS_HEIGHT, WHITENESS);
    // 绘制
    OnDrawBorder(hdc_mem);
    // 绘制完之后就判断一下

    if(3 == Tetris.GameState)
    {
        // 选择速度问题
        OnDrawSpeedSelect(hdc_mem);
    }
    else
    {
        // 正常在玩游戏(正常游戏当中)
        UpdateCurrBlockDestinationPosition();
        // 绘制也要进行重绘制一下
        OnDrawBlock(hdc_mem);
    }

    // 然后调用结束
    OnDrawResult(hdc_mem);   // 绘制结果
    // 将内存DC里面的内容直接复制到屏幕上来
    // 将内存DC里面的内容复制到屏幕上来 显示在DC中, 完成显示工作
    BitBlt(hdc, 0, 0, WINDOWS_WIDTH, WINDOWS_HEIGHT, hdc_mem, 0, 0, SRCCOPY);
    // 复制完之后一定要选择他
    SelectObject(hdc_mem, hBitmapOld);
    DeleteObject(hBitmap); // 删除对象
    DeleteDC(hdc_mem);// 删除DC
}

#pragma endregion


//////////////////////////////////////////////////////////////////////////
//  窗口回调函数
//////////////////////////////////////////////////////////////////////////
static LRESULT CALLBACK WindowsMessageProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc = { 0 };
    PAINTSTRUCT ps = { 0 };
    LRESULT rs = { 0 };

    switch(nMsg)
    {
        case WM_CREATE:
            {
                Tetris.GameState = 3;
                Tetris.InitialSpeed = 0;
                SetTimer(hWnd, Tetris.timerid, 20, (TIMERPROC)NULL);
            }
            break;

        case WM_TIMER:
            {
                // 时间处理
                if(3 != Tetris.GameState)  // 处于玩的状态
                {
                    static unsigned int timerCounter = 0;   // 定时器计数
                    static unsigned int interval = 0;       // 间隔
                    ++timerCounter;
                    interval = 1000;    // 一秒执行一次

                    // 判断一下
                    if((timerCounter * 20) >= interval)
                    {
                        if(1 == Tetris.GameState)
                        {
                            // 直接往下落 游戏的开始
                            StepDown();
                        }

                        timerCounter = 0;
                    }

                    // 重新刷新区域绘制一下
                    InvalidateRect(hWnd, NULL, 0);
                }
            }
            break;

        // 我们是如何控制的(如何进行开始游戏)
        case WM_KEYDOWN:
            {
                switch(wParam)
                {
                    case VK_UP:
                        {
                            if(1 == Tetris.GameState)  // 处于运行状态
                            {
                                StepRotateDirection();
                            }

                            if(3 == Tetris.GameState)
                            {
                                ++Tetris.InitialSpeed;
                                Tetris.InitialSpeed = Tetris.InitialSpeed > 49 ? 0 : Tetris.InitialSpeed;
                            }
                        }
                        break;

                    case VK_LEFT:
                        {
                            if(1 == Tetris.GameState)
                            {
                                StepLeft();
                            }
                        }
                        break;

                    case VK_RIGHT:
                        {
                            // 控制方向键往右边
                            if(1 == Tetris.GameState)
                            {
                                StepRight();
                            }
                        }
                        break;

                    case VK_DOWN:
                        {
                            if(1 == Tetris.GameState) // 处于运行状态
                            {
                                StepDown();
                            }

                            if(3 == Tetris.GameState)
                            {
                                --Tetris.InitialSpeed;
                                Tetris.InitialSpeed = Tetris.InitialSpeed > 49 ? 49 : Tetris.InitialSpeed;
                            }
                        }
                        break;

                    case VK_SPACE:
                        {
                            // 空格直接掉下来
                            if(1 == Tetris.GameState) // 如果游戏为运行状态
                            {
                                // 按Space键 ,直接落地
                                while(1 == StepDown()) {}
                            }
                        }
                        break;

                    case 'C':
                        {
                            // 调整初始化速度
                            if(3 != Tetris.GameState)
                            {
                                Tetris.GameState = 3;
                            }
                        }
                        break;

                    case 'S':
                        {
                            // 按S键的时候离开就处于暂停的状态
                            if(2 == Tetris.GameState)
                            {
                                Tetris.GameState = 1;
                            }

                            // 按S键  就重新开始游戏
                            if((0 == Tetris.GameState) || 3 == Tetris.GameState/*暂停的时候游戏也可以继续*/)
                            {
                                // 暂停和停止的时候游戏可以重新开始
                                // 保留初始化的速度不变
                                unsigned int temp = Tetris.InitialSpeed;
                                // 初始化游戏动起来
                                memset((void*)Container, 0, sizeof(unsigned char)*CONTAINER_HEIGHT * CONTAINER_WIDTH);
                                // 清除一下
                                memset((void*)&Tetris, 0, sizeof(struct tagTETRIS));
                                // 首先游戏状态除以开始
                                Tetris.GameState = 1;
                                // 分数也是从0开始
                                Tetris.Score = 0;
                                // 修改初始化速度
                                Tetris.InitialSpeed = temp;
                                // 把初始化速度赋给他 0 + 上初始化速度 一个一个的加这个速度  是可以变得
                                Tetris.OriginalSpeed = 0 + Tetris.InitialSpeed;
                                // 处理方块
                                GenerateBlock();
                            }
                        }
                        break;

                    case 'P':
                        {
                            // 处理游戏的暂停
                            if(1 == Tetris.GameState)
                            {
                                // 按P键暂停
                                Tetris.GameState = 2;
                            }
                        }
                        break;

                    case VK_ESCAPE:
                        {
                            //PostQuitMessage(0);
                            PostMessage(hWnd, WM_DESTROY, (WPARAM)NULL, (LPARAM)NULL);
                        }
                        break;

                    default:
                        {
                            rs = DefWindowProc(hWnd, nMsg, wParam, lParam);
                        }
                        break;
                }

                InvalidateRect(hWnd, NULL, 0);
            }
            break;

        case WM_PAINT:
            {
                // 绘制窗口
                hdc = BeginPaint(hWnd, &ps);
                // 绘制当前窗口
                OnPaint(hdc);
                EndPaint(hWnd, &ps);
            }
            break;

        case WM_ERASEBKGND:
            {
                // 禁止系统刷新背景
            }
            break;

        case WM_DESTROY:
            {
                KillTimer(hWnd, Tetris.timerid);
                PostQuitMessage(0);
            }
            break;

        default:
            {
                rs = DefWindowProc(hWnd, nMsg, wParam, lParam);
            }
            break;
    }

    return rs;
}

//////////////////////////////////////////////////////////////////////////
// Main函数生成窗口
//////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    HWND hWnd = { 0 };
    MSG  msg = { 0 };
    WNDCLASS wndClass = { 0 };
    // 窗口区域
    RECT DesktopRect = { 0 };
    // 窗口句柄
    HWND hWndDesktop = { 0 };
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WindowsMessageProc;
    wndClass.cbClsExtra = 0; // 类的附加内存
    wndClass.cbWndExtra = 0; // 窗口的附加类存
    wndClass.hInstance = hInstance;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = TEXT("Tetris");
    wndClass.hIcon = (HICON)::LoadImage(NULL, L"res\\TetrisIco.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    // 注册窗口
    RegisterClass(&wndClass);
    // 获得窗口句柄
    hWndDesktop = GetDesktopWindow();
    // 获得区域
    GetClientRect(hWndDesktop, &DesktopRect);
    // 创建一个窗口
    hWnd = CreateWindow(TEXT("Tetris"),
                        TEXT("智能版俄罗斯方块 V 4.23.2018.1.13.38"),
                        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, // 窗口风格
                        (DesktopRect.right - WINDOWS_WIDTH) / 2, // 左上角的X
                        (DesktopRect.bottom - WINDOWS_HEIGHT) / 2, // 左上角的Y
                        WINDOWS_WIDTH,// 窗口的宽度
                        WINDOWS_HEIGHT,// 窗口的高度
                        NULL,
                        NULL,
                        hInstance,
                        NULL);
    // 显示窗口
    ShowWindow(hWnd, nShowCmd);
    //  更新窗口
    UpdateWindow(hWnd);
    // 循环播放背景音乐
    //PlaySound(L"res\\Hawker'sSong.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

    // 消息循环
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
/*
俄罗斯方块整体框架的实现(已经完毕)
自己完善
怎么变形  自己去做
还缺美化工作
添加一下创新

主干部分已经写玩了(还缺创新的细节)
*/