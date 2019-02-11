#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef UNICODE
#define UNICODE
#endif

#include <wchar.h>
#include <iostream>
#include <string>

#include <vector>
#include <math.h>
#include "olcConsoleGameEngine.h"

using namespace std;

class Racer : public olcConsoleGameEngine {

public:
    Racer() {
        m_sAppName = L"Racer boi";
    }

private:
    float m_carPos = 0.0f;
    float m_distance = 0.0f;
    float m_speed = 0.0f;

    float m_curvature = 0.0f;
    float m_trackDistance = 0.0f;

    // The game is essencially about alining these two parameter
    // where one is given by the track(the game), and the other is given
    // by the player (by inputs)
    float m_trackcurvature = 0.0f;
    float m_playercurvature = 0.0f;

    float m_currentLapTime = 0.0f;

    vector<pair<float, float>> vecTrack; // curvature, distance
    list<float> m_lastLapTimes;

protected:
    virtual bool OnUserCreate() {

        vecTrack.push_back(make_pair(0.0f, 10.0f)); // start and finish line section
        vecTrack.push_back(make_pair(0.0f, 200.0f));
        vecTrack.push_back(make_pair(1.0f, 200.0f));
        vecTrack.push_back(make_pair(0.0f, 400.0f));
        vecTrack.push_back(make_pair(-1.0f, 100.0f));
        vecTrack.push_back(make_pair(0.0f, 200.0f));
        vecTrack.push_back(make_pair(-1.0f, 200.0f));
        vecTrack.push_back(make_pair(1.0f, 200.0f));
        vecTrack.push_back(make_pair(0.0f, 200.0f));
        vecTrack.push_back(make_pair(0.2f, 500.0f));
        vecTrack.push_back(make_pair(0.0f, 200.0f));

        for (auto t : vecTrack) {
            m_trackDistance += t.second;
        }

        m_lastLapTimes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

        return true;
    }

    virtual bool OnUserUpdate(float deltaTime) {

        if (m_keys[VK_UP].bHeld) {
            m_speed += (2.0f * deltaTime);
        } else {
            m_speed -= (1.0f * deltaTime);
        }

        if (m_keys[VK_LEFT].bHeld) {
            m_playercurvature -= 0.7f * deltaTime;
        }

        if (m_keys[VK_RIGHT].bHeld) {
            m_playercurvature += 0.7f * deltaTime;
        }

        if (fabs(m_playercurvature - m_trackcurvature) >= 0.63f) {
            m_speed -= 5.0f * deltaTime;
        }

        // Clamping on speed
        if (m_speed < 0.0f) {
            m_speed = 0.0f;
        } else if ( m_speed >= 1.0f ) {
            m_speed = 1.0f;
        }


        m_distance += (70.0f * m_speed) * deltaTime;

        float offset = 0;
        unsigned int trackSection = 0;

        m_currentLapTime += deltaTime;

        if (m_distance >= m_trackDistance) {
            m_distance -= m_trackDistance;
            m_lastLapTimes.push_front(m_currentLapTime);
            m_lastLapTimes.pop_back();
            m_currentLapTime = 0.0f;
        }

        for (;trackSection < vecTrack.size() && offset <= m_distance; trackSection++) {
            offset += vecTrack[trackSection].second;
        }

        // track curvature interprepolation
        float targetCurvature = vecTrack[trackSection - 1].first;
        float trackCurveDiff = (targetCurvature - m_curvature) * m_speed * deltaTime;

        m_curvature += trackCurveDiff;

        m_trackcurvature += (m_curvature) * deltaTime * m_speed;

        // sky
        for(int y = 0; y < ScreenHeight() / 2; y++)
        {
            for(int x = 0; x < ScreenWidth(); x++)
            {
                Draw(x, y, y < ScreenHeight() / 4 ? PIXEL_HALF : PIXEL_SOLID, FG_DARK_BLUE);
            }
        }

        for(int x = 0; x < ScreenWidth(); x++)
        {
            int hillHeight = (int)(fabs(sinf(x * 0.01f + m_trackcurvature) * 16.0f));
            for(int y = (ScreenHeight() / 2) - hillHeight; y < ScreenHeight() / 2; y++)
            {
                Draw(x, y, PIXEL_SOLID, FG_DARK_YELLOW);
            }
        }
        

        // draw landscape
        for ( int y = 0; y < ScreenHeight() / 2; y++ ) {
            for ( int x = 0; x < ScreenWidth(); x++ ) {

                // --  Drawing the road and grass -- 
                // perspective is simply a ratio of the y index to the lower half of the screen
                float perspective = (float) y / (ScreenHeight() / 2.0f);

                // Everything drawn is based around a middle point that is normalized (0.0 to 1.0)
                // The curvature of the road is determined by a polynomial function of the perspective
                float middlePoint = 0.5f + m_curvature * powf((1.0f - perspective), 3);

                float roadWidth = 0.1f + perspective;
                float clipWidth = roadWidth * 0.15f;

                // making the road narrower
                roadWidth *= 0.42f;

                int leftGrass = (middlePoint - roadWidth - clipWidth) * ScreenWidth();
                int leftClip = (middlePoint - roadWidth) * ScreenWidth();

                int rightGrass = (middlePoint + roadWidth + clipWidth) * ScreenWidth();
                int rightClip = (middlePoint + roadWidth) * ScreenWidth();

                int nRow = ScreenHeight() / 2 + y;
                int grassColor = sinf(20.0f * powf(1.0f - perspective, 3) + m_distance * 0.1f) > 0.0f ? FG_GREEN : FG_DARK_GREEN;
                int clipColor = sinf(20.0f * powf(1.0f - perspective, 2) + m_distance * 0.1f) > 0.0f ? FG_RED : FG_WHITE;
                
                int roadColor = (trackSection - 1) == 0 ? FG_WHITE : FG_GREY;

                if ( x >= 0 && x < leftGrass ) {
                    Draw(x, nRow, PIXEL_SOLID, grassColor);
                }
                if ( x >= leftGrass && x < leftClip ) {
                    Draw(x, nRow, PIXEL_SOLID, clipColor);
                }
                if ( x >= leftClip && x < rightClip ) {
                    Draw(x, nRow, PIXEL_SOLID, roadColor);
                }
                if ( x >= rightClip && x < rightGrass ) {
                    Draw(x, nRow, PIXEL_SOLID, clipColor);
                }
                if ( x >= rightGrass && x < ScreenWidth() ) {
                    Draw(x, nRow, PIXEL_SOLID, grassColor);
                }
            }
        }

        // Draw car
        m_carPos = m_playercurvature - m_trackcurvature;
        int cPos = (ScreenWidth() / 2) + ((int)(ScreenWidth() * m_carPos) / 2.0f) - 7;

        // position clamp
        if ( cPos <= 0 ) {
            cPos = 0;
        } else if ( cPos >= ScreenWidth() - 14 ) {
            cPos = ScreenWidth() - 14;
        }

        int baseY = 60;
        DrawStringAlpha(cPos, baseY,     L"   ||####||   ");
        DrawStringAlpha(cPos, baseY + 1, L"      ##      ");
        DrawStringAlpha(cPos, baseY + 2, L"     ####     ");
        DrawStringAlpha(cPos, baseY + 3, L"     ####     ");
        DrawStringAlpha(cPos, baseY + 4, L"|||  ####  |||");
        DrawStringAlpha(cPos, baseY + 5, L"|||########|||");
        DrawStringAlpha(cPos, baseY + 6, L"|||  ####  |||");


        // debug stats
        DrawString(0, 0, L"Distance        : " + to_wstring(m_distance));
        DrawString(0, 1, L"Target Curvature: " + to_wstring(m_curvature));
        DrawString(0, 2, L"Player Position : " + to_wstring(cPos));
        DrawString(0, 3, L"Player Curvature: " + to_wstring(m_playercurvature));
        DrawString(0, 4, L"Player Speed    : " + to_wstring(m_speed));
        DrawString(0, 5, L"Track  Curvature: " + to_wstring(m_trackcurvature));

        // Lap times:
        auto display_time = [](float t) {
            int minutes = t / 60.0f;
            int seconds = t - (minutes * 60.0f);
            int milisecs = (t - (float) seconds) * 1000.0f;
            return to_wstring(minutes) + L"." + to_wstring(seconds) + L":" + to_wstring(milisecs);
        };

        DrawString(10, 7, L"Time:");
        DrawString(10, 8, display_time(m_currentLapTime));
        int j = 10;
        for (auto l : m_lastLapTimes) {
            DrawString(10, j, display_time(l));
            j++;
        }

        return true;
    }

};

extern "C"
int wmain() {
    Racer game;
    game.ConstructConsole(160, 80, 8, 8);

    game.Start();
    return 0;
}