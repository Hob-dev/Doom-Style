#define SDL_MAIN_HANDLED

#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>   
#include <vector>
#include <filesystem>
using namespace std;

const int res = 1;
const int SH = 605;
const int GSW = 600; // Graph screen width
const int TSW = 200; // Toolbar screen width
const int pixelScale = 5;
const int sGSW = GSW/pixelScale; // Scaled graph screen width
const int sSH = SH/pixelScale; // Scaled screen height
const int gridSize = 10;
int numSects = 0;
int current = 0; // current wall


struct Time {
    int milliseconds;
    int late;
} T;

struct Sector {
    int wstart, wend; // dictate which walls are in a sector
    int z1 = 0;
    int z2 = 40; // height level for each coordinate - specify this in toolbar
    int c1 = 0;
    int c2 = 1; // color value for top and bottom surfaces - specify this in toolbar
}; vector<Sector> sectors = vector<Sector>(30);

struct Wall {
    int x1, y1; // 
    int x2, y2;
    int color;
}; vector<Wall> walls = vector<Wall>(30);

// struct Button {

// }

void drawPixel(SDL_Renderer*& r, int x, int y, int color) { // only graph

    y *= -1; // flip y coordinate so we have standard coordinate system
    x += sGSW/2; // Set the origin to the middle of the program
    y +=  sSH/2;
    
    
    vector<int> rgb(4);
    switch (color) {
        case 1:
            rgb = {83, 92, 145, 255};
            break;
        case 2:
            rgb = {255, 255, 255, 100};
        default:
            rgb = {255, 255, 255, 255};
            break;
    }

    SDL_SetRenderDrawColor(r, rgb.at(0), rgb.at(1), rgb.at(2), rgb.at(3));
    SDL_RenderDrawPoint(r, x, y);

}

void drawLine(SDL_Renderer*& r, int x1, int y1, int x2, int y2, int color) {
 
    y1 *= -1; // flip y coordinate so we have standard coordinate system
    x1 += sGSW/2; // Set the origin to the middle of the program
    y1 += sSH/2;
    y2 *= -1;
    x2 += sGSW/2;
    y2 += sSH/2;
    

    vector<int> rgb(4);
    switch (color) {
        case 1:
            rgb = {83, 92, 145, 255};
            break;
        case 2:
            rgb = {255, 255, 255, 100};
        default:
            rgb = {255, 255, 255, 255};
            break;
    }
    
    SDL_SetRenderDrawColor(r, rgb.at(0), rgb.at(1), rgb.at(2), rgb.at(3));   
    SDL_RenderDrawLine(r, x1, y1, x2, y2);
    
}

void drawGrid(SDL_Renderer*& r) {

    for (int x = -GSW/2; x <= GSW/2; x += gridSize) {

        drawLine(r, x, -SH/2, x, SH/2, 1);

    }

    for (int y = -SH/2+2; y <= SH/2; y += gridSize) {

        drawLine(r, -sGSW/2, y, sGSW/2, y, 1);

    }

}

void drawToolbar(SDL_Renderer*& r, SDL_Texture*& toolbar, SDL_Rect* barViewPort) {

    SDL_RenderSetViewport(r, barViewPort);
    SDL_SetRenderTarget(r, toolbar);
    SDL_SetRenderDrawColor(r, 27, 26, 85, 255);
    SDL_RenderClear(r);



    SDL_SetRenderTarget(r, NULL);
    SDL_RenderCopy(r, toolbar, NULL, NULL);

} 

void drawGraph(SDL_Renderer*& r, SDL_Texture*& graph, SDL_Rect* graphViewPort) {
    
    SDL_RenderSetViewport(r, graphViewPort);
    SDL_SetRenderTarget(r, graph);
    SDL_SetRenderDrawColor(r, 7, 15, 43, 255);
    SDL_RenderClear(r);
        
    drawGrid(r);

}

void draw2D(SDL_Renderer* r) {

    for (int s = 0; s <= numSects; s++) {
    
        for (int w = sectors.at(s).wstart; w < sectors.at(s).wend; w++) {
            drawLine(r, walls.at(w).x1, walls.at(w).y1, walls.at(w).x2, walls.at(w).y2, 0);
        }
    
    }

}

void load() {
    
    ofstream sectorfile("./src/WAD/sector.WAD");
    ofstream wallfile("./src/WAD/wall.WAD");

    if (sectorfile && wallfile) {

        for (int i = 0; i < numSects; i++) {
            Sector s = sectors.at(i);
            sectorfile << s.wstart << ", " << s.wend << ", " << s.z1 << ", " << s.z2 << ", " << s.c1 << ", " << s.c2 << ", " << endl;
        }
        sectorfile.close();

        for (int i = 0; i < current; i++) {
            Wall w = walls.at(i);
            wallfile << w.x1 << ", " << w.y1 << ", " << w.x2 << ", " << w.y2 << ", " << w.color << ", " << endl;
        }
        wallfile.close();

    }


}

void colorMessage(int currentColor) {
    switch(currentColor) {
                    case 0:
                        cout << "Yellow" << endl;
                        break;
                    case 1:
                        cout << "Green" << endl;
                        break;
                    case 2:
                        cout << "Blue" << endl;
                        break;
                    case 3:
                        cout << "Black" << endl;
                        break;
                    case 4:
                        cout << "Red" << endl;
                        break;
                    case 5:
                        cout << "Magenta" << endl;
                        break;
                    case 6:
                        cout << "Cyan" << endl;
                        break;
    }
}

int main(int argc, char* args[]) {

    SDL_Window* window = NULL; // graph
    
    SDL_Renderer* renderer = NULL; // graph
    
    SDL_Event event;

    SDL_Point cursor;

    SDL_Point first;
    // SDL_Texture graph;

    int done = 0;
    int clicks = 0;
    bool canExit = true;
    int currentColor = 0;
    sectors.at(numSects).wstart = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL Error: " << SDL_GetError() << endl;
    } else {

        window = SDL_CreateWindow("Doom-Style Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GSW + TSW, SH, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        }

        renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer == NULL) {
            cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        }

        SDL_Rect graphViewPort;
        graphViewPort.x = 0;
        graphViewPort.y = 0;
        graphViewPort.w = GSW+(1*pixelScale);
        graphViewPort.h = SH;

        SDL_Rect barViewPort;
        barViewPort.x = GSW+(1*pixelScale);
        barViewPort.y = 0;
        barViewPort.w = TSW;
        barViewPort.h = SH;


        SDL_Texture* graph = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, sGSW+1, sSH);
        SDL_Surface* tTexture = SDL_LoadBMP("./src/Images/Toolbar.bmp");
        
        SDL_Texture* toolbar = SDL_CreateTextureFromSurface(renderer, tTexture);

        while (!done) {
            

            if ((T.milliseconds - T.late) >= 16.67) { 

                drawToolbar(renderer, toolbar, &barViewPort);
                
                drawGraph(renderer, graph, &graphViewPort);

                draw2D(renderer);

                SDL_GetMouseState(&cursor.x, &cursor.y);
                cursor.x = (cursor.x/pixelScale) - (sGSW/2); // for cursor, reverse coordination abstraction
                cursor.y = -((cursor.y/pixelScale) - (sSH/2));
            
                if (cursor.x <= sGSW/2) {

                
                    if (clicks == 0) {
                        first.x = cursor.x;
                        first.y = cursor.y;
                        drawPixel(renderer, cursor.x, cursor.y, 0); 
                    
                    }
                    if (clicks == 1) {
                        
                        drawLine(renderer, first.x, first.y, cursor.x, cursor.y, 0);
                    
                    }
                    if (clicks == 2) {
                        canExit = false;
                        walls.at(current).x1 = first.x;
                        walls.at(current).y1 = first.y;
                        walls.at(current).x2 = cursor.x;
                        walls.at(current).y2 = cursor.y;
                        walls.at(current).color = currentColor;
                        sectors.at(numSects).wend = current + 1;
                        cout << "Last Wall Beginning: " << walls.at(current).x1 << " " << walls.at(current).y1 << endl;
                        cout << "Last Wall End: " << walls.at(current).x2 << " " << walls.at(current).y2 << endl;
                        if ( walls.at(sectors.at(numSects).wstart).x1 == walls.at(current).x2 && walls.at(sectors.at(numSects).wstart).y1 == walls.at(current).y2 ) {
                            
                            canExit = true;
                            numSects++;
                            current++;
                            cout << "Current sectors: " << numSects << endl;
                            cout << "Current walls: " << current << endl;
                            sectors.at(numSects).wstart = current;
                            clicks = 0;
                            
                        } else {
                            first.x = cursor.x;
                            first.y = cursor.y;
                            current++;
                            cout << "Current sectors: " << numSects << endl;
                            cout << "Current walls: " << current << endl;
                            clicks = 1;
                        }


                    }    

                }

                
                SDL_SetRenderTarget(renderer, NULL);
                SDL_RenderCopy(renderer, graph, NULL, NULL);
                SDL_RenderPresent(renderer);
                        

                T.late = T.milliseconds;
            
            }

            T.milliseconds = SDL_GetTicks64();


            SDL_PollEvent(&event);
            if (event.type == SDL_QUIT) {
                cout << "Program Quit" << endl;
                break;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == 1) {
                    if (cursor.x <= sGSW/2) {
                        clicks++;
                        cout << clicks << endl;
                        cout << "Position: " << cursor.x << " " << cursor.y << endl;
                    }
                }
                if (event.button.button == 3) {
                    if (canExit)
                        clicks = 0;
                }
            }

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_m) {
                cout << "Load initiated..." << endl;
                load();
                cout << "Load Complete" << endl;
            }

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP) {
                if (numSects > 0) {
                    sectors.at(numSects-1).z2++;
                    cout << "Current Height: " << sectors.at(numSects-1).z2 << endl;
                }
            }

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN) {
                if (numSects > 0) {
                    if (sectors.at(numSects-1).z2 > sectors.at(numSects-1).z1)
                        sectors.at(numSects-1).z2--;
                    cout << "Current Height: " << sectors.at(numSects-1).z2 << endl;
                }
            }
            
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT) {
                currentColor--;
                if (currentColor < 0)
                    currentColor = 6;
                cout << "Current Color: ";
                colorMessage(currentColor);
            }



            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT) {
                currentColor++;
                if (currentColor > 6)
                    currentColor = 0;
                cout << "Current color: ";
                colorMessage(currentColor);

            }

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_1) {
                if (numSects > 0) {
                    sectors.at(numSects-1).c1 = currentColor;
                    cout << "Floor of last sector is ";
                    colorMessage(currentColor);
                    
                }
            }

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_2) {
                if (numSects > 0) {
                    sectors.at(numSects-1).c2 = currentColor;
                    cout << "Roof of last sector is ";
                    colorMessage(currentColor);
                    
                }
            }

        }       
    }

    SDL_DestroyRenderer(renderer);
    
    
    SDL_DestroyWindow(window);
   

    SDL_Quit();
    return 0;
}