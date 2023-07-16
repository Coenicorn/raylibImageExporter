#include <stdio.h>
#include <math.h>

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

// #define DEBUG

#define IMGNAMEBUFSIZE 15

int main(void)
{
#ifdef DEBUG
    SetTraceLogLevel(LOG_DEBUG);
#else
    SetTraceLogLevel(LOG_NONE);
#endif

    InitWindow(500, 500, "Raylib Image as Code Exporter");
    SetTargetFPS(60);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetExitKey(0);

    GuiSetStyle(TEXTBOX, TEXT_WRAP_MODE, 2);

    bool hasExited = false;
    bool showExitDialogue = false;
    Rectangle exitDialogueBounds;
    {
        int t_height = 100, t_width = 200;

        exitDialogueBounds.x = GetScreenWidth() / 2 - t_width / 2;
        exitDialogueBounds.y = GetScreenHeight() / 2 - t_height / 2;
        exitDialogueBounds.width = t_width;
        exitDialogueBounds.height = t_height;
    }

    bool validFileDropped = false;
    int validFileShakeTimeMax = 10;
    int validFileShakeTime = 0;

    bool hasImage = 0;
    Image droppedImage;
    Texture droppedImageTexture;
    char imgName[IMGNAMEBUFSIZE] = "\0";
    bool isSavedToDisk = false;

    bool hasClickedSave = false;

    while (!hasExited)
    {
        // trigger exit dialogue
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q) || WindowShouldClose()) showExitDialogue = !showExitDialogue;

        if (IsFileDropped())
        {
            // reset 'saved!' text box
            isSavedToDisk = false;

            FilePathList files = LoadDroppedFiles();

            TraceLog(LOG_DEBUG, "file count: %d", files.count);

            if (files.count != 1) goto ERRFILE;

            char *path = files.paths[0];

            const char *fileExtension = GetFileExtension(path);

            if (strcmp(fileExtension, ".png")) goto ERRFILE;

            if (hasImage)
            {
                UnloadTexture(droppedImageTexture);
                UnloadImage(droppedImage);
            }

            droppedImage = LoadImage(path);
            hasImage = true;

            droppedImageTexture = LoadTextureFromImage(droppedImage);

            goto NOERR;

            ERRFILE:

            validFileShakeTime = validFileShakeTimeMax;

            NOERR:

            UnloadDroppedFiles(files);
        }

        BeginDrawing();

            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));


#ifdef DEBUG
            if (IsKeyPressed(KEY_E)) validFileShakeTime = validFileShakeTimeMax;
#endif

            if (!hasImage)
            {
                float fileDropTextXOffset = 0.f;
                Color fileDropTextTint = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
                if (validFileShakeTime > 0)
                {
                    fileDropTextTint = RED;
                    fileDropTextXOffset = sin(validFileShakeTime * 2) * 2;
                    validFileShakeTime--;
                }
                // TraceLog(LOG_DEBUG, "shake time: %d, has image: %d", validFileShakeTime, hasImage);
            
                Rectangle dropTextBounds = (Rectangle){ GetScreenWidth() / 2 - 70 + fileDropTextXOffset, GetScreenHeight() / 2 - 10, TEXT_SPACING * 21, TEXT_SIZE * 1 };

                GuiDrawText("Drop image files here", dropTextBounds, TEXT_ALIGN_LEFT, fileDropTextTint);

                GuiDrawText("This program saves a dropped '.png' to the current", (Rectangle){ GetScreenWidth() / 2 - 150, TEXT_SIZE, 300, TEXT_SIZE }, TEXT_ALIGN_LEFT, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
                GuiDrawText("working directory of the program with a specified name.", (Rectangle){ GetScreenWidth() / 2 - 150, TEXT_SIZE * 2, 300, TEXT_SIZE }, TEXT_ALIGN_LEFT, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
            
            
            }
            
            if (hasImage)
            {
                Rectangle imageDrawBounds = (Rectangle){ GetScreenWidth() / 2 - 100, 100, 200, 200 };

                DrawTexturePro(droppedImageTexture, (Rectangle){ 0, 0, droppedImageTexture.width, droppedImageTexture.height }, imageDrawBounds, (Vector2){ 0, 0 }, 0, WHITE);

                int saveButtenReturn = GuiButton((Rectangle){ GetScreenWidth() / 2 - 100, imageDrawBounds.y + imageDrawBounds.height + 10, TEXT_SPACING * 13, TEXT_SIZE * 2 }, "save image");
                
                if (!hasClickedSave) hasClickedSave = saveButtenReturn;

                if (hasClickedSave) 
                {
                    int textInputReturn = GuiTextInputBox((Rectangle){ GetScreenWidth() / 2 - 100, 10, 200, 150 }, "export image name", "automatically appends '.h'", "save", &imgName, IMGNAMEBUFSIZE, 0);

                    // clicked x
                    if (textInputReturn == 0) hasClickedSave = false;

                    // clicked 'save'
                    if (textInputReturn == 1)
                    {
                        const char finalImageName[IMGNAMEBUFSIZE + 2];

                        strcpy(finalImageName, imgName);
                        strcat(finalImageName, ".h");

                        ExportImageAsCode(droppedImage, &finalImageName);

                        isSavedToDisk = true;

                        hasClickedSave = false;

                        TraceLog(LOG_DEBUG, "saved image");
                    }

                }

            }

            if (isSavedToDisk) GuiDrawText("saved image!", (Rectangle){ 10, 10, TEXT_SPACING * 12, TEXT_SIZE }, TEXT_ALIGN_LEFT, GREEN);

            // draw exit dialogue

            if (showExitDialogue)
            {
                DrawRectangleRec(exitDialogueBounds, Fade(RAYWHITE, 0.8f));
                int result = GuiMessageBox(exitDialogueBounds, GuiIconText(ICON_EXIT, "Close Window"), "Do you want to exit?", "Yes;No");

                if ((result == 0) || (result == 2)) showExitDialogue = false;
                else if (result == 1) hasExited = true;
            }



        EndDrawing();
    }

    if (hasImage)
    {
        UnloadTexture(droppedImageTexture);
        UnloadImage(droppedImage);
    }

    CloseWindow();

    return 0;
}