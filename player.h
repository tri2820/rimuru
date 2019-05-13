#ifndef PLAYER_FLAG
#define PLAYER_FLAG

typedef struct Player{
    Color color;
    Vector3 position;

    // Could have made direction and speed using one Vector3
    // But that's hard to debug
    Vector3 direction;
    Vector3 speed;

    double size;

    double phi;

    Vector3 (*get_animate_direction)(double);
} Player;

Vector3 Player_Get_New_Position(Player *, double);
void Player_Update_Phi(Player *, double);

#endif 