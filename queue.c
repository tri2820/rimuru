#include <math.h>
#include <stdlib.h>

#include "raylib.h"
#include "queue.h"
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

void Queue_Add(Queue* t, QueueItem tm){
    t->items[t->next]=malloc(sizeof(QueueItem));
    if (t->next>=QUEUE_SIZE) t->next=0;  

    t->items[t->next]->position = tm.position;
    t->items[t->next]->color = tm.color;
    t->items[t->next]->size = tm.size;

    t->next++;
    t->size=MAX(t->next, t->size);
}

void Queue_DrawF(Queue *Queue, Model *mark){
    // smaller from now to previous
    for (int i=0; i<Queue->size; i++){
        
        int history_index = (Queue->next-i)-1;
        if (history_index<0) history_index+=Queue->size;

        if (Queue->items[history_index] == (QueueItem*)(NULL)) continue;

        DrawModel(*mark,(Vector3){Queue->items[history_index]->position.x,\
        0.0f,\
        Queue->items[history_index]->position.z},\
        Queue->items[history_index]->size*(QUEUE_SIZE-i)/QUEUE_SIZE,\
        Queue->items[history_index]->color);
    }
}


void Queue_Draw(Queue *Queue, Model *mark){
    // no smaller from now to previous
    for (int i=0; i<Queue->size; i++){
        
        int history_index = (Queue->next-i)-1;
        if (history_index<0) history_index+=Queue->size;
        if (Queue->items[history_index] == (QueueItem*)(NULL)) continue;

        DrawModel(*mark,(Vector3){Queue->items[history_index]->position.x,\
        Queue->items[history_index]->position.y,\
        Queue->items[history_index]->position.z},\
        Queue->items[history_index]->size,\
        Queue->items[history_index]->color);

    }
}


void Queue_DrawEx(Queue *Queue, Model *mark){
    // no smaller from now to previous
    // random rotation phi
    double delta_time = GetTime()-(int)GetTime();
    for (int i=0; i<Queue->size; i++){
        int history_index = (Queue->next-i)-1;
        if (history_index<0) history_index+=Queue->size;
        if (Queue->items[history_index] == (QueueItem*)(NULL)) continue;

        DrawModelEx(*mark,(Vector3){Queue->items[history_index]->position.x,\
        Queue->items[history_index]->position.y,\
        Queue->items[history_index]->position.z}, \
        (Vector3){1,1,1},\
        delta_time*360,\
        (Vector3){Queue->items[history_index]->size,Queue->items[history_index]->size,Queue->items[history_index]->size},\
        Queue->items[history_index]->color);

    }
}