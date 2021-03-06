#include "Constants.hpp"
#include "DllTrialManager.hpp"
#include "DllTrialManager.hpp"
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

void DllTrialManager::frameStepTrial()
{
    if ( !initialized )
        return;

    //TrialManager::comboTrialText = { "5A >", "2A >", "5B >", "5C" };
    TrialManager::comboTrialText = comboText[TrialManager::currentTrial];
    TrialManager::comboName = comboNames[TrialManager::currentTrial];
    TrialManager::comboTrialLength = 4;

    //comboSeq = {{ 4, 1, 2, 3, -1 }};

    char buf[1000];
    sprintf(buf, "temp1=%d, temp=%d, ehitC=%d, rhitc=%d,cCombo=%d, p2cseq=%d, currSeq=%d, exSeq=%d",
            //offaddrp=%d, offAddr=0x%08X,
            //comboDrop,
            //cStartcomboStart,
            (int)TrialManager::trialTextures,
            (int)&TrialManager::trialTextures,
            currentHitcount,
            getHitcount(),
            TrialManager::currentTrial,
            //(*CC_P1_COMBO_OFFSET_ADDR * 0x2C),
            //((*CC_P1_COMBO_OFFSET_ADDR * 0x2C) / 4 + CC_P1_COMBO_HIT_BASE_ADDR ),
            *CC_P2_SEQUENCE_ADDR,
            *CC_P1_SEQUENCE_ADDR,
            comboSeq[TrialManager::currentTrial][TrialManager::comboTrialPosition]);
    TrialManager::dtext = buf;
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
            comboStart = false;
            currentHitcount = 0;
            //    TrialManager::comboTrialPosition = -1;
        }
    } else if ( *CC_P1_SEQUENCE_ADDR ==  comboSeq[TrialManager::currentTrial][0] &&
                *CC_P2_SEQUENCE_ADDR != 0 ) {
        TrialManager::comboTrialPosition = 1;
        comboDrop = false;
        comboStart = true;
        currentHitcount = getHitcount();
    }
    if ( ( GetAsyncKeyState ( VK_F3 ) & 0x1 ) == 1 ) {
        loadCombo( TrialManager::currentTrial + 1 );
    }

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
                getline( trialFile, str);
                if ( j != numInputs - 1 )
                    str.append( " >");
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
        tmp2 = 300;
    }
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

void DllTrialManager::render()
{
    if ( tmp2 < 0 ) tmp2 = 300;
    tmp2 -= 2;
    CallDrawSprite ( 25, 0, *(int*)0x74d5e8, tmp2, 24, 25, 0, 0x19, 0x19, 0x19, 0xFFFFFFFF, 0, 0x2cc );
    CallDrawSprite ( 25, 0, *(int*)0x74d5e8, tmp2+15, 24+25, 25, 0x19, 0x19, 0x19, 0x19, 0xFFFFFFFF, 0, 0x2cc );
    CallDrawSprite ( 25, 0, *(int*)0x74d5e8, tmp2+40, 24+50, 25, 0x19*2, 0x19, 0x19, 0x19, 0xFFFFFFFF, 0, 0x2cc );
    //CallDrawSprite ( 25, 0, *(int*)0x74d5e8, 50, 24, 25, 0x19, 0x19, 0x19, 0x19, 0xFFFFFFFF, 0, 0x2cc );
    //CallDrawSprite ( 25, 0, (int)TrialManager::trialTextures3, 50, 24, 25, 0, 0, 9, 9, 0xFFFFFFFF, 0, 0x2cb );
    //CallDrawSprite ( 67, 0, (int) TrialManager::trialTextures2, 20+67-15, 20, 32, 4, 34, 67, 32, 0xFFFFFFFF, 0, 0x2cb );
    //CallDrawSprite ( 67, 0, (int) TrialManager::trialTextures3, 20+67-15, 20, 64, 4, 34, 67, 64, 0xFFFFFFFF, 0, 0x2cb );
    CallDrawSprite ( i1, 0, (int) TrialManager::trialTextures3, i3, i4, i2, i5, i6, i1, i2, 0xFFFFFFFF, 0, 0x2cb );
    CallDrawSprite ( i1, 0, (int) TrialManager::trialTextures3, i3, i4+25, i2, i5, i6, i1, i2, 0xFFFFFFFF, 0, 0x2cb );
    CallDrawSprite ( i1, 0, (int) TrialManager::trialTextures3, i3, i4+50, i2, i5, i6, i1, i2, 0xFFFFFFFF, 0, 0x2cb );
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
