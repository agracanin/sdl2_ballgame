#include <iostream>
#include <cstdlib>
#include <ctime>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL2_framerate.h>

using namespace std;

// Constants for window dimensions and framecount for animations
const int WIDTH = 500;
const int HEIGHT = 500;
const int FCNT = 7;

int main(int argc, char *argv[])
{
    srand(time(nullptr)); // Seeding rand
    SDL_Window *w = nullptr;

    // Initializing SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
    {
        cerr << "SDL_Init() fail... " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    // Window and renderer creation
    w = SDL_CreateWindow("Prog 4: gracana2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *rend = SDL_CreateRenderer(w, -1, 0);

    // Loading BG image and creating the texture
    SDL_Surface *bg_s = SDL_LoadBMP("background.bmp");
    SDL_Texture *bg = SDL_CreateTextureFromSurface(rend, bg_s);
    SDL_FreeSurface(bg_s);

    // Loading all sprites and setting dimensions
    SDL_Surface *ball_s = SDL_LoadBMP("ball.bmp");
    int BW = ball_s->w;
    int BH = ball_s->h;
    SDL_Texture *tball = SDL_CreateTextureFromSurface(rend, ball_s);
    SDL_FreeSurface(ball_s);

    SDL_Surface *sprite_s = SDL_LoadBMP("standing.bmp");
    int SW = sprite_s->w / FCNT; // finding width of single frame
    int SH = sprite_s->h;
    SDL_Texture *sprite = SDL_CreateTextureFromSurface(rend, sprite_s);
    SDL_FreeSurface(sprite_s);

    SDL_Surface *running_s = SDL_LoadBMP("running.bmp");
    SDL_Texture *running = SDL_CreateTextureFromSurface(rend, running_s);
    SDL_FreeSurface(running_s);

    // Enum for state management of player sprite
    enum SpriteState
    {
        STANDING,
        RUNNING
    };
    SpriteState currentState = STANDING; // initial state
    bool facingRight = true;             // boolean for direction

    // Setting to 30 FPS
    FPSmanager fps;
    SDL_initFramerate(&fps);
    SDL_setFramerate(&fps, 30);
    int frame = 0;

    // Ball variables (position and velocity)
    float bx = (WIDTH - BW) / 2;
    float by = 0;
    float vy = 0;
    float vx = 0;

    // Player sprite variables
    int spriteX = 0;
    int spriteY = HEIGHT - SH;

    // Game scoring and target position (mouse pos)
    int currentScore = 0;
    int highScore = 0;
    int targetX = 0;

    // Text and font, realized I didn't need to use this to display text..
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    SDL_Color textColor = {0, 0, 0, 255};

    SDL_Event e;
    bool quit = false;
    while (!quit)
    {
        SDL_RenderClear(rend);
        // Render background
        SDL_Rect bgRect = {0, 0, WIDTH, HEIGHT};
        if (bg) // A check if the background texture is not NULL
        {
            SDL_RenderCopy(rend, bg, NULL, &bgRect);
        }
        else
        {
            // If background image has issues, set background to white
            SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
            SDL_RenderFillRect(rend, &bgRect);
        }
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_MOUSEMOTION:
            {
                // Set the target X position based on the mouse position adjusted by half of the sprite width
                targetX = e.motion.x - SW / 2;
                // Determine the sprite facing direction and state based on the target X position
                if (spriteX < targetX) // If sprite is left of the target
                {
                    facingRight = true;
                    currentState = RUNNING;
                }
                else if (spriteX > targetX) // If sprite is right of the target
                {
                    facingRight = false;
                    currentState = RUNNING;
                }
                else // Sprite is at the target
                {
                    currentState = STANDING;
                }

                // Outside boundary prevention
                if (spriteX < 0)
                    spriteX = 0;
                else if (spriteX + SW > WIDTH)
                    spriteX = WIDTH - SW;
            }
            break;
            // Handling ball hit
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    // Rectangular areas for the ball and the player for collision detection
                    SDL_Rect ballRect = {(int)bx, (int)by, BW, BH};
                    SDL_Rect spriteRect = {spriteX, spriteY, SW, SH};

                    // Center positions for the ball and player
                    float ballCenterX = bx + BW / 2;
                    float ballCenterY = by + BH / 2;
                    float spriteCenterX = spriteX + SW / 2;
                    float spriteCenterY = spriteY + SH / 2;

                    // Check for intersection area
                    if (SDL_HasIntersection(&ballRect, &spriteRect))
                    {
                        // Calculate the vector from the player to the ball
                        float vectorX = (bx + BW / 2) - (spriteX + SW / 2);
                        float vectorY = (by + BH / 2) - (spriteY + SH / 2);

                        // Vector magnitude
                        float magnitude = sqrt(vectorX * vectorX + vectorY * vectorY);

                        // Normalize the vector dividing by magnitude
                        vectorX /= magnitude;
                        vectorY /= magnitude;

                        float tapStrength = 25.0; // Strength of tap effect

                        // Applying the tap strength to the X and Y velocity of the ball
                        vx = tapStrength * vectorX;
                        vy = tapStrength * -abs(vectorY); // Making sure ball only goes up

                        // Increment score, change high score if needed
                        currentScore++;
                        if (currentScore > highScore)
                        {
                            highScore = currentScore;
                        }
                    }
                }
                break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                case SDLK_SPACE: // Lob functionality
                {
                    // Get the areas
                    SDL_Rect ballRect = {(int)bx, (int)by, BW, BH};
                    SDL_Rect spriteRect = {spriteX, spriteY, SW, SH};
                    // Check for intersection
                    if (SDL_HasIntersection(&ballRect, &spriteRect))
                    {
                        vx = (rand() % 6) - 3; // Random small horizontal velocity
                        vy = -35;              // Strong upward velocity
                        currentScore = 0;      // Reset score
                    }
                }
                break;
                default:
                    break;
                }
            }
        }

        /*
         * gravity increases velocity.  falling down to our eyes is actually
         * increasing the Y position (NOT making it smaller)
         */
        vy++;
        by += vy;

        /*
         * Dampen the sideways velocity all the time, but just a bit...
         * give the feel of air drag
         */
        vx *= 0.98;
        bx += vx;

        /*
         * Bounce off the bottom of the screen, but also let the floor
         * "absorb" some energy by losing 30% of Y velocity.  Also, constrain
         * the ball so it's not off the bottom of the screen ever.
         */
        if (by + BH > HEIGHT - 1)
        {
            vy = -(vy * 0.7);
            by = HEIGHT - 1 - BH;
            if (fabs(vy) < 2)
            {
                vx *= 0.9;
            }
            currentScore = 0; // adding score reset on ground touch
        }

        /*
         * Don't go off the right or of the screen.  Bounces absorb energy
         */
        if (bx + BW > WIDTH - 1)
        {
            vx = -(vx * 0.6);
            bx = WIDTH - 1 - BW;
        }
        if (bx <= 0)
        {
            vx = -(vx * 0.6);
            bx = 0;
        }

        float easingFactor = 0.20; // Define an easing factor used to make the player ease into position

        /*  Updating the players x-position:
         *  This moves the player closer to the target x-position
         *  with the distance moved being a percentage (easingFactor) of the distance remaining
         */
        spriteX += (targetX - spriteX) * easingFactor;

        /*  Used to determine if close enough to the target position
         *  if the distance between the sprite and target is less than 10.0f set the state to STANDING
         *  this also makes small movements not change the standing animation
         */
        if (fabs(spriteX - targetX) < 10.0f)
        {
            currentState = STANDING;
        }

        // Formatting, creating, rendering score text
        char scoreText[100];
        sprintf(scoreText, "Current Score: %d  High Score: %d", currentScore, highScore);
        SDL_Surface *textSurface = TTF_RenderText_Solid(font, scoreText, textColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(rend, textSurface);
        SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
        SDL_FreeSurface(textSurface);
        SDL_RenderCopy(rend, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);

        SDL_Rect ballRect = {(int)round(bx), (int)round(by), BW, BH}; // Rectangle of balls position and size
        SDL_RenderCopy(rend, tball, nullptr, &ballRect);              // Copying the ball to the renderer

        /*
        Choosing sprite to render based on state
        Standing sprite sheet used for standing
        Running sprite sheet used for running
        */
        SDL_Rect spriteRect;
        SDL_Rect dstRect{spriteX, spriteY, SW, SH};
        if (currentState == STANDING)
        {
            spriteRect = {(frame / 3 % FCNT) * SW, 0, SW, SH};
            SDL_RenderCopy(rend, sprite, &spriteRect, &dstRect);
        }
        else if (currentState == RUNNING)
        {
            int row = facingRight ? 0 : 1; // Choose sprite row; 0 for running right, 1 for running left
            spriteRect = {(frame / 3 % FCNT) * SW, row * SH, SW, SH};
            SDL_RenderCopy(rend, running, &spriteRect, &dstRect);
        }
        SDL_RenderPresent(rend);
        SDL_framerateDelay(&fps);
        frame++; // frame variable to store 30fps
    }

    // Clean-up
    TTF_CloseFont(font);
    SDL_DestroyTexture(tball);
    SDL_DestroyTexture(sprite);
    SDL_DestroyTexture(bg);
    SDL_DestroyTexture(running);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(w);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}