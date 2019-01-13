#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <chrono>
static constexpr int GRIDSIZE = 32;
static constexpr int SCREENWIDTH(GRIDSIZE * 25), SCREENHEIGHT(GRIDSIZE * 20);
enum MOVINGDIRECTION : std::size_t {
  HODL, LEFT, RIGHT, UP, DOWN
};
struct collectible {
  SDL_Rect collectibleRect = { 0, 0, GRIDSIZE, GRIDSIZE };
};
enum LEVEL : std::size_t {
  BONUS,
  L1, L2, L3, L4, L5,
  L6, L7, L8, L9, L10
};
class game {
public:
  game();
  ~game();

  void run();

private:

  int getRandomNumber(const int& start, const int& end);

  void handleKeys();

  void init_systems();

  void load_assets();

  SDL_Surface *load_pic(const std::string &path, SDL_Surface *srfc);

  void main_loop();

  void clean();

  //void clean();

  void moveLeft();

  void moveRight();

  void moveUp();

  void moveDown();

  MOVINGDIRECTION movingDirection;

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *surface;
  SDL_Event event;

  SDL_Rect screenSizeRect = { 0, 0, SCREENWIDTH, SCREENHEIGHT };
  SDL_Surface *levels[11];

  const SDL_Rect snakeTrainPieceSize = { 0, 0, GRIDSIZE, GRIDSIZE };
  SDL_Rect trainHeadLocRect = { 0, 64, GRIDSIZE, GRIDSIZE };
  SDL_Surface *trains[11];
  SDL_Surface *cabs[11];
  Mix_Chunk *sounds[5];

  int cabsCollected;

  std::vector<SDL_Rect> trainCabs;

  bool quit = false;
  int lowestspeed = 26;

  Uint32 frameStart = 0;
  int frameTime = 0;
  const int FPS = 60;
  const int frameDelay = 1000 / FPS; //16ms @ 60fps

  LEVEL level;

  int startingCabsRequiredToAdvance;  //plus level

  int winCondition();

  void moveTrain();

  int collectiblesSpawned;

  void spawnCollectibles();

  std::vector<collectible> collectibles;

  void collectPickups();

  void resetCabsCollected();

  int levelSpeed();

  void drawChanges();

  void handleConditions();

  bool equalToPreviousCollectible(const int &x, const int &y);

  void SetTrainLocationToDefaultStart();

};

