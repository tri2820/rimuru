#ifndef QUEUE_FLAG
#define QUEUE_FLAG
#define QUEUE_SIZE 200

typedef struct QueueItem{
    Vector3 position;
    Color color;
    double size;
    double angle;
} QueueItem;

typedef struct Queue{
    int next;
    int size;
    QueueItem * items[QUEUE_SIZE];
} Queue;

void Queue_Add(Queue* , QueueItem);
void Queue_DrawF(Queue*, Model *);
void Queue_Draw(Queue*, Model *);
void Queue_DrawEx(Queue*, Model *);

#endif 