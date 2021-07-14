#include "Constants.hpp"
#include "DllTrialManager.hpp"
#include "DllTrialManager.hpp"
#include "DllNetplayManager.hpp"
#include "CharacterSelect.hpp"

#include <fstream>
#include <locale>
#include <codecvt>
#include <string>
#include <windows.h>


namespace TrialManager
{
wstring comboName;
vector<wstring> comboTrialText;
vector<wstring> fullStrings;
string dtext;
int comboTrialTextAlign = 0;
int comboTrialLength = 0;
int comboTrialPosition = 0;
int currentTrial = 0;
bool hideText = false;
LPDIRECT3DTEXTURE9 trialTextures = NULL;
int trialTextures2 = 0;
int trialTextures3 = 0;
} // namespace TrialManager
using namespace std;

extern NetplayManager* netManPtr;

void DllTrialManager::frameStepTrial()
{
    //tmp2 += 1;
    //tmp2 %= 300;
    if ( !initialized )
        return;

    //TrialManager::comboTrialText = { L"5A >", L"2A >", L"5B >", L"5C" };
    TrialManager::comboTrialText = comboText[TrialManager::currentTrial];
    TrialManager::comboName = comboNames[TrialManager::currentTrial];
    TrialManager::comboTrialLength = 4;

    //comboSeq = {{ 4, 1, 2, 3, -1 }};

    char buf[1000];
    sprintf(buf, "temp1=%d, temp=%d, ehitC=%d, rhitc=%02d,cCombo=%02d, p2cseq=%03d, currSeq=%03d, exSeq=%02d",
            //offaddrp=%d, offAddr=0x%08X,
            //comboDrop,
            //cStartcomboStart,
            //(int)TrialManager::trialTextures,
            //(int)&TrialManager::trialTextures,
            0,0,
            currentHitcount,
            getHitcount(),
            TrialManager::currentTrial,
            //(*CC_P1_COMBO_OFFSET_ADDR * 0x2C),
            //((*CC_P1_COMBO_OFFSET_ADDR * 0x2C) / 4 + CC_P1_COMBO_HIT_BASE_ADDR ),
            *CC_P2_SEQUENCE_ADDR,
            *CC_P1_SEQUENCE_ADDR,
            comboSeq[TrialManager::currentTrial][TrialManager::comboTrialPosition]);
    TrialManager::dtext = buf;
    cout << TrialManager::dtext << endl;
    if ( !comboDrop ) {
        if ( *CC_P1_SEQUENCE_ADDR == comboSeq[TrialManager::currentTrial][0] &&
             *CC_P2_SEQUENCE_ADDR != 0 && !comboStart ) {
            TrialManager::comboTrialPosition = 1;
            comboStart = true;
            currentHitcount = getHitcount();
        } else if ( ( *CC_P1_SEQUENCE_ADDR == comboSeq[TrialManager::currentTrial][TrialManager::comboTrialPosition] ||
                      ( ( comboSeq[TrialManager::currentTrial][TrialManager::comboTrialPosition] == 0 ) && ( *CC_P1_SEQUENCE_ADDR == 12 ) ) )
                    && comboStart &&
                    ( (comboHit[TrialManager::currentTrial][TrialManager::comboTrialPosition]) ?
                      (getHitcount() > currentHitcount) : true ) ) {
            TrialManager::comboTrialPosition += 1;
            currentHitcount = getHitcount();
        }
        if ( *CC_P2_SEQUENCE_ADDR == 0 ) {
            comboDrop = true;
            comboDropPos = TrialManager::comboTrialPosition;
            comboStart = false;
            currentHitcount = 0;
            //    TrialManager::comboTrialPosition = -1;
        }
    } else if ( *CC_P1_SEQUENCE_ADDR == 0 &&
                *CC_P2_SEQUENCE_ADDR == 0 ) {
        TrialManager::comboTrialPosition = 0;
    } else if ( *CC_P1_SEQUENCE_ADDR ==  comboSeq[TrialManager::currentTrial][0] &&
                *CC_P2_SEQUENCE_ADDR != 0 ) {
        TrialManager::comboTrialPosition = 1;
        comboDrop = false;
        comboStart = true;
        currentHitcount = getHitcount();
    }
    /*
    if ( ( GetAsyncKeyState ( VK_F3 ) & 0x1 ) == 1 ) {
        loadCombo( TrialManager::currentTrial + 1 );
    }
    */
    if ( ( GetAsyncKeyState ( VK_F2 ) & 0x1 ) == 1 ) {
        loadCombo( TrialManager::currentTrial - 1 );
    }
    if ( ( GetAsyncKeyState ( VK_F12 ) & 0x1 ) == 1 ) {
        if ( TrialManager::comboTrialTextAlign > 1 ) {
            TrialManager::comboTrialTextAlign = -1;
        }
        else if ( TrialManager::comboTrialTextAlign < 1 ) {
            TrialManager::comboTrialTextAlign = 0;
        }
        else if ( TrialManager::comboTrialTextAlign == 0 ) {
            TrialManager::comboTrialTextAlign = 1;
        }
    }
}

wstring tmp( string s ){
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    //std::string narrow = converter.to_bytes(wide_utf16_source_string);
    std::wstring wide = converter.from_bytes(s);
    return wide;
}
void DllTrialManager::loadTrialFile()
{
    string fileNameBase = "cccaster/trials/";
    string moon;
    switch ( *CC_P1_MOON_SELECTOR_ADDR )
    {
        case 0:
            moon = "C";
            break;
        case 1:
            moon = "F";
            break;
        case 2:
            moon = "H";
            break;
        default:
            break;
    }
    fileNameBase.append( moon );
    const char* cname = getShortCharaName ( *CC_P1_CHARACTER_ADDR );
    fileNameBase.append( cname );
    fileNameBase.append( ".txt" );
    ifstream trialFile(  fileNameBase );
    if ( trialFile ) {
        string str;
        getline( trialFile, str );
        numCombos = stoi( str );
        for ( int i = 0; i < numCombos; ++i ) {
            getline( trialFile, str);
            wstring ncomboName = tmp( str );
            getline( trialFile, str);
            int numInputs = stoi( str );
            vector<wstring> ncomboText;
            vector<int> ncomboSeq;
            vector<int> ncomboHit;
            wstring nfullstring;
            for ( int j = 0; j < numInputs; ++j ) {
                getline( trialFile, str );
                //if ( j != numInputs - 1 )
                //    str.append( " >");
                wstring tstr = tmp( str );
                ncomboText.push_back( tstr );
                nfullstring.append( tstr );
                getline( trialFile, str );
                ncomboSeq.push_back( stoi( str ) );
                getline( trialFile, str );
                ncomboHit.push_back( stoi( str ) );
            }
            comboNames.push_back( ncomboName );
            comboText.push_back( ncomboText );
            comboSeq.push_back( ncomboSeq );
            comboHit.push_back( ncomboHit );
            TrialManager::fullStrings.push_back( nfullstring );
        }
        initialized = true;
    }
    //*CC_SHOW_ATTACK_DISPLAY = 1;
    FILE* file = fopen ("coords.txt", "r");
    fscanf (file, "%d %d %d %d %d %d", &i1, &i2, &i3, &i4, &i5, &i6);
    fclose( file );
    //cout << TrialManager::fullStrings[0];
}

void DllTrialManager::loadCombo( int comboId )
{
    if ( comboId < 0 )
        comboId = numCombos - 1;
    TrialManager::currentTrial = ( comboId % numCombos );
    TrialManager::comboTrialPosition = 0;
    currentHitcount = 0;
    comboDrop = false;
    comboStart = false;

    if ( comboId < numCombos ) {
    }
}

int DllTrialManager::getHitcount()
{
    return *(uint32_t*)((*CC_P1_COMBO_OFFSET_ADDR * 0x2C) / 4 + CC_P1_COMBO_HIT_BASE_ADDR );
}

extern "C" int CallDrawSprite ( int spriteWidth, int dxdevice, int texAddr, int screenXAddr, int screenYAddr, int spriteHeight, int texXAddr, int texYAddr, int texXSize, int texYSize, int flags, int unk, int layer );

extern "C" int CallDrawText ( int width, int height, int xAddr, int yAddr, char* text, int textAlpha, int textShade, int textShade2, void* font, int spacing, int unk, char* out );

extern "C" int CallDrawRect ( int screenXAddr, int screenYAddr, int width, int height, int A, int B, int C, int D, int layer );

void DllTrialManager::drawButton( int buttonId, int screenX, int screenY, int width, int height )
{
    // Buttons:
    // 0: A 1: B 2: C 3:D
    CallDrawSprite ( width, 0, *(int*)BUTTON_SPRITE_TEX, screenX, screenY, height, 0x19*buttonId, 0x19, 0x19, 0x19, 0xFFFFFFFF, 0, 0x2cc );
}

void DllTrialManager::drawArrow( int direction, int screenX, int screenY, int width, int height )
{
    CallDrawSprite ( width, 0, *(int*)BUTTON_SPRITE_TEX, screenX, screenY, height, 0x19*direction, 0, 0x19, 0x19, 0xFFFFFFFF, 0, 0x2cc );
}

void DllTrialManager::drawText( string text, int screenX, int screenY, int width, int height )
{
    vector<char> ctext(text.begin(), text.end());
    ctext.push_back(0);

    CallDrawText ( width, height, screenX, screenY, &ctext[0],
                   0xff, // alpha
                   0xff, // shade
                   0xff, // also alpha?
                   (void*) FONT2,
                   0, 0, 0 );
}

void DllTrialManager::drawTextWithBorder( string text, int screenX, int screenY, int width, int height )
{
    vector<char> ctext(text.begin(), text.end());
    ctext.push_back(0);

    CallDrawText ( width, height, screenX-1, screenY, &ctext[0],
                   0xff, // alpha
                   0x0, // shade
                   0xff, // also alpha?
                   (void*) FONT2,
                   0, 0, 0 );
    CallDrawText ( width, height, screenX+1, screenY, &ctext[0],
                   0xff, // alpha
                   0x0, // shade
                   0xff, // also alpha?
                   (void*) FONT2,
                   0, 0, 0 );
    CallDrawText ( width, height, screenX, screenY-1, &ctext[0],
                   0xff, // alpha
                   0x0, // shade
                   0xff, // also alpha?
                   (void*) FONT2,
                   0, 0, 0 );
    CallDrawText ( width, height, screenX, screenY+1, &ctext[0],
                   0xff, // alpha
                   0x0, // shade
                   0xff, // also alpha?
                   (void*) FONT2,
                   0, 0, 0 );

}

void DllTrialManager::drawShadowButton( int buttonId, int screenX, int screenY, int width, int height )
{
    // Buttons:
    // 0=A 1=B 2=C 3=D
    CallDrawSprite ( width, 0, (int) TrialManager::trialTextures3, screenX, screenY, height, 0x19*buttonId, 125, 0x19, 0x19, 0xFFFFFFFF, 0, 0x2cc );
}

void DllTrialManager::drawShadowArrow( int direction, int screenX, int screenY, int width, int height )
{
    CallDrawSprite ( width, 0, (int) TrialManager::trialTextures3, screenX, screenY, height, 0x19*direction, 100, 0x19, 0x19, 0xFFFFFFFF, 0, 0x2cc );
}

int DllTrialManager::drawComboBacking( MovePosition position, MoveStatus status, int screenX, int screenY, int width, int height )
{
    // Type: 0=Full, 1=Start, 2=End
    // Status: 0=Next, 1=Current, 3=Completed, 4=Fail Point
    int texStartX = 4;
    int centerTexWidth = 33;
    int texStartY = 2;
    int yoffset = status*32;
    int centerStartX = 18;
    int endStartX = 52;
    int nextX = screenX;
    bool drawStart = true;
    bool drawEnd = true;
    switch ( position ) {
        case Middle:
            break;
        case Start:
            drawStart = false;
            break;
        case Ending:
            drawEnd = false;
            break;
        default:
            break;
    }
    if ( drawStart ) {
        CallDrawSprite ( 15, 0, (int) TrialManager::trialTextures2, screenX, screenY, height, texStartX, texStartY+yoffset, 15, 32, 0xFFFFFFFF, 0, 0x2cb );
        screenX +=15;
        nextX += 15;
    }
    CallDrawSprite ( width, 0, (int) TrialManager::trialTextures2, screenX, screenY, height, centerStartX, texStartY+yoffset, centerTexWidth, 32, 0xFFFFFFFF, 0, 0x1cc );
    nextX += width;
    if ( drawEnd ) {
        CallDrawSprite ( 18, 0, (int) TrialManager::trialTextures2, screenX+width, screenY, height, endStartX, texStartY+yoffset, 18, 32, 0xFFFFFFFF, 0, 0x2cb );
        nextX += 4;
    }
    return nextX ;
}
void DllTrialManager::drawInputs()
{
    int width = 640;
    int left = width / 2 - 12 - 25;
    int top = 480 - 50;
    uint16_t rawinput = netManPtr->getRawInput( 1 );
    uint16_t dir = 0xF & rawinput;
    int leftmod = 0;
    int topmod = 0;
    for ( uint16_t i=1; i < 10; ++i ){
        if ( dir == i ) {
            drawArrow( i, left+leftmod, top-topmod );
        } else {
            drawShadowArrow( i, left+leftmod, top-topmod );
        }
        leftmod = ( leftmod + 25 ) % 75;
        if ( i % 3 == 0)
            topmod = ( topmod + 25 );
    }
    uint16_t buttons = rawinput >> 4;
    if ( buttons & CC_BUTTON_A ) {
        drawButton( 0, left, top + 25 );
        drawSolidRect( 2, 0, 25, boxHeight - 32, red );
    } else {
        drawShadowButton( 0, left, top + 25 );
    }
    if ( buttons & CC_BUTTON_B ) {
        drawButton( 1, left+25, top + 25 );
        drawSolidRect( 29, 0, 25, boxHeight - 32, red );
    } else {
        drawShadowButton( 1, left+25, top + 25 );
    }
    if ( buttons & CC_BUTTON_C ) {
        drawButton( 2, left+50, top + 25 );
        drawSolidRect( 56, 0, 25, boxHeight - 32, red );
    } else {
        drawShadowButton( 2, left+50, top + 25 );
    }
    if ( buttons & CC_BUTTON_D ) {
        drawButton( 3, left+75, top + 25 );
    } else {
        drawShadowButton( 3, left+75, top + 25 );
    }
}

vector<Move> DllTrialManager::tokenizeText( vector<wstring> text )
{
    wstring dirs = L"12346789";
    wstring buttons = L"ABCD";
    wstring brackets = L"[]{}";
    vector<Move> moves;
    for( unsigned int j = 0; j < text.size(); ++j ) {
        wstring move = text[j];
        vector<Token> tokens;
        unsigned int i = 0;
        while( i < move.length() ) {
            wchar_t curMove = move[i];
            if ( curMove == L'd' ) {
                tokens.push_back( Token{ "d.", Symbol, 2 } );
                i++;
            } else if ( curMove == L'j' ) {
                tokens.push_back( Token{ "j.", Symbol, 2 } );
                i++;
            } else if ( curMove == L't' ) {
                tokens.push_back( Token{ "tk.", Symbol, 3 } );
                i += 2;
            } else if ( dirs.find( curMove ) != wstring::npos ) {
                tokens.push_back( Token{ string( 1, curMove ), Direction, 1 } );
            } else if ( buttons.find( curMove ) != wstring::npos ) {
                if ( i+1 < move.length() && move[i+1] == L'T' ) {
                    tokens.push_back( Token{ "AT", String, 2 } );
                    i++;
                } else if ( i+1 < move.length() && move[i+1] == L'd' ) {
                    tokens.push_back( Token{ "Add.", Symbol, 4 } );
                    i += 3;
                } else {
                    tokens.push_back( Token{ string( 1, curMove ), Button, 1 } );
                }
            } else if ( curMove == L'(' ) {
                string textFrag;
                int len = 2;
                while( move[i] != L')' ){
                    textFrag += move[i];
                    i++;
                    len++;
                }
                textFrag += L')';
                i++;
                tokens.push_back( Token{ textFrag, String, len } );
            } else if ( brackets.find( curMove ) != wstring::npos ) {
                tokens.push_back( Token{ string( 1, curMove ), String, 1 } );
            }
            i++;
        }
        MovePosition pos;
        if ( j == 0 ) {
            pos = Start;
        } else if ( j == text.size() - 1 ) {
            pos = Ending;
        } else {
            pos = Middle;
        }
        moves.push_back( Move{ tokens, pos } );
    }
    return moves;
}

int DllTrialManager::drawMove( Move move, MoveStatus color, int x, int y )
{
    int loffset = i1;
    if ( move.position != Start ) {
        loffset += 15;
    }
    int yoffset = i2;
    int ytextoffset = i2;
    int roffset = i3;
    int currX = x + loffset;
    int nextX = x + loffset;
    int moveWidth = 0;
    for( Token token : move.text ) {
        if ( token.type == Button ) {
            nextX += 25;
            if ( nextX > 630){
                y += 25;
                currX = x + loffset;
            }
            int buttonId = token.text[0] - L'A';
            drawButton( buttonId, currX, y+yoffset );
            currX += 25;
            moveWidth += 25;
        } else if ( token.type == Direction ) {
            int buttonId = token.text[0] - L'0';
            drawArrow( buttonId, currX, y+yoffset );
            currX += 25;
            moveWidth += 25;
        } else if ( token.type == String ) {
            drawText( token.text, currX, y+ytextoffset );
            currX += 24*token.text.length();
            moveWidth += 24*token.text.length();
        } else if ( token.type == Symbol ) {
            if ( token.text[ 0 ] == 'd' ) {
                drawText( "d", currX, y+ytextoffset );
                drawText( ".", currX + 15, y+ytextoffset );
                currX += 35;
                moveWidth += 35;
            } else if ( token.text[ 0 ] == 'j' ) {
                drawText( "j", currX, y+ytextoffset );
                drawText( ".", currX + 10, y+ytextoffset );
                currX += 30;
                moveWidth += 30;
            } else if ( token.text[ 0 ] == 'A' ) {
                drawText( "A", currX, y+ytextoffset );
                currX += 20;
                moveWidth += 20;
                drawText( "d", currX, y+ytextoffset );
                currX += 16;
                moveWidth += 16;
                drawText( "d", currX, y+ytextoffset );
                currX += 14;
                moveWidth += 14;
                drawText( ".", currX, y+ytextoffset );
                currX += 17;
                moveWidth += 17;
            } else if ( token.text[ 0 ] == 't' ) {
                drawText( "t", currX, y+ytextoffset );
                currX += 15;
                moveWidth += 15;
                drawText( "k", currX, y+ytextoffset );
                currX += 11;
                moveWidth += 11;
                drawText( ".", currX, y+ytextoffset );
                currX += 15;
                moveWidth += 15;
            }
        }
    }
    x = drawComboBacking( move.position, color, x, y, moveWidth+roffset );
    return x;
}

void DllTrialManager::drawCombo()
{
    drawComboBacking( Middle,Next,20+67-15,20,67,50);
    drawComboBacking( Start,Current,20+67-15,70,67,50);
    drawComboBacking( Ending,Done,20+67-15,120,67,50);
    drawComboBacking( Start,Done,20+67-15,170,67,50);
    drawComboBacking( Middle,Done,20+67-15+67+4,170,67,50);
    drawComboBacking( Ending,Failed,20+67-15+(67+4)*2+15,170,500,50);
    char* test = "j.[B]";

    /*
    CallDrawText ( i1, i2, i3, i4, test, 0xff, 0xff, 0xcc,
                   (void*) FONT2,
                   0, 0, 0 );
    */
    int x = i4;
    int newx = i4;
    int y = i5;
    int loffset = 0;
    int yoffset = 0;
    int roffset = 10;
    int currentMove = 0;
    int currentFail = 3;
    MoveStatus color;

    vector<wstring> comboTrialText = { L"5A", L"2A", L"5B", L"AT",
        L"tk.236B", L"Add.A", L"j.B", L"[B]"};
    //    L"(delay)j.5B(hold)"};
    if ( !TrialManager::comboTrialText.empty() ) {
        //if ( initialized ) {
        comboTrialText = TrialManager::comboTrialText;
        currentMove = TrialManager::comboTrialPosition;
        currentFail = comboDropPos;
    }
    vector<Move> moveList = tokenizeText( comboTrialText );

    for( int i = 0; i < moveList.size(); ++i ) {
        if ( i < currentMove ) {
            color = Done;
        } else if ( i == currentMove ) {
            color = Current;
        } else if ( i == currentFail ) {
            color = Failed;
        } else {
            color = Next;
        }
        Move move = moveList[i];

        newx = drawMove( move, color, x+loffset, y+yoffset );
        if ( newx < x ) {
            y += 25;
        }
        x = newx;
    }
}

void DllTrialManager::drawSolidRect( int x, int y, int width, int height, ARGB color, int layer ){
    int colorValue = ( color.alpha << 24 ) + ( color.red << 16 ) +
        ( color.green << 8 ) + ( color.blue );
    CallDrawRect( x, y, width, height, colorValue, colorValue, colorValue, colorValue, layer );
    //CallDrawRect( 0, 0, 100,480, 0xffffffff, 0xff0000ff, 0xff00, colorValue, 0x2cb );
}
void DllTrialManager::drawiidx(){
    if (tmp2 > boxHeight + 10 ) {
        tmp2=-20;
    }
    tmp2+=3;
    drawSolidRect( 0, boxHeight - 32, boxWidth, 5, red );
    drawSolidRect( 0, 0, boxWidth, boxHeight, ARGB{ 0xaa, 0x0, 0x0, 0x0 } );
    drawSolidRect( 0, boxHeight - 27, boxWidth, 2, white );
    drawSolidRect( 0, boxHeight, boxWidth, 2, white );
    drawSolidRect( 0, 0, 2, boxHeight, white );
    drawButton( 0, 2, boxHeight - 25 );
    drawSolidRect( 27, 0, 2, boxHeight, white );
    drawButton( 1, 29, boxHeight - 25 );
    drawSolidRect( 54, 0, 2, boxHeight, white );
    drawButton( 2, 56, boxHeight - 25 );
    drawSolidRect( 81, 0, 2, boxHeight, white );

    drawSolidRect( 2, tmp2, 25, 2, white, 0x2cc );
    drawSolidRect( 29, tmp2-10, 25, 2, white, 0x2cc );
    drawSolidRect( 56, tmp2-20, 25, 2, white, 0x2cc );
}

void DllTrialManager::drawAttackDisplay()
{
    string test = "MAX TEST";
    string testVal = "50";
    int leftX = 0x1a4;
    int rightX = leftX + 200;
    int y = 0xe8;
    //drawText(test, leftX, y, 0xa, 0xe );
    //drawText(test, leftX, y, 0xa, 0xe );
    //drawTextWithBorder(test, leftX, y, 0xa, 0xe );
    //drawText(testVal, rightX-0xa*2, y, 0xa, 0xe );

    int currentMeter = *CC_P1_METER_ADDR;
    if ( *CC_P2_SEQUENCE_ADDR != 0 ) {
        if ( currentMeter > lastSeenMeter ) {
            meterGained = currentMeter - lastSeenMeter;
            totalMeterGained += meterGained;
        }
        showTotalMeterGained = totalMeterGained;
        lastSeenMeter = currentMeter;
    } else {
        lastSeenMeter = currentMeter;
        totalMeterGained = 0;
    }
    string meter = "METER GAIN";
    string gained = "";
    y += 14;
    char buf[10];
    sprintf(buf, "%d.%02d", meterGained/100, meterGained%100 );
    gained = string(buf);
    drawText(meter, leftX, y, 0xa, 0xe );
    drawTextWithBorder(meter, leftX, y, 0xa, 0xe );
    drawText(gained, rightX-0xa*gained.length(), y, 0xa, 0xe );
    drawTextWithBorder(gained, rightX-0xa*gained.length(), y, 0xa, 0xe );
    y+=14;
    string total = "TOTAL METER GAIN";
    sprintf(buf, "%d.%02d", showTotalMeterGained/100, showTotalMeterGained%100 );
    string totalgained = string(buf);
    drawText(total, leftX, y, 0xa, 0xe );
    drawTextWithBorder(total, leftX, y, 0xa, 0xe );
    drawText(totalgained, rightX-0xa*totalgained.length(), y, 0xa, 0xe );
    drawTextWithBorder(totalgained, rightX-0xa*totalgained.length(), y, 0xa, 0xe );
    y+=14;
    ifstream file( "trainingentries.txt");
    char buf2[40];
    if ( file ) {
        string label;
        string value;
        string type;
        getline( file, label );
        int numEntries = stoi( label );
        for ( auto i=0; i<numEntries; ++i ) {
            getline( file, label );
            getline( file, value );
            getline( file, type );
            int val = stoi(value, nullptr, 16);
            if ( type == "float" ) {
                sprintf(buf2, "%f", *(float*)val);
            } else if ( type == "int" ) {
                sprintf(buf2, "%d", *(int*)val);
            }
            //sprintf(buf2, "%d", *val);
            string formatValue = string(buf2);
            drawAttackDisplayRow( label, formatValue, y );
            y+= 14;
        }
    }
}
void DllTrialManager::drawAttackDisplayRow( string label, string value, int y )
{
    int leftX = 0x1a4;
    int rightX = leftX + 200;
    drawText(label, leftX, y, 0xa, 0xe );
    drawTextWithBorder(label, leftX, y, 0xa, 0xe );
    drawText(value, rightX-0xa*value.length(), y, 0xa, 0xe );
    drawTextWithBorder(value, rightX-0xa*value.length(), y, 0xa, 0xe );

}
void DllTrialManager::render()
{
    if ( *CC_SHOW_ATTACK_DISPLAY ) {
        drawAttackDisplay();
    }
    //drawInputs();
    //drawCombo();
    //drawiidx();
}

void DllTrialManager::clear()
{
    comboNames.clear();
    comboText.clear();
    comboSeq.clear();
    comboHit.clear();
    TrialManager::comboTrialText.clear();
    TrialManager::fullStrings.clear();
    TrialManager::comboTrialPosition = 0;
    currentHitcount = 0;
    comboDrop = false;
    comboStart = false;

}
