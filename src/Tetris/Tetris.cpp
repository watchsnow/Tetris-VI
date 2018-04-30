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
 *              ����ϸ��֮��Ĳ���              *
 *----------------------------------------------*/
#define CELLS_WIDTH     (30)
#define CELLS_DISTANCE  (1)

/*==============================================*
 *                  ��������                    *
 *----------------------------------------------*/
#define BLOCK_KINDS     (7)
#define BLOCK_ROTATES   (4)
#define BLOCK_WIDTH     (4)     // ����ı߳�4
#define BLOCK_SIZE      (16)    // ����ռ�õľ��� 4*4 BLOCK_WIDTH * BLOCK_WIDTH

/*==============================================*
 *            ���������еķ�������              *
 *----------------------------------------------*/
static const int ScoreSets[BLOCK_WIDTH] = { 100, 400, 800, 1600 };

/*========================================================*
 * ���ɷ���Ŀռ�, �߼�����,������width, ���ϵ���height *
 *--------------------------------------------------------*/
#define CONTAINER_WIDTH     (10)
#define CONTAINER_HEIGHT    (20)

/*==============================================*
 *             ���������ľ�������               *
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
 *       ������һ�����������ľ�������           *
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
 *              ����һ�´��ڵĴ�С             *
 *----------------------------------------------*/
// ���ڿ�� = (�����)��϶ + ������һ����߽�(������һ��) + ����Ŀ�� CONTAINER_WIDTH*(��϶+CELLS_WIDTH) + ��϶ + �����ҷ��߽�+ ��϶����һ��(|)+��һ����������߽�
// �ڼ��Ϸ���Ŀ��(BLOCK_WIDTH)*(��϶+CELLS_WIDTH)+(������һ��)��϶+ ��һ���������ұ߽�+ ��϶+ ���ڵ�����ռ�õĲ���
#define WINDOWS_WIDTH   (CELLS_DISTANCE + CELLS_DISTANCE + CONTAINER_WIDTH  * (CELLS_WIDTH + 1) + CELLS_DISTANCE + CELLS_DISTANCE + CELLS_DISTANCE + CELLS_DISTANCE + BLOCK_WIDTH * (CELLS_WIDTH + 1) + CELLS_DISTANCE + CELLS_DISTANCE + CELLS_DISTANCE+  7)   // 7 Ϊ���ڱ���ռ�õ�
#define WINDOWS_HEIGHT  (CELLS_DISTANCE + CELLS_DISTANCE + CONTAINER_HEIGHT * (CELLS_WIDTH + 1) + CELLS_DISTANCE + CELLS_DISTANCE + CELLS_DISTANCE + 33) // 33 �������ĸ߶�

/*==============================================*
 *            ���ö���˹���������              *
 *----------------------------------------------*/

#pragma region ���ö���˹���������
typedef struct tagTETRIS
{
    unsigned int timerid;       // ʱ��
    unsigned int GameState;     // ��Ϸ״̬: 0 ��Ϸ����״̬, 1 ��Ϸ��ʼ, 2 ��Ϸ��ͣ
    unsigned int InitialSpeed;   // ��ʼ����Ϸ�ٶ�
    unsigned int OriginalSpeed; // ��Ϸ������ٶ�
    unsigned int Score;         // ��Ϸ�ķ���

    // ��һ������
    const unsigned char *next_block;
    unsigned int next_block_kind;           // ��һ�����������
    unsigned int next_block_rotate;         // ��һ���������ת

    // ��ǰ����
    const unsigned char *current_block;
    unsigned int current_block_kind;        // ��ǰ���������
    unsigned int current_block_rotate;      // ��ǰ�������ת
    // ��ǰ��������� (ֱ�����µ�����) ���Ƿ��������������
    int offset_left;                    // �߼����� left
    int offset_top;                     // ������߼����� top
    int offset_top_destination;         // ������ʾ��ǰ����ֱ�������λ��

    // ������� �ұ� ����������Ͻ� ���½�
    int min_left;
    int max_right;
    int min_top;
    int max_bottom;
} TETRIS;
#pragma endregion


// ��ʼ������˹���� ����һ���������͵ı���
TETRIS Tetris = { 0 };  // ����һ��������Ŷ���˹����

// �������˹��������� ����ô����
static unsigned char Container[CONTAINER_HEIGHT][CONTAINER_WIDTH] = { 0 };

/*
����ʽ
0   1   2   3   4   5   6   7   8   9   x CONTAINER_WIDTH
00  �� �� �� �� �� �� �� �� ��
01  �� �� �� �� �� �� �� �� ��
02  �� �� �� �� �� �� �� �� ��
03  �� �� �� �� �� �� �� �� ��
04  �� �� �� �� �� �� �� �� ��
05  �� �� �� �� �� �� �� �� ��
06  �� �� �� �� �� �� �� �� ��
07  �� �� �� �� �� �� �� �� ��
08  �� �� �� �� �� �� �� �� ��
09  �� �� �� �� �� �� �� �� ��
10  �� �� �� �� �� �� �� �� ��
11  �� �� �� �� �� �� �� �� ��
12  �� �� �� �� �� �� �� �� ��
13  �� �� �� �� �� �� �� �� ��
14  �� �� �� �� �� �� �� �� ��
15  �� �� �� �� �� �� �� �� ��
16  �� �� �� �� �� �� �� �� ��
17  �� �� �� �� �� �� �� �� ��
18  �� �� �� �� �� �� �� �� ��
19  �� �� �� �� �� �� �� �� ��
y CONTAINER_HEIGHT
*/

// ���÷��������
static const  unsigned char BlockSets[BLOCK_KINDS * BLOCK_ROTATES * BLOCK_SIZE] =
{
    /*
    �����еķ���ļ���( I J L O S T Z ), ȫ���㵽����� ÿ������һ��˳ʱ����ת ��ת4��, ÿ��4*4�����¼һ����״
    Ϊ�˱�֤�ײ�����ת�Ĳ��ᵼ�������������һ��,������Ųһ��, �Ӷ��Ϳ��Լ����½� ��ת���� ���ֵײ�ˮƽ
    ��
    ��     ��     ��
    ��     ��     ��     ����    ����        ����      ��
    ��   ����        ����  ����  ����        ����    ������
    ���������飬�󹵣��ҹ���͹�飬����ӣ��ҹ���
    ��       ��   ��       ��
    ���� ����
    I��һ����������Ĳ�
    J�����ң�������������㣬����������
    L������������㣬����������
    O������һ������
    S�����ң��������㣬������ɿ׶�
    Z �����ң��������㣬������ɿ׶�
    T��������

    ��ʼ����
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    */
    #pragma region ���Ƶ�һ�� I
    /*
    ��������
    ��������
    ��������
    ��������
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

    #pragma region     ���Ƶڶ��� J
    /*
    ��������
    ��������
    ��������
    ��������
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

    #pragma region ���Ƶ����� L
    /*
    ��������
    ��������
    ��������
    ��������
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

    #pragma region  ���Ƶ��ĸ� O
    /*
    ��������
    ��������
    ��������
    ��������
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

    #pragma region ���Ƶ���� S
    /*
    ��������
    ��������
    ��������
    ��������
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

    #pragma region ���Ƶ����� T
    /*
    ��������
    ��������
    ��������
    ��������
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

    #pragma region ���Ƶ��߸� Z
    /*
    ��������
    ��������
    ��������
    ��������
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

// �����Ϸ�Ƿ����
static void CheckGameOver(void)
{
    // ��һ��: �����з�������Ϸ����
    // �ڶ���: ����ѻ�������, ÿһ�ж��з���, ����Ϸ����
    // ����: ��һ��û�з���, �Ǿ�֤����û�н���
    int i = 0, j = 0;
    // ����һ��������ڻ��ǲ�����
    int LineBlockFlags = 0; // 1 ���ڷ��� 0 �����ڷ���
    int GameOverFlags = 1;  // 0 δ����    1 ����

    // ��ʼѭ��
    for(i = 0; i < CONTAINER_HEIGHT && 1 == GameOverFlags; i++)
    {
        LineBlockFlags = 0;

        // �ڲ�ѭ��
        for(j = 0; j < CONTAINER_WIDTH && 0 == LineBlockFlags; j++)
        {
            if(1 == Container[i][j])  // һ����Ⱦ�֤���������Ϸ���Ͻ���
            {
                LineBlockFlags = 1;
            }
        }

        // ��û�н���
        if(0 == LineBlockFlags)
        {
            GameOverFlags = 0;
        }
    }

    Tetris.GameState = (0 == GameOverFlags) ? 1 : 0;
}

// ������Ϸ��ķ��鸴�������ڵ�λ�ð���һЩ���ѹ���
static void CopyToContainer(void)
{
    int i = 0, j = 0;

    for(i = Tetris.min_top; i <= Tetris.max_bottom; ++i)
    {
        for(j = Tetris.min_left; j <= Tetris.max_right; ++j)
        {
            // �жϵ�ǰ����Ŀ��  ֻ���Ƴ�������Ϸ�г��ֵķ���
            if(1 == *(Tetris.current_block + BLOCK_WIDTH * i + j)
                    && 0 <= Tetris.offset_top + i && 0 <= Tetris.offset_left + j)
            {
                // ֱ�Ӹ���
                // ֻҪ���Ѿ���������Ϸ����(����)����ķ���
                Container[Tetris.offset_top + i][Tetris.offset_left + j] = 1;
            }
        }
    }
}

// �����ṹ�崦����е��������
// struct QUEUE_NODE_T
struct tagQueueNodeT
{
    int data; // �ֶε���������
    struct tagQueueNodeT *next;  // ��һ���ֶε���������
};

// ͨ�����涨�����������
typedef struct tagQueueNodeT QueueNodeT;  // �ҵ������λ�ø���, ��ص�����

// ������ջ
static unsigned int queue_push(QueueNodeT **head, int data)
{
    // ����һ����ǰ�ڵ�
    QueueNodeT *currentNode = NULL;

    // ����һ��β�ڵ�
    if(NULL == *head)
    {
        // �����ֶ�����ͷ��㻹û�г������δ���ֵ����
        // û�г��־Ϳ��ٿռ�
        *head = (QueueNodeT*)malloc(sizeof(QueueNodeT));
        // ���ϰ�ͷָ�븳����ǰָ��
        currentNode = *head;
    }
    // ͷ��㲻Ϊ��
    else
    {
        // ��ͷ���ֱ�Ӹ�����ǰ�ڵ�
        currentNode = *head;

        // Ѱ��β�ڵ�
        while(NULL != currentNode->next)
        {
            currentNode = (QueueNodeT*)currentNode->next;
        }

        // ������һ���ڵ㲢��ת����һ���ڵ�
        {
            currentNode->next = (QueueNodeT *)malloc(sizeof(QueueNodeT));
            // ��ǰ�ڵ�ֱ�ӽ���ǿ��ת��
            currentNode = (QueueNodeT*)currentNode->next;
        }
    }

    // ��β�ڵ㸳��ֵ
    currentNode->data = data;
    currentNode->next = NULL;
    return 1;
}

// ��ջ
static unsigned int queue_pop(QueueNodeT **head, int *data)
{
    // ����һ���м����
    QueueNodeT *temp = NULL;
    // ��ͷ��㸳ֵ���м���� ��֤ͷ��㲻�ᱻ����
    temp = (QueueNodeT*)(*head)->next;
    *data = (*head)->data;

    // �ж�ͷ����Ƿ�Ϊ��
    if(NULL != *head)
    {
        free(*head);
        *head = NULL;
    }

    *head = temp;
    return 1;
}

// ����һ��Ҫ�Ʒ�
static void CalculateScore(void)
{
    QueueNodeT *head = NULL;
    unsigned int award = 0;
    int count = 0;
    int i = 0, j = 0; // ��������
    int i_max = 0;
    // ��ȥһ�з��������ľ�������(��Ҫ��������)  ֻ��Ҫ��������Ϸ����������ֵķ��������
    //
    // ��¼��Ҫ������һ��, ������һ�������ڴ����ľ�е�����
    // ֻ��Ҫ�����Ѿ���������Ϸ��������ķ�������� (����֮��Ͳ��ù���ֻ��Ҫ��¼��Щ��û�������ľͺ���)
    i = (0 < Tetris.offset_top + Tetris.min_top) ? Tetris.offset_top + Tetris.min_top : 0;
    i_max = (0 < Tetris.offset_top + Tetris.max_bottom) ? Tetris.offset_top + Tetris.max_bottom : 0;

    //�õ�i i_max��ֵ�Ϳ��Ը�ֵ���   �Ƿ����ͳ�Ʒ���  ����ͨ����������
    for(; i <= i_max; ++i)
    {
        // ͳ�Ʒ���
        count = 0;

        // �ڲ�ѭ��
        for(j = 0; j < CONTAINER_WIDTH; ++j)
        {
            count += Container[i][j];
        }

        if(count == CONTAINER_WIDTH)
        {
            queue_push(&head, i); // ��ջ
            ++award;
        }
    }

    // ͳ�Ʒ���֮���������
    if(0 != award)
    {
        while(head != NULL)    // ͷ��㲻���ڿ�
        {
            // ͳ������
            int Line = 0;
            queue_pop(&head, &Line);    // ��ջ

            // ѭ������
            for(i = Line - 1; i >= 0; i--)   // �еĸ߶�
            {
                // �еĸ߶�����ν��и��Ƶ�   �����ڴ�
                memcpy(&Container[i + 1][0], &Container[i][0], sizeof(unsigned char)*CONTAINER_WIDTH);
                // ��㲻���ܳ��ַ������� �ᵼ����Ϸ���� ����Ҫ��֤�����ڶ����ǿհ׵������ �ӵ�����һ�и���
            }

            // ��㲻���ܳ��ַ����������� (�ᵼ����Ϸ����), �������Ǳ�֤�˵����ڶ����ǿհ׵�(�ӵ�����һ�и���),
            // ���ҵ�����һ�еĿհ�Ҳ�Ǻ����, ����Ҫ����
        }

        // ͳ�Ʒ���  �÷�
        Tetris.Score += ScoreSets[award - 1];
        // ���� (�ȼ�Խ���ٶ�Խ��)(������ �ٶ�Խ��)
        Tetris.OriginalSpeed = Tetris.Score / 10000;
        Tetris.OriginalSpeed += Tetris.InitialSpeed; // ��ʼ���ٶȼ�ԭ�����ٶȾͱ����
        Tetris.OriginalSpeed %= 50; // ���ٶ��������������Ϊһ��  ����50%
    }
}

// ����һ�� ������ߵ���С����, �ұߵ�������, ��ߵĶ�������, �ұߵĵײ�����, �ſ��Լ�����ǵ���Ϸ�Ƿ�����ȵ�
// ���㷽��߽�
static void CalculateBlockBoundry(void)
{
    int i = 0, j = 0, isFounded = 0;
    // 1. ������С��ߵľ���
    // CalculateMinLeft
    isFounded = 0;

    for(j = 0; j < BLOCK_WIDTH && 0 == isFounded; ++j)
    {
        // ++ ���½��м���
        for(i = 0; i < BLOCK_WIDTH && 0 == isFounded; ++i)
        {
            // �жϿ�� ������߾���
            // ��ǰ�ķ�����Ͽ���ټ�������
            if(*(Tetris.current_block + BLOCK_WIDTH * i + j) == 1)
            {
                // �ѽ�������任
                Tetris.min_left = j;
                isFounded = 1;
                // �����ǵ���߾���
            }
        }
    }

    // 2. ��������ұߵľ���
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

    // 3. ������С�����ľ���
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

    // 4. �������Ͳ��ľ���
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

// ���ɷ���
static void GenerateBlock(void)
{
    // �ж���Ϸ�Ƿ����
    CheckGameOver();

    if(Tetris.GameState == 0)
    {
        return;
    }

    // �����жϵ�ǰ��������Ƿ�Ϊ��
    /*���ɷ���*/
    if(NULL == Tetris.current_block)
    {
        // ��һ�����ɷ��� �������һ��
        // �����������ַ������Ͷ��������
        srand((unsigned int)time(0));
        Tetris.current_block_kind = rand() % BLOCK_KINDS;
        // ��ת����
        Tetris.current_block_rotate = rand() % BLOCK_ROTATES;
        // ���ɵ�ǰ����
        Tetris.current_block = BlockSets + Tetris.current_block_kind * BLOCK_SIZE * BLOCK_ROTATES + Tetris.current_block_rotate * BLOCK_SIZE;
        // ������һ��������ʲô����
        Tetris.next_block_kind = rand() % BLOCK_KINDS;
        Tetris.next_block_rotate = rand() % BLOCK_ROTATES;
    }
    // ����ֱ�Ӵ��� ֱ�Ӵ���
    /*��ǰ����*/
    else
    {
        // ��һ������
        Tetris.current_block_kind = Tetris.next_block_kind;
        // ��ת����
        Tetris.current_block_rotate = Tetris.next_block_rotate;
        // ���ɵ�ǰ���� ��һ������
        Tetris.current_block = Tetris.next_block;
        srand((unsigned int)time(0));
        // ������һ��������ʲô����
        Tetris.next_block_kind = rand() % BLOCK_KINDS;
        Tetris.next_block_rotate = rand() % BLOCK_ROTATES;
    }

    /*��һ������*/
    // �ж���ص�λ�� (��һ������)
    Tetris.next_block = BlockSets + Tetris.next_block_kind * BLOCK_SIZE * BLOCK_ROTATES + Tetris.next_block_rotate * BLOCK_SIZE;
    // (���ɷ���֮��Ҫ) ���㷽��ı߽�
    CalculateBlockBoundry();
    // �·�����м�����, һ��һ�е�����
    // ԭʼ���ݽ�ƫ����� һ������Ϸ��˼ά����ƫ�����
    Tetris.offset_left = (CONTAINER_WIDTH - BLOCK_WIDTH) / 2 - 1 + 1; // ��Ϊԭʼ������һ�㶼��ƫ�����, ��������Ҫ��һ�� 1
    Tetris.offset_top = -Tetris.max_bottom;
}

// ��ⷽ���Ƿ�����ײ����
// ��ⷽ����ײ����
static unsigned int DetectCollision(const unsigned char *block, int offset_left, int offset_top)
{
    // ���� 1 ������ײ  ���糬������� ���߳������ұ�  �����±�
    // ���������ײ(�»�ľ�������ڲ��Ļ�ľ��ײ����): 1 ������ײ(������߽�  �����ұ߽� �����±߿�)  0 û����ײ
    // ͨ�����̿��Ʒ���ķ���   ������߽߱�  �϶������ƶ�(������ƶ�)   �����ұ�  ���ǲ��������ұ��ƶ�   ����������ǲ�������������
    // ����һ��״̬�ı��
    unsigned int state = 0;

    if(0 <= offset_left + Tetris.min_left // �����߽�
            && offset_left + Tetris.max_right <= CONTAINER_WIDTH - 1 // ͬʱ��Ҫ����ұ߽�
            // �����ɵķ��������ϱ߽罻��
            && offset_top + Tetris.max_bottom <= CONTAINER_HEIGHT - 1) // �ټ���±߽�
    {
        // һ������������� ������� i j��Ϊ0
        int i = 0, j = 0;

        // ֱ���ж����ұ�  ���������ɵķ����Ƿ��ص�  �ȵ�  ��Щ����  �������ʵ��
        // ѭ���ж�����
        for(i = Tetris.min_top; i <= Tetris.max_bottom && state == 0; ++i)
        {
            for(j = Tetris.min_left; j <= Tetris.max_right && state == 0; ++j)
            {
                // �жϷ���Ŀ���Ƿ���� 1   ����������������top��ƫ�Ƶ�
                if(*(block + BLOCK_WIDTH * i + j) == 1
                        && Container[offset_top + i][offset_left + j] == 1)  // 1 �·��������ϱ߽罻��  ����̫�������±ߵĻ�ľ��ײ�ص�  ֻ��Ҫ���� �����������ڲ��ļ�� ����˵���Գ����ϱ߽粿�� ӦΪ����������
                {
                    // �·��������������߽���н���, ���ǲ��������·�����(�ڲ���ľ)�ص�(��ײ)
                    // ֻ��Ҫ���Ƿ����������ڻ�ľ����ײ, ���Գ����ϱ߽粿�ֵ����������
                    if(offset_top + Tetris.min_top >= 0)
                    {
                        state = 2;  // ֤��������ײ(�»�ľ)
                    }
                }
            }
        }
    }
    else
        // ������ײ  ���� ���ұ�
    {
        state = 1;  // ֤��������ײ: �Ѿ������� ���  �����ұ� �����±߿�
    }

    return state;   // ���س����Ľ��
}

#pragma endregion

#pragma region day03
// ���ö���˹��������� ���ұ� �������µ�������
// ������ƶ�
static void StepLeft(void) // ���������
{
    --Tetris.offset_left;

    if(DetectCollision(Tetris.current_block, Tetris.offset_left, Tetris.offset_top))
    {
        ++Tetris.offset_left;
    }
}

static void StepRight(void) // �������ұ�
{
    ++Tetris.offset_left;

    if(DetectCollision(Tetris.current_block, Tetris.offset_left, Tetris.offset_top))
    {
        --Tetris.offset_left;
    }
}

static int StepDown(void) // ����������
{
    // 0: �����ױ� 1: ��������
    ++Tetris.offset_top;

    if(DetectCollision(Tetris.current_block, Tetris.offset_left, Tetris.offset_top))
    {
        --Tetris.offset_top;
        // ���Ʒ���
        CopyToContainer();
        // �Ʒ�
        CalculateScore();
        // ���ɷ���
        GenerateBlock();
        return 0;
    }

    // ���򷵻� 1 ������������
    return 1;
}

// ��������ص�֮�� ����һ���Ͱ���ɾ��
// ���µ�ǰĿ��λ��
static void UpdateCurrBlockDestinationPosition(void)
{
    // �Ѷ����ķ���ȷ����
    Tetris.offset_top_destination = Tetris.offset_top;

    // ��ʵ���¹��̵��о���һ����ѭ��(���ϵĲ��� ����ѭ��  ���ϵĸ���)
    for(;;)
    {
        if(DetectCollision(Tetris.current_block, Tetris.offset_left, Tetris.offset_top_destination)) // Ŀ��λ�ø��¾��� ���������� ����һ����ȷ���ж�
        {
            --Tetris.offset_top_destination;
            break; // ��������  ֱ���ж�
        }

        // ����  ��++
        ++Tetris.offset_top_destination;
    }
}

// ������ת����
static void StepRotateDirection(void)
{
    ++Tetris.current_block_rotate;
    // ���ݷ�������ת��
    Tetris.current_block_rotate %= BLOCK_ROTATES;
    // ��ת��ǰ���������  ����Ҫ��ת����
    Tetris.current_block = BlockSets + Tetris.current_block_kind * BLOCK_SIZE * BLOCK_ROTATES + Tetris.current_block_rotate * BLOCK_SIZE;
    // ����һ�±߽�  ��Ϊ�п���������ʱ���п��ܼ��㲻��
    // �����µı߽�
    CalculateBlockBoundry();

    // �ڱ߽��������Ҫ����
    // ��������һ���ұ߽�
    if(Tetris.offset_left + Tetris.max_right > (CONTAINER_WIDTH - 1)) // ���ڿ��  ��������
    {
        Tetris.offset_left = (CONTAINER_WIDTH - 1) - Tetris.max_right;// Tetris.max_right ��������ֵ
    }

    // ��߽�
    if(Tetris.offset_left + Tetris.offset_left < 0)
    {
        Tetris.offset_left = -Tetris.offset_left;
    }

    // �±߽�
    if(Tetris.offset_top + Tetris.max_bottom/*��������ֵ*/ > CONTAINER_HEIGHT - 1) // ˵��������
    {
        Tetris.offset_top = CONTAINER_HEIGHT - 1 - Tetris.max_bottom/*�ײ��ı߽�*/;
    }

    // �ж��±߽� ֮�� һ��Ҫ�ж�һ�·���
    // ����Ƿ���ײ�±߽�(������������)
    if(DetectCollision(Tetris.current_block, Tetris.offset_left, Tetris.offset_top))
    {
        // --����
        --Tetris.Score;
        // ��ת
        Tetris.current_block_rotate %= BLOCK_ROTATES;
        Tetris.current_block = BlockSets + Tetris.current_block_kind * BLOCK_SIZE * BLOCK_ROTATES + Tetris.current_block_rotate * BLOCK_SIZE;
        // �ָ���ת֮ǰ�ı߽� (��һ�����浽������Ҫ�뻻������һ������)
        CalculateBlockBoundry();
    }
}
// ���Ʊ߿� ���߿�
static void OnDrawBorder(HDC hdc_mem) // ���Ʊ߿�(���ƿ��)
{
    HPEN hPen = { 0 };
    HPEN hOldPen = { 0 };
    // ����һֻ�����ߵĻ���
    hPen = CreatePen(PS_DASH, 1, RGB(0xC0, 0xC0, 0xC0));
    // �����ߵĻ��ʷ����DC
    hOldPen = (HPEN)SelectObject(hdc_mem, hPen);
    // ��ǰ���������߽�
    Rectangle(hdc_mem, ContainerRect.left, ContainerRect.top, ContainerRect.right, ContainerRect.bottom);
    // ��һ�����������ı߽� (������Ļ�����һ���� û��������)
    Rectangle(hdc_mem, NextBlockRect.left, NextBlockRect.top, NextBlockRect.right, NextBlockRect.bottom);
    // ��һ������ĸ�����
#if 0
    /*�ж���ĸ�����*/
    MoveToEx(hdc_mem, ContainerRect, left + (CONTAINER_WIDTH / 2) * (CELLS_WIDTH + 1) + CELLS_DISTANCE, CELLS_DISTANCE.top + CELLS_DISTANCE, NULL);
    LineTo(hdc_mem, ContainerRect, left + (CONTAINER_WIDTH / 2) * (CELLS_WIDTH + 1) + CELLS_DISTANCE, CELLS_DISTANCE.bottom - CELLS_DISTANCE);
#else
    // �Ա�׼����û��Ӱ��
    // ��������ʵ��������λ��, ���ֱ�����Ż�������û�
#endif // 0
    // �ָ�����
    SelectObject(hdc_mem, hOldPen);
    // ɾ������
    DeleteObject(hPen);
    return;
}

// �����ٶ�
static void OnDrawSpeedSelect(HDC hdc_mem)  // �ٶ�
{
    // ���ַ���ʾ��������
    // ����һ���ı����ַ�
    TCHAR buffer[64] = { 0 };
    int size = 0;
    size = wsprintf(buffer, TEXT("��Ϸ��ʼ�ٶ�����->��"));
    TextOut(hdc_mem, ContainerRect.left + (CONTAINER_WIDTH / 3)*CELLS_WIDTH, ContainerRect.top + (CONTAINER_HEIGHT / 2)*CELLS_WIDTH + 1 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("��Ϸ��ʼ�ٶȼ���->��"));
    TextOut(hdc_mem, ContainerRect.left + (CONTAINER_WIDTH / 3)*CELLS_WIDTH, ContainerRect.top + (CONTAINER_HEIGHT / 2)*CELLS_WIDTH + 2 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("���ý����Ϳ�ʼ�ٶ�->S"));
    TextOut(hdc_mem, ContainerRect.left + (CONTAINER_WIDTH / 3)*CELLS_WIDTH, ContainerRect.top + (CONTAINER_HEIGHT / 2)*CELLS_WIDTH + 3 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("��Ϸ��ǰ��ʼ�ٶ�->%d"), Tetris.InitialSpeed + 1);
    TextOut(hdc_mem, ContainerRect.left + (CONTAINER_WIDTH / 3)*CELLS_WIDTH, ContainerRect.top + (CONTAINER_HEIGHT / 2)*CELLS_WIDTH + 4 * CELLS_WIDTH, buffer, size);
}

// ���Ʒ���
static void OnDrawBlock(HDC hdc_mem/*���Ʒ���Ҫ�õ����*/)
{
    // ˵���˾��ǵ�������
    // ����������ˢ
    HBRUSH hBrushPrompt = { 0 };        // ��ʾ��ˢ
    HBRUSH hBrushBlue   = { 0 };        // ��ɫ��ˢ(��ʾ����ɫ)
    HBRUSH hBrushOld    = { 0 };        // �ɵĻ�ˢ
    // ����һ����ʵ�ĵĻ�ˢ ���ͼ���ڲ�
    // ����һ��ʵ�ĵĻ�ˢ, ϵͳ��Ҫ���������ͼ���ڲ�������λ��
    hBrushBlue = CreateSolidBrush(RGB(0, 0, 255)); // ��ɫ
    hBrushPrompt = CreateSolidBrush(RGB(0X07, 0XFF, 0X00)); // ��ɫ
    // ��ʵ�ĵ�ˢ�ӷ����ڴ��DC���
    hBrushOld = (HBRUSH)SelectObject(hdc_mem, hBrushPrompt);
    // ���Ƶ�ǰ���鷽���λ��
    // ���Ƶ�ǰ�����Ŀ��λ��
    {
        int i = 0, j = 0;

        for(i = 0; i < BLOCK_WIDTH; ++i)
        {
            for(j = 0; j < BLOCK_WIDTH; ++j)
            {
                // ֻ����������������ֵķ���
                // ֻ�������ڴ����ڵķ���
                if(1 == *(Tetris.current_block + BLOCK_WIDTH * i + j)
                        && 0 <= Tetris.offset_left + j
                        && 0 <= Tetris.offset_top_destination + i)
                {
                    // ��һ������
                    Rectangle(hdc_mem,
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_left + j) * (CELLS_WIDTH + 1),
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_top_destination + i) * (CELLS_WIDTH + 1),
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_left + j) * (CELLS_WIDTH + 1) + CELLS_WIDTH,
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_top_destination + i) * (CELLS_WIDTH + 1) + CELLS_WIDTH);
                }
            }
        }
    }
    // ��ʵ�Ļ�ˢֱ�ӷ�����ڴ�DC
    hBrushOld = (HBRUSH)SelectObject(hdc_mem, hBrushBlue);
    // ������ɫ
    DeleteObject(hBrushPrompt);
    // ���Ƶ�ǰ����
    {
        int i = 0, j = 0;

        for(i = 0; i < BLOCK_WIDTH; ++i)
        {
            for(j = 0; j < BLOCK_WIDTH; ++j)
            {
                // ֻ����������������ֵķ���
                // ֻ�������ڴ����ڵķ���
                if(1 == *(Tetris.current_block + BLOCK_WIDTH * i + j)
                        && 0 <= Tetris.offset_left + j
                        && 0 <= Tetris.offset_top + i)
                {
                    // ��һ������
                    Rectangle(hdc_mem,
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_left + j) * (CELLS_WIDTH + 1),
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_top + i) * (CELLS_WIDTH + 1),
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_left + j) * (CELLS_WIDTH + 1) + CELLS_WIDTH,
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE + (Tetris.offset_top + i) * (CELLS_WIDTH + 1) + CELLS_WIDTH);
                }
            }
        }
    }
    // ��������
    {
        int i = 0, j = 0;

        for(i = 0; i < CONTAINER_HEIGHT; ++i)
        {
            for(j = 0; j < CONTAINER_WIDTH; ++j)
            {
                // ֻ����������������ֵķ���
                // ֻ�������ڴ����ڵķ���
                if(1 == Container[i][j])  // ���� 1 �Ϳ��Ի���
                {
                    // ��һ������
                    Rectangle(hdc_mem,
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE + j * (CELLS_WIDTH + 1),
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE +  i * (CELLS_WIDTH + 1),
                              ContainerRect.left + CELLS_DISTANCE + CELLS_DISTANCE +  j * (CELLS_WIDTH + 1) + CELLS_WIDTH,
                              ContainerRect.top + CELLS_DISTANCE + CELLS_DISTANCE +  i * (CELLS_WIDTH + 1) + CELLS_WIDTH);
                }
            }
        }
    }
    // ������һ������
    {
        int i = 0, j = 0;

        for(i = 0; i < BLOCK_WIDTH; ++i)
        {
            for(j = 0; j < BLOCK_WIDTH; ++j)
            {
                // ֻ����������������ֵķ���
                // ֻ�������ڴ����ڵķ���
                if(1 == *(Tetris.next_block + BLOCK_WIDTH * i + j))
                {
                    // ��һ������
                    Rectangle(hdc_mem,
                              NextBlockRect.left + CELLS_DISTANCE + CELLS_DISTANCE + j * (CELLS_WIDTH + 1),
                              NextBlockRect.top + CELLS_DISTANCE + CELLS_DISTANCE + i * (CELLS_WIDTH + 1),
                              NextBlockRect.left + CELLS_DISTANCE + CELLS_DISTANCE + j * (CELLS_WIDTH + 1) + CELLS_WIDTH,
                              NextBlockRect.top + CELLS_DISTANCE + CELLS_DISTANCE + i * (CELLS_WIDTH + 1) + CELLS_WIDTH);
                }
            }
        }
    }
    // �Ѹ�ɾ����ɾ��  �ָ���ǰ������
    // ����һ��
    // �ָ���ǰ����
    SelectObject(hdc_mem, hBrushOld);
    DeleteObject(hBrushBlue);
    return;
}
#pragma endregion

#pragma region day04
// ��ʵ�ֵ�ʱ�� ע��һЩ����  ����һЩ��صĽ�� ���������ʼ ��ͣ �˳� ��ת ��ת�ȵ�  ��Щ������ѧ��
static void OnDrawResult(HDC hdc_mem)
{
    // ����Ļ��������ǵ���ص��ַ�
    TCHAR buffer[64] = { 0 };
    int size = 0;
    // size������Ƿ���
    size = wsprintf(buffer, TEXT("%d"), Tetris.Score);//wsprintf ��ʽ�����
    // ���
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 1 * CELLS_WIDTH, TEXT("SCORE:"), 6);
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 2 * CELLS_WIDTH, buffer, size);
    // ����ٶ�����
    size = wsprintf(buffer, TEXT("%d"), Tetris.OriginalSpeed + 1); //wsprintf ��ʽ�����
    // ���
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 3 * CELLS_WIDTH, TEXT("SPEED:"), 6);
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 4 * CELLS_WIDTH, buffer, size);

    // switch ���ж���Ϸ�����Ǵ��ڽ������Ǵ�����ͣ
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
    size = wsprintf(buffer, TEXT("��ʼ = S"));//wsprintf ��ʽ�����
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 6 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("��ͣ = P"));//wsprintf ��ʽ�����
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 7 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("�˳� = ESC"));//wsprintf ��ʽ�����
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 8 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("��ת = ��"));//wsprintf ��ʽ�����
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 9 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("���� = ��"));//wsprintf ��ʽ�����
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 10 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("���� = ��"));//wsprintf ��ʽ�����
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 11 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("���� = ��"));//wsprintf ��ʽ�����
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 12 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("��� = SPACE"));//wsprintf ��ʽ�����
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 13 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("���¿�ʼ = C"));//wsprintf ��ʽ�����
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + 14 * CELLS_WIDTH, buffer, size);
    size = wsprintf(buffer, TEXT("Next:"));//wsprintf ��ʽ�����
    TextOut(hdc_mem, NextBlockRect.left + CELLS_DISTANCE, NextBlockRect.bottom + (-4) * CELLS_WIDTH, buffer, size);
    return;
}

// ����(�����̻��Ƶ����Ǵ���������)
static void OnPaint(HDC hdc)
{
    HDC hdc_mem = { 0 };
    HBITMAP hBitmap = { 0 };
    HBITMAP hBitmapOld = { 0 };
    // �����ڴ�
    hdc_mem = CreateCompatibleDC(hdc);
    // ����һ��bmp(λͼ)�ڴ�ռ�
    hBitmap = CreateCompatibleBitmap(hdc, WINDOWS_WIDTH, WINDOWS_HEIGHT);
    // ��BMP����ڴ�ռ�ֱ�ӷ�����ڴ�DC(��ʵ���Ǹ�ֵ)
    hBitmapOld = (HBITMAP)SelectObject(hdc_mem, hBitmap);
    // ���ñ���ɫ��λͼ����ɾ�, ����ʹ�ð�ɫ��Ϊ����
    PatBlt(hdc_mem, 0, 0, WINDOWS_WIDTH, WINDOWS_HEIGHT, WHITENESS);
    // ����
    OnDrawBorder(hdc_mem);
    // ������֮����ж�һ��

    if(3 == Tetris.GameState)
    {
        // ѡ���ٶ�����
        OnDrawSpeedSelect(hdc_mem);
    }
    else
    {
        // ����������Ϸ(������Ϸ����)
        UpdateCurrBlockDestinationPosition();
        // ����ҲҪ�����ػ���һ��
        OnDrawBlock(hdc_mem);
    }

    // Ȼ����ý���
    OnDrawResult(hdc_mem);   // ���ƽ��
    // ���ڴ�DC���������ֱ�Ӹ��Ƶ���Ļ����
    // ���ڴ�DC��������ݸ��Ƶ���Ļ���� ��ʾ��DC��, �����ʾ����
    BitBlt(hdc, 0, 0, WINDOWS_WIDTH, WINDOWS_HEIGHT, hdc_mem, 0, 0, SRCCOPY);
    // ������֮��һ��Ҫѡ����
    SelectObject(hdc_mem, hBitmapOld);
    DeleteObject(hBitmap); // ɾ������
    DeleteDC(hdc_mem);// ɾ��DC
}

#pragma endregion


//////////////////////////////////////////////////////////////////////////
//  ���ڻص�����
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
                // ʱ�䴦��
                if(3 != Tetris.GameState)  // �������״̬
                {
                    static unsigned int timerCounter = 0;   // ��ʱ������
                    static unsigned int interval = 0;       // ���
                    ++timerCounter;
                    interval = 1000;    // һ��ִ��һ��

                    // �ж�һ��
                    if((timerCounter * 20) >= interval)
                    {
                        if(1 == Tetris.GameState)
                        {
                            // ֱ�������� ��Ϸ�Ŀ�ʼ
                            StepDown();
                        }

                        timerCounter = 0;
                    }

                    // ����ˢ���������һ��
                    InvalidateRect(hWnd, NULL, 0);
                }
            }
            break;

        // ��������ο��Ƶ�(��ν��п�ʼ��Ϸ)
        case WM_KEYDOWN:
            {
                switch(wParam)
                {
                    case VK_UP:
                        {
                            if(1 == Tetris.GameState)  // ��������״̬
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
                            // ���Ʒ�������ұ�
                            if(1 == Tetris.GameState)
                            {
                                StepRight();
                            }
                        }
                        break;

                    case VK_DOWN:
                        {
                            if(1 == Tetris.GameState) // ��������״̬
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
                            // �ո�ֱ�ӵ�����
                            if(1 == Tetris.GameState) // �����ϷΪ����״̬
                            {
                                // ��Space�� ,ֱ�����
                                while(1 == StepDown()) {}
                            }
                        }
                        break;

                    case 'C':
                        {
                            // ������ʼ���ٶ�
                            if(3 != Tetris.GameState)
                            {
                                Tetris.GameState = 3;
                            }
                        }
                        break;

                    case 'S':
                        {
                            // ��S����ʱ���뿪�ʹ�����ͣ��״̬
                            if(2 == Tetris.GameState)
                            {
                                Tetris.GameState = 1;
                            }

                            // ��S��  �����¿�ʼ��Ϸ
                            if((0 == Tetris.GameState) || 3 == Tetris.GameState/*��ͣ��ʱ����ϷҲ���Լ���*/)
                            {
                                // ��ͣ��ֹͣ��ʱ����Ϸ�������¿�ʼ
                                // ������ʼ�����ٶȲ���
                                unsigned int temp = Tetris.InitialSpeed;
                                // ��ʼ����Ϸ������
                                memset((void*)Container, 0, sizeof(unsigned char)*CONTAINER_HEIGHT * CONTAINER_WIDTH);
                                // ���һ��
                                memset((void*)&Tetris, 0, sizeof(struct tagTETRIS));
                                // ������Ϸ״̬���Կ�ʼ
                                Tetris.GameState = 1;
                                // ����Ҳ�Ǵ�0��ʼ
                                Tetris.Score = 0;
                                // �޸ĳ�ʼ���ٶ�
                                Tetris.InitialSpeed = temp;
                                // �ѳ�ʼ���ٶȸ����� 0 + �ϳ�ʼ���ٶ� һ��һ���ļ�����ٶ�  �ǿ��Ա��
                                Tetris.OriginalSpeed = 0 + Tetris.InitialSpeed;
                                // ������
                                GenerateBlock();
                            }
                        }
                        break;

                    case 'P':
                        {
                            // ������Ϸ����ͣ
                            if(1 == Tetris.GameState)
                            {
                                // ��P����ͣ
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
                // ���ƴ���
                hdc = BeginPaint(hWnd, &ps);
                // ���Ƶ�ǰ����
                OnPaint(hdc);
                EndPaint(hWnd, &ps);
            }
            break;

        case WM_ERASEBKGND:
            {
                // ��ֹϵͳˢ�±���
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
// Main�������ɴ���
//////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    HWND hWnd = { 0 };
    MSG  msg = { 0 };
    WNDCLASS wndClass = { 0 };
    // ��������
    RECT DesktopRect = { 0 };
    // ���ھ��
    HWND hWndDesktop = { 0 };
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WindowsMessageProc;
    wndClass.cbClsExtra = 0; // ��ĸ����ڴ�
    wndClass.cbWndExtra = 0; // ���ڵĸ������
    wndClass.hInstance = hInstance;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = TEXT("Tetris");
    wndClass.hIcon = (HICON)::LoadImage(NULL, L"res\\TetrisIco.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    // ע�ᴰ��
    RegisterClass(&wndClass);
    // ��ô��ھ��
    hWndDesktop = GetDesktopWindow();
    // �������
    GetClientRect(hWndDesktop, &DesktopRect);
    // ����һ������
    hWnd = CreateWindow(TEXT("Tetris"),
                        TEXT("���ܰ����˹���� V 4.23.2018.1.13.38"),
                        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, // ���ڷ��
                        (DesktopRect.right - WINDOWS_WIDTH) / 2, // ���Ͻǵ�X
                        (DesktopRect.bottom - WINDOWS_HEIGHT) / 2, // ���Ͻǵ�Y
                        WINDOWS_WIDTH,// ���ڵĿ��
                        WINDOWS_HEIGHT,// ���ڵĸ߶�
                        NULL,
                        NULL,
                        hInstance,
                        NULL);
    // ��ʾ����
    ShowWindow(hWnd, nShowCmd);
    //  ���´���
    UpdateWindow(hWnd);
    // ѭ�����ű�������
    //PlaySound(L"res\\Hawker'sSong.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

    // ��Ϣѭ��
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
/*
����˹���������ܵ�ʵ��(�Ѿ����)
�Լ�����
��ô����  �Լ�ȥ��
��ȱ��������
���һ�´���

���ɲ����Ѿ�д����(��ȱ���µ�ϸ��)
*/