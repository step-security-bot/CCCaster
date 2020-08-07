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
    sprintf(buf, "cDrop=%d, cStart=%d, ehitC=%d, rhitc=%d,cCombo=%d, p2cseq=%d, currSeq=%d, exSeq=%d",
            //offaddrp=%d, offAddr=0x%08X,
            comboDrop,
            comboStart,
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
    }
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
