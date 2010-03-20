/***************************************************************
 *
 * The weights for the General POS tagging model
 *
 * Yue Zhang, 2007.10
 *
 ***************************************************************/

#ifndef _COMMON_TAGGER_WEIGHTS_H
#define _COMMON_TAGGER_WEIGHTS_H 1

#include "tagger_include.h"

#include "weight_base.h"

namespace TARGET_LANGUAGE {
namespace tagger {

#include "tagger_impl.h"
#include "stateitem.h"

#define iterate_templates(left, right)\
   left(m_mapLastTagByTag)right\
   left(m_mapLastTwoTagsByTag)right\
\
   left(m_mapCurrentTag)right\
   left(m_mapTagByPrevWord)right\
   left(m_mapTagByNextWord)right\
   left(m_mapTagBySecondPrevWord)right\
   left(m_mapTagBySecondNextWord)right\
\
   left(m_mapContainHyphen)right\
   left(m_mapContainNumber)right\
   left(m_mapContainCapitalLetter)right\
   left(m_mapTagByPrefix)right\
   left(m_mapTagBySuffix)right

//
// TYPE DEFINITIONS
//
typedef CScoreMap< CWord, SCORE_TYPE > CWordMap;
typedef CScoreMap< CTwoWords, SCORE_TYPE > CTwoWordsMap;
//typedef CScoreMap< unsigned long, SCORE_TYPE > CIntMap;
//typedef CScoreMap< pair<unsigned long, unsigned long>, SCORE_TYPE > CTwoIntMap;
//typedef CScoreMap< pair<CWord, unsigned long>, SCORE_TYPE > CWordIntMap;
typedef CScoreMap< CTagSet<CTag, 2>, SCORE_TYPE > CTagSet2Map;
typedef CScoreMap< CTagSet<CTag, 3>, SCORE_TYPE > CTagSet3Map;
typedef CScoreMap< pair<CWord, CTag>, SCORE_TYPE > CWordTagMap;
typedef CScoreMap< CTag, SCORE_TYPE > CTagMap;

/*===============================================================
 *
 * CWeight - the definition of weights, in feature
 *
 *==============================================================*/

struct CWeight : public CWeightBase {

   // FEATURE TEMPLATES 
   CTagSet2Map m_mapLastTagByTag;
   CTagSet3Map m_mapLastTwoTagsByTag;

   CWordTagMap m_mapCurrentTag;
   CWordTagMap m_mapTagByPrevWord;
   CWordTagMap m_mapTagByNextWord;
   CWordTagMap m_mapTagBySecondPrevWord;
   CWordTagMap m_mapTagBySecondNextWord;

   CTagMap m_mapContainHyphen;
   CTagMap m_mapContainNumber;
   CTagMap m_mapContainCapitalLetter;
   CWordTagMap m_mapTagByPrefix;
   CWordTagMap m_mapTagBySuffix;

   // CONSTRUCTOR 
   CWeight(const string &sFeatureDB, bool bTrain) : CWeightBase(sFeatureDB, bTrain) , 
                                                    m_mapLastTagByTag("TagBigram", 65537),
                                                    m_mapLastTwoTagsByTag("TagTrigram", 65537),
                                                    m_mapCurrentTag("TagUnigram", 65537),
                                                    m_mapTagByPrevWord("Tag and previous word", 65537),
                                                    m_mapTagByNextWord("Tag and next word", 65537),
                                                    m_mapTagBySecondPrevWord("Tag and second previous", 65537),
                                                    m_mapTagBySecondNextWord("Tag and second next", 65537),
                                                    m_mapContainHyphen("Contains hyphen", 65537),
                                                    m_mapContainNumber("Contains number", 65537),
                                                    m_mapContainCapitalLetter("ontains capitalized letter", 65537),
                                                    m_mapTagByPrefix("Tag by prefix", 65537),
                                                    m_mapTagBySuffix("Tag by suffix", 65537) {}

   // MEHTODS
   void loadScores(); 
   void saveScores(); 
   void computeAverageFeatureWeights(int round);
 
};

};
};

#endif