/*
  Project-wise types and definitions
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#include "ebeamerDefs.h"

bool isLinearArray(MicConfig m){
    switch(m){
        case ULA_1ESTICK:
        case ULA_2ESTICK:
        case ULA_3ESTICK:
        case ULA_4ESTICK:
            return true;
        case URA_2ESTICK:
        case URA_3ESTICK:
        case URA_4ESTICK:
        case URA_2x2ESTICK:
            return false;
    }
};
