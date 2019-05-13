#include <math.h>
#include "raylib.h"
#include "player.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void Player_Update_Phi(Player * p, double phi){
    while (phi>=2*PI) phi-=2*PI;
    while (phi<=-2*PI) phi+=2*PI;
    p->phi = phi;

    p->direction.z = sin(phi);
    p->direction.x = cos(phi);
    p->direction.y = 1;
}


Vector3 Player_Get_New_Position(Player * p, double delta_time){
    Vector3 new_position;

    double linear_sticky_function_value = (sin(delta_time*2*PI-PI/2)+1)/2;
    double jump_function_value = sin(delta_time*2*PI)*0.016;

    new_position.x = p->position.x+p->direction.x*p->speed.x*linear_sticky_function_value;
    new_position.y = p->position.y+p->direction.y*p->speed.y*jump_function_value;
    new_position.z = p->position.z+p->direction.z*p->speed.z*linear_sticky_function_value;
    
    return new_position;
}