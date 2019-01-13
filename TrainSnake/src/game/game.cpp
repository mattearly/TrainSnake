#include "game.h"
#include <iostream>
#include <assert.h>
game::game() {}


game::~game() {

  clean();
}

void game::run() {
  level = L1;
  startingCabsRequiredToAdvance = 1;
  init_systems();
  load_assets();
  resetCabsCollected();
  main_loop();
  //clean();
}

int game::getRandomNumber(const int &start, const int &end) {
  // initialize the random number generator with time-dependent seed
  static long long int timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch()
    .count();
  static std::seed_seq ss{ uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };
  // initialize a uniform distribution between 0 and 1
  std::uniform_int_distribution<int> unif(start, end);

  static std::mt19937 mgen(ss);

  return unif(mgen);
}

void game::handleKeys() {
  while (SDL_PollEvent(&event) != 0) {
    if (event.type == SDL_QUIT) {
      quit = true;
    }

    if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
      case SDLK_a:
      case SDLK_LEFT:
        moveLeft();
        break;
      case SDLK_d:
      case SDLK_RIGHT:
        moveRight();
        break;
      case SDLK_w:
      case SDLK_UP:
        moveUp();
        break;
      case SDLK_s:
      case SDLK_DOWN:
        moveDown();
        break;
      default:
        break;
      }
    }
  }
}

void game::init_systems() {
  /* initialize core windows graphics */
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_CreateWindowAndRenderer(800, 640, SDL_WINDOW_SHOWN, &window, &renderer);
  surface = SDL_GetWindowSurface(window);

  /* initialize sound */
  int audio_rate = 22050;
  Uint16 audio_format = AUDIO_S16SYS;
  int audio_channels = 2;
  int audio_buffers = 4096;
  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels,
    audio_buffers) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't init audio: %s",
      Mix_GetError());
    exit(-3);
  }
}

void game::load_assets() {
  /* load png assets */
  for (int i = 0; i < 11; i++) {
    std::stringstream sstreamlevels("../TrainSnake/res/images/levels/",
      std::ios_base::app |
      std::ios_base::out);
    sstreamlevels << i;
    sstreamlevels << ".png";
    std::stringstream sstreamtrains("../TrainSnake/res/images/trains/",
      std::ios_base::app |
      std::ios_base::out);
    sstreamtrains << i;
    sstreamtrains << ".png";
    std::stringstream sstreamcabs("../TrainSnake/res/images/cabs/",
      std::ios_base::app |
      std::ios_base::out);
    sstreamcabs << i;
    sstreamcabs << ".png";

    std::string level_path = sstreamlevels.str();
    std::string train_path = sstreamtrains.str();
    std::string cab_path = sstreamcabs.str();

    levels[i] = load_pic(level_path, surface);
    trains[i] = load_pic(train_path, surface);
    cabs[i] = load_pic(cab_path, surface);

  }

  /* load sound assets */
  sounds[0] = Mix_LoadWAV("../TrainSnake/res/sounds/thomas_the_train_theme_song.wav");
  sounds[1] = Mix_LoadWAV("../TrainSnake/res/sounds/today.wav");
  sounds[2] = Mix_LoadWAV("../TrainSnake/res/sounds/oh_my.wav");
  sounds[3] = Mix_LoadWAV("../TrainSnake/res/sounds/music_clip_01.wav");
  sounds[4] = Mix_LoadWAV("../TrainSnake/res/sounds/train_coupling_impact.wav");
  for (auto &sound : sounds) {
    if (sound == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
        "Couldn't load audio: %s",
        Mix_GetError());
    }
  }
}

SDL_Surface * game::load_pic(const std::string & path, SDL_Surface * srfc) {

  SDL_Surface *final_surface = nullptr;

  SDL_Surface *loaded_surface = IMG_Load(path.c_str());

  if (loaded_surface == nullptr) {
    printf("unable to load %s", path.c_str());
  } else {
    final_surface = SDL_ConvertSurface(loaded_surface, srfc->format, 0);
    if (final_surface == nullptr) {
      printf("unable to optimize image %s", path.c_str());
    }
    SDL_FreeSurface(loaded_surface);
  }
  return final_surface;
}

void game::main_loop() {
  int animation_accumulator = 1;
  SetTrainLocationToDefaultStart();
  resetCabsCollected();
  Mix_PlayChannel(-1, sounds[3], 0); // "start tune"
  while (!quit) {
    frameStart = SDL_GetTicks();

    handleKeys();

    /* game logic - update trains, cabs, collectibles, and game progression */
    if (animation_accumulator > levelSpeed()) animation_accumulator = levelSpeed();

    if (animation_accumulator % levelSpeed() == 0) {
      moveTrain();
      collectPickups();
      animation_accumulator = 1;
    }

    drawChanges();

    frameTime = SDL_GetTicks() - frameStart;
    animation_accumulator += frameTime;
    if (frameDelay > frameTime) {
      SDL_Delay(static_cast<Uint32>(frameDelay - frameTime));
    }
  }
}

void game::clean() {
  for (int i = 0; i < 11; i++) {
    SDL_FreeSurface(levels[i]);
    SDL_FreeSurface(trains[i]);
    SDL_FreeSurface(cabs[i]);
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void game::moveLeft() {
  if (movingDirection != RIGHT)
    movingDirection = LEFT;
}

void game::moveRight() {
  if (movingDirection != LEFT)
    movingDirection = RIGHT;
}

void game::moveUp() {
  if (movingDirection != DOWN)
    movingDirection = UP;
}

void game::moveDown() {
  if (movingDirection != UP)
    movingDirection = DOWN;
}

int game::winCondition() {
  return startingCabsRequiredToAdvance + (int)level;
  //return 2;
}

void game::moveTrain() {

  /* train history */
  for (int i = winCondition(); i > 0; i--) {
    //if (i == 0) SDL_Log("Win condition == 0 pickups! Should not ever be zero here!");
    trainCabs[i] = trainCabs[i - 1];
  }
  trainCabs[0] = trainHeadLocRect;

  /* move the train head */
  switch (movingDirection) {
  case HODL:
    break;
  case LEFT:
    trainHeadLocRect.x -= GRIDSIZE;
    break;
  case RIGHT:
    trainHeadLocRect.x += GRIDSIZE;
    break;
  case UP:
    trainHeadLocRect.y -= GRIDSIZE;
    break;
  case DOWN:
    trainHeadLocRect.y += GRIDSIZE;
    break;
  }

  /*don't allow train off the screen */
  if (trainHeadLocRect.x >= SCREENWIDTH) {
    trainHeadLocRect.x -= GRIDSIZE;
  }
  if (trainHeadLocRect.x <= 0) {
    trainHeadLocRect.x = 0;
  }
  if (trainHeadLocRect.y >= SCREENHEIGHT) {
    trainHeadLocRect.y -= GRIDSIZE;
  }
  if (trainHeadLocRect.y <= 0) {
    trainHeadLocRect.y = 0;
  }
}

void game::spawnCollectibles() {
  if (collectiblesSpawned < 4) {
    collectible collectible1{ 0, 0, GRIDSIZE, GRIDSIZE };

    int x = 0;
    int y = 0;

    while (collectiblesSpawned < 4) {

      do {
        x = getRandomNumber(0, 24);
        y = getRandomNumber(0, 19);


        /*make sure x and y don't spawn on the train head or a previous cab */
      } while (x != trainHeadLocRect.x && y != trainHeadLocRect.y &&
        equalToPreviousCollectible(x, y));

      collectible1.collectibleRect.x = x * GRIDSIZE;
      collectible1.collectibleRect.y = y * GRIDSIZE;
      collectibles.push_back(collectible1);
      collectiblesSpawned++;

    }
  }
}

void game::collectPickups() {
  int to_remove = 0;

  for (auto &c : collectibles) {

    if (c.collectibleRect.x == trainHeadLocRect.x
      && c.collectibleRect.y == trainHeadLocRect.y) {
      Mix_PlayChannel(-1, sounds[4], 0);  // train coupling impact sound
      collectibles.erase(collectibles.begin() + to_remove,
        collectibles.begin() + to_remove + 1);
      cabsCollected++;
      SDL_Log("Cabs Collected: %i", cabsCollected);
      collectiblesSpawned--;
      break;
    }
    to_remove++; /*don't increment as we pick one up */
  }

  spawnCollectibles();

  handleConditions();
}

void game::resetCabsCollected() {
  cabsCollected = 0;
  trainCabs.clear();
  for (int i = 0; i < winCondition(); i++) {
    SDL_Rect new_cab = { 0, 0, GRIDSIZE, GRIDSIZE };
    trainCabs.push_back(new_cab);
  }
  collectibles.clear();
  collectiblesSpawned = 0;
  spawnCollectibles();
}

int game::levelSpeed() {
  /* lower is faster */
  if (((int)level * 2) > lowestspeed) {
    return 1;
  }
  return lowestspeed - (level * 2);
}

void game::drawChanges() {
  SDL_RenderClear(renderer);

  /* drawing latest changes */
  // background
  SDL_BlitSurface(levels[level], &screenSizeRect, surface, &screenSizeRect);

  // collectible cabs to pick up
  for (auto &c : collectibles) {
    SDL_BlitSurface(cabs[level], &snakeTrainPieceSize, surface,
      &c.collectibleRect);
  }

  // extra lengths of the train snake
  for (int i = 0; i < cabsCollected; i++)
    SDL_BlitSurface(cabs[level], &snakeTrainPieceSize, surface, &trainCabs[i]);

  // train head
  SDL_BlitSurface(trains[level], &snakeTrainPieceSize, surface, &trainHeadLocRect);

  SDL_UpdateWindowSurface(window);
}

void game::handleConditions() {
  /* advance to next level if score is good enough */
  if (cabsCollected >= winCondition()) {

    if (level == L10) {
      SDL_Log("You won the game!");
      Mix_PlayChannel(-1, sounds[0], 0); //thomas the train theme song
      SDL_Delay(35000);
      Mix_HaltChannel(-1);

      quit = true;
      return;
    }

    /* advance level and reset level stuff */
    int next_level = level + 1;
    level = static_cast<LEVEL>(next_level);

    SetTrainLocationToDefaultStart();

    resetCabsCollected();

    Mix_PlayChannel(1, sounds[2], 0); // "oh my"

  }
}

bool game::equalToPreviousCollectible(const int & x, const int & y) {
  for (auto &cb : collectibles) {
    if (cb.collectibleRect.x == x && cb.collectibleRect.y == y) return true;
  }
  return false;
}

void game::SetTrainLocationToDefaultStart() {
  trainHeadLocRect.x = 0;
  trainHeadLocRect.y = 64;
  movingDirection = RIGHT;
}

