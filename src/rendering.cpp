#define SDL_MAIN_HANDLED

#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <sstream>

using namespace std;

const int SCREEN_WIDTH = 720;
const int SCREEN_HEIGHT = 480;
const double PI = 3.14159265359;
int numSect = 0;
int numWalls = 0;
const double FOV = 200.0;

struct Controls {
    int w, s, a, d;
    // int sl, sr;
    // int ;
} pInput;

struct Time {
    int milliseconds;
    int late;
} T;

struct Lookup {
    vector<double> cosVal = vector<double>(360);
    vector<double> sinVal = vector<double>(360);
} Look;

struct Player {
    int x, y, z; // position
    int a; // hor AoR
    int l; // vert AoR
} P;

struct Walls {
    int x1, y1;
    int x2, y2;
    int color;
}; vector<Walls> walls = vector<Walls>(30);

struct Sectors {
    int wstart, wend;
    int z1, z2;
    int x, y;
    int d;
    int surface; // check to see if top/bottom should be drawn or not
    int c1, c2;
    vector<int> surf = vector<int>(SCREEN_WIDTH); // hold the points of the bottom or top perimeter of sector to draw pixels to
    
}; vector<Sectors> sectors = vector<Sectors>(30);

// Sector data to be loaded in from editor

vector<int> loadSectors{
    //0, 4, 0, 40, 2, 3,
    // 4, 8, 0, 40, 0, 1,    
};

// Wall data to be loaded in from editor to sector

vector<int> loadWalls{
    // 0, 0, 32, 0, 0,
    // 32, 0, 32, 32, 1,
    // 32, 32, 0, 32, 0,
    // 0, 32, 0, 0, 1,

    // 64, 0, 96, 0, 2,
    // 96, 0, 96, 32, 3,
    // 96, 32, 64, 32, 2,
    // 64, 32, 64, 0, 3,
};

double DEG2RAD(double d) {
    return (d * (PI/180.0));
}

double RAD2DEG(double r) {
    return (r * (180.0/PI));
}

int dist(int x1, int y1, int x2, int y2) {
    return sqrt( pow((x2-x1), 2) + pow((y2-y1), 2) );
}

void drawPixel(SDL_Renderer* r, int x, int y, int color) {
    y = -y+SCREEN_HEIGHT; // set origin to bottom left instead of top left
    vector<int> rgb(3);
    switch (color) {
        case 0:
            rgb = {225, 225, 0}; // yellow
            break;
        case 1:
            rgb = {0, 225, 0}; // green
            break;
        case 2:
            rgb = {0, 0, 225}; // blue  
            break;
        case 3:
            rgb = {0, 0, 0}; // black
            break;
        case 4:
            rgb = {225, 0, 0}; // red
            break;
        case 5:
            rgb = {225, 0, 225}; // magenta
            break;
        case 6:
            rgb = {0, 225, 225}; // cyan
            break;
        
    }
    SDL_SetRenderDrawColor(r, rgb.at(0), rgb.at(1), rgb.at(2), 225);
    SDL_RenderDrawPoint(r, x, y);


}

void clipBehind( int* x1, int* y1, int* z1, int x2, int y2, int z2) { //clipping
    double d1 = *y1;
    double d2 = y2;
    double d = d1-d2;
    if (d == 0)
        d = 1;
    double intersect = d1/(d);
    *x1 = *x1 + intersect*(x2 - (*x1));
    *y1 = *y1 + intersect*(y2 - (*y1));
    *z1 = *z1 + intersect*(z2 - (*z1));

    if (*y1 == 0) {
        *y1 = 1;
    }
}

void drawWall (SDL_Renderer* r, int x1, int x2, int b1, int b2, int c1, int c2, int color, int s) {
    
    int dyb = b2-b1;
    int dyc = c2-c1; 
    int dx = x2-x1;
    if (dx == 0)
        dx = 1;
    int xs = x1;

    // CLIP
    if (x1 < 0) {
        x1 = 0;
    }
    if (x2 < 0) {
        x2 = 0;
    }
    if (x1 > SCREEN_WIDTH) {
        x1 = SCREEN_WIDTH;
    }
    if (x2 > SCREEN_WIDTH) {
        x2 = SCREEN_WIDTH;
    }
    // cout << x1 << " " << x2 << endl;
    for (int x = x1; x < x2; x++) {
        int y1 = dyb*(x-xs+0.5)/dx+b1;
        int y2 = dyc*(x-xs+0.5)/dx+c1;
        
        // CLIP
        if (y1 < 0) {
            y1 = 0;
        }
        if (y2 < 0) {
            y2 = 0;
        }
        if (y1 > SCREEN_HEIGHT) {
            y1 = SCREEN_HEIGHT;
        }
        if (y2 > SCREEN_HEIGHT) {
            y2 = SCREEN_HEIGHT;
        }
        
        if (sectors.at(s).surface == 1) {
            sectors.at(s).surf.at(x) = y1;
            continue;
        }
        if (sectors.at(s).surface == 2) {
            sectors.at(s).surf.at(x) = y2;
            continue;
        }

        if (sectors.at(s).surface == -1) {
            for (int y = sectors.at(s).surf.at(x); y < y1; y++) {
                drawPixel(r, x, y, sectors.at(s).c1);
            }
        }
        if (sectors.at(s).surface == -2) {
            for (int y = y2; y < sectors.at(s).surf.at(x); y++) {
                drawPixel(r, x, y, sectors.at(s).c2);
            };
        }

        for (int y = y1; y < y2; y++) {            
            drawPixel(r, x, y, color);
        }
        // drawPixel(r, x, y1, 3);
        // drawPixel(r, x, y2, 3);
    }

}

void draw3D(SDL_Renderer* r) {
    
    vector<int> worldx(4), worldy(4), worldz(4);
    double C = Look.cosVal.at(P.a);
    double S = Look.sinVal.at(P.a);

    for (int s = 0; s < numSect-1; s++) {
        for (int w = 0; w <  numSect-1-s; w++) {
            if (sectors.at(w).d < sectors.at(w+1).d) {
                Sectors swap = sectors.at(w);
                sectors.at(w) = sectors.at(w+1);
                sectors.at(w+1) = swap;
                
            }
        }
    }


    for (int s = 0; s < numSect; s++) {

        if (sectors.at(s).z1 > P.z) { // DRAW bottom surface
            sectors.at(s).surface = 1;
        }
        else if (sectors.at(s).z2 < P.z) { // DRAW top Surface
            sectors.at(s).surface = 2;
        }
        else {
            sectors.at(s).surface = 0; // DRAW no surface
        }

        sectors.at(s).d = 0;
        for (int flip = 0; flip < 2; flip++) {
            for (int w = sectors.at(s).wstart; w < sectors.at(s).wend; w++) {
                
                int x1 = walls.at(w).x1 - P.x;
                int y1 = walls.at(w).y1 - P.y;
                int x2 = walls.at(w).x2 - P.x;
                int y2 = walls.at(w).y2 - P.y;

                if (flip == 0) {
                    int swap = x1;
                    x1 = x2;
                    x2 = swap;
                    swap = y1;
                    y1 = y2;
                    y2 = swap;
                }

                // World X position
                //cout << "Before: " << endl;
                worldx.at(0) = x1*C-y1*S;
                worldx.at(1) = x2*C-y2*S;
                worldx.at(2) = worldx.at(0);
                worldx.at(3) = worldx.at(1);
                //cout << "World x: " << worldx.at(0) << " " << worldx.at(1) << endl;
                // World Y position
                worldy.at(0) = y1*C+x1*S;
                worldy.at(1) = y2*C+x2*S;
                worldy.at(2) = worldy.at(0);
                worldy.at(3) = worldy.at(1);

                sectors.at(s).d += dist(0, 0, (worldx.at(0) + worldx.at(1))/2, (worldy.at(0) + worldy.at(1))/2);
                //cout << "World y: " << worldy.at(0) << " " << worldy.at(1) << endl;
                //World Z position
                worldz.at(0) = sectors.at(s).z1-P.z+((P.l*worldy.at(0))/32.0);
                worldz.at(1) = sectors.at(s).z1-P.z+((P.l*worldy.at(1))/32.0);
                worldz.at(2) = worldz.at(0) + sectors.at(s).z2;
                worldz.at(3) = worldz.at(1) + sectors.at(s).z2;

                if (worldy.at(0) < 1 && worldy.at(1) < 1) {
                    continue;
                }

                if (worldy.at(0) < 1) {
                    clipBehind(&worldx.at(0), &worldy.at(0), &worldz.at(0), worldx.at(1), worldy.at(1), worldz.at(1));
                    clipBehind(&worldx.at(2), &worldy.at(2), &worldz.at(2), worldx.at(3), worldy.at(3), worldz.at(3));
                }

                if (worldy.at(1) < 1) {
                    clipBehind(&worldx.at(1), &worldy.at(1), &worldz.at(1), worldx.at(0), worldy.at(0), worldz.at(0));
                    clipBehind(&worldx.at(3), &worldy.at(3), &worldz.at(3), worldx.at(2), worldy.at(2), worldz.at(2));
                }

                //screen x, screen y position
                worldx.at(0) = (worldx.at(0)*(FOV/worldy.at(0)))+(SCREEN_WIDTH/2.0);
                worldy.at(0) = (worldz.at(0)*(FOV/worldy.at(0)))+(SCREEN_HEIGHT/2.0);
                worldx.at(1) = (worldx.at(1)*(FOV/worldy.at(1)))+(SCREEN_WIDTH/2.0);
                worldy.at(1) = (worldz.at(1)*(FOV/worldy.at(1)))+(SCREEN_HEIGHT/2.0);
                //worldx.at(2) = (worldx.at(2)*(FOV/worldy.at(2)))+(SCREEN_WIDTH/2.0);
                worldy.at(2) = (worldz.at(2)*(FOV/worldy.at(2)))+(SCREEN_HEIGHT/2.0);
                //worldx.at(3) = (worldx.at(3)*(FOV/worldy.at(3)))+(SCREEN_WIDTH/2.0);
                worldy.at(3) = (worldz.at(3)*(FOV/worldy.at(3)))+(SCREEN_HEIGHT/2.0);
                
                
                drawWall(r, worldx.at(0), worldx.at(1), worldy.at(0), worldy.at(1), worldy.at(2), worldy.at(3), walls.at(w).color, s);
            }
            
            sectors.at(s).surface *= -1;
        
        }
        sectors.at(s).d /= (sectors.at(s).wend - sectors.at(s).wstart);
    }
}

void updateFrame(SDL_Renderer* r) {
    
    if ((T.milliseconds - T.late) >= 50) {

        SDL_SetRenderDrawColor(r, 225, 225, 225, 225);
        SDL_RenderClear(r);

        draw3D(r);
        
        SDL_RenderPresent(r);

        T.late = T.milliseconds;
        
    } 

    T.milliseconds = SDL_GetTicks64();
    
}



void init() {

    // get input from data files
    ifstream infile;
    infile.open("./src/WAD/sector.WAD");
    if (infile) {

        string input;
        int data;
        int nsec = 0;
        int nwall = 0;
        while (getline(infile, input, ',')) {
            stringstream(input) >> data;
            cout << data << " ";
            loadSectors.push_back(data);
            nsec++;
            if (nsec%6 == 0)
                numSect++;
        }
        infile.close();

        infile.open("./src/WAD/wall.WAD");
        while (getline(infile, input, ',')) {
            stringstream(input) >> data;
            cout << data << " ";    
            loadWalls.push_back(data);
            if (nwall%5 == 0)
                numWalls++;
        }
        infile.close();

    }
    
    

    // initalize look up tables
    for (int i = 0; i < 360; i++) {
        Look.cosVal.at(i) = cos(DEG2RAD(i));
        Look.sinVal.at(i) = sin(DEG2RAD(i));
    }

    // player location
    P.x = 70;
    P.y = -110;
    P.z = 20;
    P.a = 0;
    P.l = 0;

    // load sectors
    int n1 = 0;
    int n2 = 0;
    for (int s = 0; s < numSect; s++) {
        sectors.at(s).wstart = loadSectors.at(n1 + 0);
        sectors.at(s).wend = loadSectors.at(n1 + 1);
        sectors.at(s).z1 = loadSectors.at(n1 + 2);
        sectors.at(s).z2 = loadSectors.at(n1 + 3);
        sectors.at(s).c1 = loadSectors.at(n1 + 4);
        sectors.at(s).c2 = loadSectors.at(n1 + 5);
        n1 += 6;
        for (int w = sectors.at(s).wstart; w < sectors.at(s).wend; w++) {
            walls.at(w).x1 = loadWalls.at(n2 + 0);
            walls.at(w).y1 = loadWalls.at(n2 + 1);
            walls.at(w).x2 = loadWalls.at(n2 + 2);
            walls.at(w).y2 = loadWalls.at(n2 + 3);
            walls.at(w).color = loadWalls.at(n2 + 4);
            n2 += 5;
        }
    }

}

void reload () {
    // clear vectors
    numSect = 0;
    numWalls = 0;
    loadSectors.clear();
    loadWalls.clear();
    sectors = vector<Sectors>(30);
    walls = vector<Walls>(30);
    

    // get data from files
    ifstream infile;
    infile.open("./src/WAD/sector.WAD");
    if (infile) {

        string input;
        int data;
        int nsec = 0;
        int nwall = 0;
        while (getline(infile, input, ',')) {
            stringstream(input) >> data;
            cout << data << " ";
            loadSectors.push_back(data);
            nsec++;
            if (nsec%6 == 0)
                numSect++;
        }
        infile.close();

        infile.open("./src/WAD/wall.WAD");
        while (getline(infile, input, ',')) {
            stringstream(input) >> data;
            cout << data << " ";
            loadWalls.push_back(data);
            if (nwall%5 == 0)
                numWalls++;
        }
        infile.close();

    }
    
    // load sectors
    int n1 = 0;
    int n2 = 0;
    for (int s = 0; s < numSect; s++) {
        sectors.at(s).wstart = loadSectors.at(n1 + 0);
        sectors.at(s).wend = loadSectors.at(n1 + 1);
        sectors.at(s).z1 = loadSectors.at(n1 + 2);
        sectors.at(s).z2 = loadSectors.at(n1 + 3);
        sectors.at(s).c1 = loadSectors.at(n1 + 4);
        sectors.at(s).c2 = loadSectors.at(n1 + 5);
        n1 += 6;
        for (int w = sectors.at(s).wstart; w < sectors.at(s).wend; w++) {
            walls.at(w).x1 = loadWalls.at(n2 + 0);
            walls.at(w).y1 = loadWalls.at(n2 + 1);
            walls.at(w).x2 = loadWalls.at(n2 + 2);
            walls.at(w).y2 = loadWalls.at(n2 + 3);
            walls.at(w).color = loadWalls.at(n2 + 4);
            n2 += 5;
        }
    }

    
}

int main(int argc, char* args[]) {

    SDL_Window* window = NULL;

    SDL_Renderer* renderer = NULL;
    
    SDL_Event event;

    //SDL_Texture* scaling = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH/20, SCREEN_HEIGHT/20);

    init();

    bool done = false;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL_Init Error: " << SDL_GetError() << endl;
    } else {
        
        window = SDL_CreateWindow("Doom-Style", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        }

        renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer == NULL) {
            cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        }
        

        //screenSurface = SDL_GetWindowSurface(window);
        
        //SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
        SDL_SetRenderDrawColor(renderer, 225, 225, 225, 225);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
       
        //SDL_UpdateWindowSurface(window); 

        // drawPixel(renderer, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 0);
        // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 225);

        // SDL_RenderDrawPoint(renderer, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
        while (!done) {

            updateFrame(renderer);
    
            // cout << T.milliseconds << endl;
            // if (T.milliseconds > 10000) {
            //     done = true;
            // }

            SDL_PollEvent(&event);
            if (event.type == SDL_QUIT) {
                cout << "Program quit" << endl;
                break;
            }
            if (event.type == SDL_KEYDOWN) {
                
                if (event.key.keysym.sym == SDLK_LEFT) {
                    //cout << "Left is being pressed down" << endl;
                    P.a -=4;
                    if (P.a < 0) {
                        P.a += 360;
                    }
                }

                if (event.key.keysym.sym == SDLK_RIGHT) {
                    //cout << "Right is being pressed down" << endl;
                    P.a += 4;
                    if (P.a > 359) {
                        P.a -= 360;
                    }
                }

                if (event.key.keysym.sym == SDLK_UP) {
                    //cout << "Up is being pressed down" << endl;
                    P.l -= 1;
                }

                if (event.key.keysym.sym == SDLK_DOWN) {
                    //cout << "Down is being pressed down" << endl;
                    P.l += 1;
                }

                int dx = Look.sinVal.at(P.a) * 10.0; //cout << "dx: " << dx << endl;
                int dy = Look.cosVal.at(P.a) * 10.0; //cout << "dy: " << dy << endl;
                
                if (event.key.keysym.sym == SDLK_a) {
                    //cout << "A is being pressed down" << endl;
                    P.x -= dy;
                    P.y += dx;    
                }

                if (event.key.keysym.sym == SDLK_d) {
                    //cout << "D is being pressed down" << endl;
                    P.x += dy;
                    P.y -= dx;
                }
                
                if (event.key.keysym.sym == SDLK_w) {
                    //cout << "W is being pressed down" << endl;
                    P.x += dx;
                    P.y += dy;
                }

                if (event.key.keysym.sym == SDLK_s) {
                    //cout << "S is being pressed down" << endl;
                    P.x -= dx;
                    P.y -= dy;
                }

                if (event.key.keysym.sym == SDLK_q) {
                    P.z -= 4;
                }

                if (event.key.keysym.sym == SDLK_e) {
                    P.z += 4;
                }

                if (event.key.keysym.sym == SDLK_m) {
                    reload();
                }

            } 

        }
        
        
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

