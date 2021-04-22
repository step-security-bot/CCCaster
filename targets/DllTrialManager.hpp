#pragma once
#include <vector>

#include <d3dx9.h>

using namespace std;

namespace TrialManager {

extern wstring comboName;

extern vector<wstring> comboTrialText;

extern vector<wstring> fullStrings;

extern string dtext;

extern int comboTrialTextAlign;

extern int comboTrialLength;

extern int comboTrialPosition;

extern int currentTrial;

extern bool hideText;

extern LPDIRECT3DTEXTURE9 trialTextures;

extern int trialTextures2;
extern int trialTextures3;
} // namespace TrialManager

enum MovePosition {
    Start,
    Middle,
    Ending
};

enum MoveStatus {
    Next,
    Current,
    Done,
    Failed
};

enum TokenTypes {
    String,
    Direction,
    Button,
    Symbol
};

struct Token {
    string text;
    TokenTypes type;
    int length;
};

struct Move {
    vector<Token> text;
    MovePosition position;
};

class DllTrialManager
{
public:

    void frameStepTrial();
    void loadTrialFile();
    void clear();
    void render();
    void drawButton( int buttonId, int screenX, int screenY, int width=25, int height=25 );
    void drawArrow( int buttonId, int screenX, int screenY, int width=25, int height=25 );
    void drawText( string text, int screenX, int screenY, int width=24, int height=24 );
    void drawShadowButton( int buttonId, int screenX, int screenY, int width=25, int height=25 );
    void drawShadowArrow( int buttonId, int screenX, int screenY, int width=25, int height=25 );
    void drawInputs();
    void drawiidx();
    int drawComboBacking( MovePosition position, MoveStatus status, int screenX, int screenY, int width, int height=32 );
    void drawCombo();
    void loadCombo( int comboId );
    int drawMove( Move move, MoveStatus color, int x, int y );
    vector<Move> tokenizeText( vector<wstring> text );

    bool initialized = false;

private:

    bool comboDrop = false;
    bool comboStart = false;

    int currentHitcount = 0;

    int numCombos;

    vector<wstring> comboNames;
    vector<vector<wstring>> comboText;
    vector<vector<int>> comboSeq;
    vector<vector<int>> comboHit;

    int getHitcount();

    int tmp2 = 0;
    int i1;
    int i2;
    int i3;
    int i4;
    int i5;
    int i6;
};
