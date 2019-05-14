#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "raylib.h"
#include "player.h"
#include "queue.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define ROTATE_UPDATE_SPEED 0.05
#define ACTION_KEY KEY_Z
#define SPOUTING_SHADOW_RATIO 3
#define MAP_X 100
#define MAP_Y 100
#define N_ENEMY 12
#define N_FOOD 34
#define TOTAL_GAME_TIME 59
#define MAIN_PLAYER_FOOD_INC_SIZE 0.1
#define ENEMY_FOOD_INC_SIZE 0.073


double slow_update(double old, double new, double change_rate){
    return old=new*change_rate+old*(1-change_rate);
}


double delta_time(){
    return GetTime()-(int)GetTime();
}

void camera_follow(Camera3D *camera, Player*p){
        double change_rate=0.01;
        Vector3 camera_relativePosition = {-40.0f,28.0f,0.0f};
        camera->position.x = slow_update(camera->position.x , log(p->size+1.7)*(p->position.x + camera_relativePosition.x), 0.01);
        camera->position.y = slow_update(camera->position.y , log(p->size+1.7)*(p->position.y + camera_relativePosition.y), 0.01);
        camera->position.z = slow_update(camera->position.z , log(p->size+1.7)*(p->position.z + camera_relativePosition.z), 0.01);
        camera->target = p->position;
}

double detect_phi(Player * p){
     double updated_phi, new_phi;
        double sum_cos=0;
        double sum_sin=0;
        if (IsKeyDown(KEY_UP)) sum_cos+=1;
        if (IsKeyDown(KEY_LEFT)) sum_sin-=1;
        if (IsKeyDown(KEY_RIGHT)) sum_sin+=1;
        if (IsKeyDown(KEY_DOWN)) sum_cos-=1;
        

        if (sum_cos==0 && sum_sin==0) 
            updated_phi=p->phi;
        else
            updated_phi = atan2(sum_sin, sum_cos);
        
        // One of theses will flip the sign of updated_phi
        if (abs(2*PI+updated_phi-p->phi)<abs(updated_phi-p->phi)) updated_phi+=2*PI;
        if (abs(-2*PI+updated_phi-p->phi)<abs(updated_phi-p->phi)) updated_phi-=2*PI;
        
        // Slowly update
        new_phi = slow_update(p->phi,updated_phi,0.05);
        return new_phi;
}

int detect_spounting(){
    if (IsKeyDown(ACTION_KEY)){
        return 1;
    }   

    return 0;
}


double distance(Vector3 A, Vector3 B){
    return pow(pow(A.x-B.x,2)+ pow(A.y-B.y,2)+ pow(A.z-B.z,2),0.5f);
}


double angle_flat(Vector3 A, Vector3 B){ 
    return atan2(B.z-A.z,B.x-A.x);
}


double greedy_brain(Player *p, Queue * foods){
    double _angle = 0;
    double min_path = __FLT_MAX__;
    double new_path;
    int min_food_i=0;
    for (int i=0; i<foods->size; i++){
        if (foods->items[i]==(QueueItem *)(NULL)) continue;
        new_path = distance(p->position,foods->items[i]->position);
        if (new_path<min_path){
            min_path = new_path;
            min_food_i = i;
        }
    }

    _angle = angle_flat(p->position,foods->items[min_food_i]->position);

    return _angle;
}

Vector3 draw_animate_spouting(Player *p, Model * m, double spouting_delta){
    Vector3 position, size, last_mark_position, last_mark_size;

    last_mark_position.x = p->position.x+p->direction.x*5;
    last_mark_position.y = 0.3f;
    last_mark_position.z = p->position.z+p->direction.z*5;

    last_mark_size.x = p->size*SPOUTING_SHADOW_RATIO;
    last_mark_size.y = p->size;
    last_mark_size.z = p->size*SPOUTING_SHADOW_RATIO;

    
    for (double i=0; i<=1; i+=0.1){
        if (i<(1-spouting_delta+0.1)){
            size.x = last_mark_size.x*1/(1+pow(2.7,-((1-i)-0.5)*4))*pow(spouting_delta,0.8) + p->size*1/(1+pow(2.7,-(i-0.5)*4))*pow(1-spouting_delta,0.8);
            size.y = last_mark_size.y*1/(1+pow(2.7,-((1-i)-0.5)*4))*pow(spouting_delta,0.8) + p->size*1/(1+pow(2.7,-(i-0.5)*4))*pow(1-spouting_delta,0.8);
            size.z = last_mark_size.z*1/(1+pow(2.7,-((1-i)-0.5)*4))*pow(spouting_delta,0.8) + p->size*1/(1+pow(2.7,-(i-0.5)*4))*pow(1-spouting_delta,0.8);
        } else {
            size.x = 0;
            size.y = 0;
            size.z = 0;
        }


        position.x = last_mark_position.x*(1-i)+p->position.x*i;
        position.y = last_mark_position.y;
        position.z = last_mark_position.z*(1-i)+p->position.z*i;

        DrawModelEx(*m,position, (Vector3){0,1,0}, -p->phi/(2*PI)*360, size,p->color);
    }

    return last_mark_position;
}


Vector3 random_flat_vector(double lim){
    Vector3 v;
    v.x = rand()%(int)(lim) * (rand()%2<1?1:-1);
    v.y = 1;
    v.z = rand()%(int)(lim) * (rand()%2<1?1:-1);
    return v;
}



int game_state = 1;
Camera3D camera;
Model mark, foodcube, player_model;
Player main_player;
Queue tail, foods;
Player *enemies[N_ENEMY];
double start_time;
double spouting_delta;

void generate_food(){
    // Generate food
    for (int i=0; i<N_FOOD; i++){
        double rand_x = rand()%(MAP_X/2) * (rand()%2<1?1:-1);
        double rand_z = rand()%(MAP_Y/2) * (rand()%2<1?1:-1);
        
        Queue_Add(&foods, (QueueItem){(Vector3){rand_x,0.5,rand_z},\
        RED,\
        1
        });
    }
}


void generate_enemies(){
    // Generate enemies
    for (int i=0; i<N_ENEMY; i++){
        double rand_x = rand()%(MAP_X/2) * (rand()%2<1?1:-1);
        double rand_z = rand()%(MAP_Y/2) * (rand()%2<1?1:-1);

        Player * a_player = malloc(sizeof(Player));

        a_player->color = GetColor(rand());
        a_player->position =  (Vector3){rand_x, 0.3f, rand_z};
        a_player->speed = (Vector3){0.3f, 1.0f, 0.3f};
        a_player->size = 1.0f;
        Player_Update_Phi(a_player,0);

        enemies[i] = a_player;
    }
}


void init(){
    SetExitKey(KEY_Q);

    InitWindow(1000, 1000, "rimuru.io");

    // Camera
    camera.up = (Vector3){ 0.0f, 0.6f, 0.0f };          
    camera.fovy = 45.0f;                  
    camera.type = CAMERA_PERSPECTIVE;     
    SetCameraMode(camera, CAMERA_CUSTOM); 
}


void new_game(){
    game_state=2;

    // Models
    mark = LoadModelFromMesh(GenMeshCylinder(1, 0.01, 16));
    foodcube = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    player_model = LoadModelFromMesh(GenMeshSphere(2, 32, 32)); 

    // Main player
    main_player.color = GetColor(0x0095ffff);
    main_player.position =  (Vector3){0.0f, 0.3f, 0.0f };
    main_player.speed = (Vector3){0.3f, 1.0f, 0.3f};
    main_player.size = 1.0f;
    Player_Update_Phi(&main_player,0);


    // Tail of main player
    tail.next = tail.size = 0;

    // Foods
    foods.next = foods.size = 0;

    generate_food();
    generate_enemies();
    start_time = GetTime();
}

void game_play(){
    // Update phi of main player from controller
    double new_phi = detect_phi(&main_player);
    Player_Update_Phi(&main_player,new_phi);

    // Update phi of enemies from greedy brain
    double phi;
    for (int i=0; i<N_ENEMY; i++){
        if (enemies[i]==(Player*)(NULL)) continue;
        phi = greedy_brain(enemies[i],&foods);
        Player_Update_Phi(enemies[i],phi);   
    }

    // Detect main_player action
    int is_spouting = detect_spounting();

    // Handle collision between foods and main player
    for (int i=0; i<foods.size; i++){
        if (foods.items[i]!=(QueueItem*)(NULL)){
            if (distance(foods.items[i]->position,main_player.position)<2*main_player.size){
                if (!is_spouting){
                    main_player.size+=MAIN_PLAYER_FOOD_INC_SIZE;
                    // New food after food got destroyed
                    foods.items[i]->position = random_flat_vector(MAP_X/2);
                }
            }
        }
    }


    // Handle collision between foods and enemies
    for (int i=0; i<foods.size; i++){
        if (foods.items[i]!=(QueueItem*)(NULL)){

            for (int j=0; j<N_ENEMY; j++){
                if (enemies[j]==(Player*)(NULL)) continue;
                if (distance(foods.items[i]->position,enemies[j]->position)<2*enemies[j]->size){
                        enemies[j]->size+=ENEMY_FOOD_INC_SIZE;
                        // New food after food got destroyed
                        foods.items[i]->position = random_flat_vector(MAP_X/2);
                        break;
                }
            }

        }
    }

    // Start drawing
    BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

            // Make main player go
            main_player.position = Player_Get_New_Position(&main_player,delta_time());

            // Make enemies go
            for (int i=0; i<N_ENEMY; i++){
                if (enemies[i]==(Player*)(NULL)) continue;
                enemies[i]->position = Player_Get_New_Position(enemies[i],delta_time());

            }

            // Draw food
            Queue_DrawEx(&foods, &foodcube);

            // Draw enemies
            for (int i=0; i<N_ENEMY; i++){
                if (enemies[i]==(Player*)(NULL)) continue;
                DrawModel(player_model,enemies[i]->position, enemies[i]->size, enemies[i]->color);
            }

            // Handle main player actions
            
            Vector3 last_mark_position, last_mark_size;
            if (!is_spouting){
                // Draw main player
                DrawModel(player_model,main_player.position, main_player.size, main_player.color);
                Queue_Add(&tail, (QueueItem){main_player.position, GetColor(0x4adcf9ff),main_player.size});
                spouting_delta=0;            
            } else {
                spouting_delta=MIN(spouting_delta+0.1,1);

                // Draw main player's spouting shadow
                Vector3 last_mark_position = draw_animate_spouting(&main_player, &mark, spouting_delta);

                // Handle main player eat enemies
                for (int i=0; i<N_ENEMY; i++){
                    if (enemies[i]==(Player*)(NULL)) continue;
                    if (distance(enemies[i]->position,last_mark_position)+enemies[i]->size<main_player.size*SPOUTING_SHADOW_RATIO/2.0f){
                            main_player.size+=enemies[i]->size*0.3;
                            enemies[i] = (Player*)(NULL);
                    }
                }
            }

            // Draw tail of main player
            Queue_DrawF(&tail,&mark);
            DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ MAP_X, MAP_Y }, GetColor(0xF8D25AFF));
            DrawGrid(100, 10.0f);

        EndMode3D();

        // Check leaderboard
        int n_better=N_ENEMY+1, n_enemy=N_ENEMY+1;
        for (int i=0; i<N_ENEMY; i++){
            if (enemies[i]==(Player*)(NULL)) {
                n_enemy--;
                n_better--;
                continue;
            }
            if (enemies[i]->size < main_player.size) n_better--;
        }

        // Drawing info banner
        char str[5];
        sprintf(str, "%d/%d", n_better, n_enemy);

        DrawRectangle( 10, 10, 550, 220, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines( 10, 10, 550, 220, BLUE);
        
        DrawText("#leaderboard", 30, 140, 30, BLACK);
        DrawText(str, 70, 80, 60, BLACK);

        DrawText("Time until end", 300, 40, 30, BLACK);
        int game_remaining_time = (int)(TOTAL_GAME_TIME-GetTime()+start_time);
        sprintf(str,"%d:%d\n",(game_remaining_time/60),(game_remaining_time%60));
        DrawText(str, 260, 70, 140, BLACK);

        // Check timer
        if (game_remaining_time==0){
            game_state = 0;
        }

    EndDrawing();

    // Limiting main player's position
    if (main_player.position.x>MAP_X/2) main_player.position.x=MAP_X/2;
    if (main_player.position.x<-MAP_X/2) main_player.position.x=-MAP_X/2;
    if (main_player.position.z>MAP_Y/2) main_player.position.z=MAP_Y/2;
    if (main_player.position.z<-MAP_Y/2) main_player.position.z=-MAP_Y/2;
}


void game_intro(){
    // LIBRAY ISSUE?: cannot detect key on the web if put after drawing
    if (IsKeyPressed(ACTION_KEY)) {
        new_game();
    }

    BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawRectangle( 50, 350, 900, 400, Fade(PINK, 0.5f));
        DrawRectangleLines( 50, 350, 900, 400, RED);
        DrawText("Rimuru! - Slime in action", 80, 400, 70, BLACK);
        DrawText("Z to eat other players", 120, 490, 50, BLACK);
        DrawText("Arrow keys to move", 120, 570, 50, BLACK);
        DrawText("Q to quit", 120, 650, 50, BLACK);
    EndDrawing();
}


void game_end(){
    if (IsKeyDown(KEY_R)) new_game();

    // Draw endgame info banner
    BeginDrawing();
        char str[250];
        
        DrawRectangle( 50, 350, 900, 400, Fade(PINK, 0.5f));
        DrawRectangleLines( 50, 350, 900, 400, RED);

        sprintf(str, "You scored %f", main_player.size);
        DrawText("Congratulations!", 80, 400, 100, BLACK);
        DrawText(str, 80, 520, 50, BLACK);
        char * ranks[] = {"AMATEUR","NEW KID ON THE BLOCK","NAIVE FRESHMAN","AVERAGE HUMAN","EXPERT","LEGENDARY ONE"};
        
        char * player_rank = ranks[0];
        if (main_player.size>25){
            player_rank = ranks[5];
        } else if (main_player.size>20){
            player_rank = ranks[4];
        } else if (main_player.size>15){
            player_rank = ranks[3];
        } else if (main_player.size>10){
            player_rank = ranks[2];
        } else if (main_player.size>5){
            player_rank = ranks[1];
        } 

        sprintf(str, "Rank: %s", player_rank);
        DrawText(str, 80, 600, 40, BLACK);
        DrawText("R to restart", 720, 650, 28, BLACK);
        DrawText("Q to quit", 720, 690, 28, BLACK);
    EndDrawing();
}

void update_frame(){

    // Game running on playing mode
    if (game_state==2){
        camera_follow(&camera,&main_player);  
        UpdateCamera(&camera);         
        game_play();
    } else if (game_state==1){
        game_intro();
    } else if (game_state==0){
        game_end();
    }
}

int main()
{

    init();

    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop(update_frame, 0, 1);
    #else
        SetTargetFPS(60);               
        while (!WindowShouldClose())
        {   
            update_frame();
        }

    #endif

    CloseWindow();      
    return 0;
}